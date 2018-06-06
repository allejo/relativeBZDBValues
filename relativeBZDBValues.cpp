/*
 * Copyright (C) 2018 Vladimir "allejo" Jimenez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <yaml-cpp/yaml.h>

#include "bzfsAPI.h"
#include "plugin_utils.h"

// Define plug-in name
const std::string PLUGIN_NAME = "Relative BZDB Values";

// Define plug-in version numbering
const int MAJOR = 1;
const int MINOR = 0;
const int REV = 0;
const int BUILD = 14;

enum class ValueConditionType
{
    INT,
    BOOL,
    DOUBLE,
    STRING,
    UNKNOWN
};

struct PlayerCountCondition
{
    int minPlayers = -1;
    std::string message = "";

    ValueConditionType valueType = ValueConditionType::UNKNOWN;

    int intValue = -1;
    bool boolValue = false;
    double doubleValue = -1.0;
    bz_ApiString stringValue = "";
};

struct BZDBCondition
{
    std::string bzdbSetting = "";
    int delayInSeconds = -1;
    std::vector<PlayerCountCondition> conditions;
};

namespace YAML
{
    template<>
    struct convert<PlayerCountCondition>
    {
        static Node encode(const PlayerCountCondition &rhs)
        {
            Node node;
            node["minPlayers"] = rhs.minPlayers;
            node["message"] = rhs.message;

            switch (rhs.valueType)
            {
                case ValueConditionType::INT:
                    node["intValue"] = rhs.intValue;
                    break;

                case ValueConditionType::BOOL:
                    node["boolValue"] = rhs.boolValue;
                    break;

                case ValueConditionType::DOUBLE:
                    node["doubleValue"] = rhs.doubleValue;
                    break;

                case ValueConditionType::STRING:
                    node["stringValue"] = rhs.stringValue.c_str();
                    break;

                default:
                    break;
            }

            return node;
        }

        static bool decode(const Node &node, PlayerCountCondition &rhs)
        {
            if (!node.IsMap())
            {
                return false;
            }

            // Handle minimum player count
            if (node["minPlayers"])
            {
                rhs.minPlayers = node["minPlayers"].as<int>();
            }
            else
            {
                rhs.minPlayers = 1;
            }

            // Handle BZDB values
            if (node["intValue"])
            {
                rhs.intValue = node["intValue"].as<int>();
                rhs.valueType = ValueConditionType::INT;
            }
            else if (node["boolValue"])
            {
                rhs.boolValue = node["boolValue"].as<bool>();
                rhs.valueType = ValueConditionType::BOOL;
            }
            else if (node["doubleValue"])
            {
                rhs.doubleValue = node["doubleValue"].as<double>();
                rhs.valueType = ValueConditionType::DOUBLE;
            }
            else if (node["stringValue"])
            {
                rhs.stringValue = node["stringValue"].as<std::string>();
                rhs.valueType = ValueConditionType::STRING;
            }
            else
            {
                rhs.valueType = ValueConditionType::UNKNOWN;
            }

            // Handle public message announcements
            if (node["message"])
            {
                rhs.message = node["message"].as<std::string>();
            }
            else
            {
                rhs.message = "";
            }

            return true;
        }
    };

    template<>
    struct convert<BZDBCondition>
    {
        static Node encode(const BZDBCondition &rhs)
        {
            Node node;
            node["bzdb"] = rhs.bzdbSetting;
            node["delay"] = rhs.delayInSeconds;
            node["values"] = rhs.conditions;

            return node;
        }

        static bool decode(const Node &node, BZDBCondition &rhs)
        {
            if (!node.IsMap())
            {
                return false;
            }

            rhs.bzdbSetting = node["bzdb"].as<std::string>();
            rhs.delayInSeconds = node["delay"].as<int>();
            rhs.conditions = node["values"].as<std::vector<PlayerCountCondition>>();

            return true;
        }
    };
}

class RelativeBZDBValues : public bz_Plugin, public bz_CustomSlashCommandHandler
{
public:
    virtual const char* Name();
    virtual void Init(const char* config);
    virtual void Cleanup();
    virtual void Event(bz_EventData* eventData);
    virtual bool SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params);

private:
    void updateVariablesIfNecessary();
    void updateBZDBCondition(BZDBCondition condition, PlayerCountCondition *value);
    void safeSendMessage(bz_ApiString message);
    void parseConfiguration();

    std::map<std::string, PlayerCountCondition> currentActiveCondition;
    std::map<std::string, double> timeConditionChanged;

    std::vector<BZDBCondition> conditions;
    std::vector<std::string> bzdbBlacklist;

    std::string configurationFile;
};

BZ_PLUGIN(RelativeBZDBValues)

const char* RelativeBZDBValues::Name()
{
    static const char* pluginBuild;

    if (!pluginBuild)
    {
        pluginBuild = bz_format("%s %d.%d.%d (%d)", PLUGIN_NAME.c_str(), MAJOR, MINOR, REV, BUILD);
    }

    return pluginBuild;
}

void RelativeBZDBValues::Init(const char* config)
{
    Register(bz_eTickEvent);

    bz_registerCustomSlashCommand("reload", this);
    bz_registerCustomSlashCommand("set", this);

    configurationFile = config;
    parseConfiguration();
}

void RelativeBZDBValues::Cleanup()
{
    Flush();

    bz_removeCustomSlashCommand("reload");
    bz_removeCustomSlashCommand("set");
}

void RelativeBZDBValues::Event(bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_eTickEvent:
        {
            updateVariablesIfNecessary();
        }
        break;

        default:
            break;
    }
}

bool RelativeBZDBValues::SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
    if (command == "reload" && bz_hasPerm(playerID, "setAll"))
    {
        if (params->size() == 0 || (params->size() > 0 && params->get(0) == "all"))
        {
            parseConfiguration();
        }
        else if (params->get(0) == "relativeBZDB")
        {
            bz_sendTextMessagef(BZ_SERVER, playerID, "Database for \"%s\" reloaded", PLUGIN_NAME.c_str());
            parseConfiguration();

            return true;
        }

        return false;
    }
    else if (command == "set" && (bz_hasPerm(playerID, "setAll") || bz_hasPerm(playerID, "setVar")))
    {
        if (params->size() >= 2)
        {
            std::string bzdbSetting = params->get(0);

            if (std::find(bzdbBlacklist.begin(), bzdbBlacklist.end(), bzdbSetting) != bzdbBlacklist.end())
            {
                bz_sendTextMessage(BZ_SERVER, playerID, "The BZDB variable you're trying to set is maintained by a plug-in.");
                bz_sendTextMessage(BZ_SERVER, playerID, "You are not allowed to change this variable.");

                return true;
            }
        }

        return false;
    }

    return false;
}

void RelativeBZDBValues::parseConfiguration()
{
    if (configurationFile.empty())
    {
        bz_debugMessagef(0, "ERROR :: %s :: The path to the required configuration cannot be blank.", PLUGIN_NAME.c_str());
        return;
    }

    try
    {
        YAML::Node configuration = YAML::LoadFile(configurationFile);

        conditions = configuration["relative_bzdb"].as<std::vector<BZDBCondition>>();
    }
    catch (YAML::Exception e)
    {
        bz_debugMessagef(0, "ERROR :: %s :: An error occurred while trying to read the YAML configuration file.", PLUGIN_NAME.c_str());
        bz_debugMessagef(0, "ERROR :: %s ::   %s", PLUGIN_NAME.c_str(), e.msg.c_str());

        return;
    }

    // Configure our BZDB blacklist
    // ---
    // These are BZDB settings that maintained by this plug-in, so we should prevent admins from /set'ing them

    bzdbBlacklist.clear();

    for (BZDBCondition &condition : conditions)
    {
        bzdbBlacklist.push_back(condition.bzdbSetting);
    }
}

void RelativeBZDBValues::updateVariablesIfNecessary()
{
    double now = bz_getCurrentTime();
    int totalPlayers = bz_getPlayerCount() - bz_getTeamCount(eObservers);

    for (BZDBCondition &condition : conditions)
    {
        double lastChangeTime = timeConditionChanged[condition.bzdbSetting];

        if (lastChangeTime + condition.delayInSeconds > now)
        {
            // Don't change a value if the number of seconds of `delay` hasn't passed yet
            continue;
        }

        auto values = condition.conditions;

        PlayerCountCondition *queuedValue = nullptr;

        // Go through to values for this BZDB setting
        for (auto &value : values)
        {
            if (totalPlayers >= value.minPlayers)
            {
                queuedValue = &value;
            }
        }

        updateBZDBCondition(condition, queuedValue);
    }
}

void RelativeBZDBValues::updateBZDBCondition(BZDBCondition condition, PlayerCountCondition* value)
{
    if (!value)
    {
        return;
    }

    if (currentActiveCondition.count(condition.bzdbSetting))
    {
        PlayerCountCondition current = currentActiveCondition[condition.bzdbSetting];

        if (current.minPlayers == value->minPlayers)
        {
            // This is the same condition that's currently active, so don't try setting it again.
            return;
        }
    }

    bool valueWasChanged = true;
    const char* bzdb = condition.bzdbSetting.c_str();

    switch (value->valueType)
    {
        case ValueConditionType::INT:
            bz_updateBZDBInt(bzdb, value->intValue);
            break;

        case ValueConditionType::BOOL:
            bz_updateBZDBBool(bzdb, value->boolValue);
            break;

        case ValueConditionType::DOUBLE:
            bz_updateBZDBDouble(bzdb, value->doubleValue);
            break;

        case ValueConditionType::STRING:
            bz_updateBZDBString(bzdb, value->stringValue.c_str());
            break;

        default:
            bz_debugMessagef(0, "WARNING :: %s :: A condition for %s does not have a valid value.", PLUGIN_NAME.c_str(), bzdb);
            valueWasChanged = false;
            break;
    }

    if (valueWasChanged)
    {
        currentActiveCondition[condition.bzdbSetting] = *value;
        timeConditionChanged[condition.bzdbSetting] = bz_getCurrentTime();

        safeSendMessage(value->message);
    }
}

void RelativeBZDBValues::safeSendMessage(bz_ApiString message)
{
    if (!message.empty())
    {
        bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, message.c_str());
    }
}
