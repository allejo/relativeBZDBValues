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

struct PlayerCountCondition
{
    int minPlayers;
    float value;
    std::string message;
};

struct BZDBCondition
{
    std::string bzdbSetting;
    int delayInSeconds;
    std::vector<PlayerCountCondition> conditions;
};

namespace YAML {
    template<>
    struct convert<PlayerCountCondition>
    {
        static Node encode(const PlayerCountCondition &rhs)
        {
            Node node;
            node["minPlayers"] = rhs.minPlayers;
            node["value"] = rhs.value;
            node["message"] = rhs.message;

            return node;
        }

        static bool decode(const Node &node, PlayerCountCondition &rhs)
        {
            if (!node.IsMap())
            {
                return false;
            }

            rhs.minPlayers = node["minPlayers"].as<int>();
            rhs.value = node["value"].as<float>();
            rhs.message = node["message"].as<std::string>();

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
    void parseConfiguration(const char* config);
    void updateVariablesIfNecessary();

    std::map<std::string, double> lastConditionChange;
    std::map<std::string, int> lastCondition;
    std::vector<BZDBCondition> conditions;

    const char* configurationFile;
};

BZ_PLUGIN(RelativeBZDBValues)

const char* RelativeBZDBValues::Name()
{
    return "Relative BZDB Values";
}

void RelativeBZDBValues::Init(const char* config)
{
    Register(bz_ePlayerJoinEvent);
    Register(bz_ePlayerPartEvent);

    bz_registerCustomSlashCommand("reload", this);
    bz_registerCustomSlashCommand("set", this);

    parseConfiguration(config);
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
        case bz_ePlayerJoinEvent:
        case bz_ePlayerPartEvent:
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
    if (command == "reload")
    {

        return false;
    }
    else if (command == "set")
    {

        return false;
    }

    return false;
}

void RelativeBZDBValues::parseConfiguration(const char* config)
{
    configurationFile = config;

    YAML::Node configuration = YAML::LoadFile(config);

    conditions = configuration["relative_bzdb"].as<std::vector<BZDBCondition>>();
}

void RelativeBZDBValues::updateVariablesIfNecessary()
{
    int totalPlayers = bz_getPlayerCount() - bz_getTeamCount(eObservers);

    for (auto &condition : conditions)
    {
        auto values = condition.conditions;

        for (auto &condition : values)
        {
            if (totalPlayers >= condition.minPlayers)
            {

            }
        }
    }
}
