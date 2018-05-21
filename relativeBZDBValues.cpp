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

#include "bzfsAPI.h"
#include "plugin_utils.h"

class RelativeBZDBValues : public bz_Plugin, public bz_CustomSlashCommandHandler
{
    virtual const char* Name();
    virtual void Init(const char* config);
    virtual void Cleanup();
    virtual void Event(bz_EventData* eventData);
    virtual bool SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params);
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

    bz_registerCustomSlashCommand("set", this);
}

void RelativeBZDBValues::Cleanup()
{
    Flush();

    bz_removeCustomSlashCommand("set");
}

void RelativeBZDBValues::Event(bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_ePlayerJoinEvent:
        {
            // This event is called each time a player joins the game
            bz_PlayerJoinPartEventData_V1 *data = (bz_PlayerJoinPartEventData_V1*)eventData;

            // Data
            // ----
            // (int)                  playerID  - The player ID that is joining
            // (bz_BasePlayerRecord*) record    - The player record for the joining player
            // (double)               eventTime - Time of event.
        }
        break;

        case bz_ePlayerPartEvent:
        {
            // This event is called each time a player leaves a game
            bz_PlayerJoinPartEventData_V1 *data = (bz_PlayerJoinPartEventData_V1*)eventData;

            // Data
            // ----
            // (int)                  playerID  - The player ID that is leaving
            // (bz_BasePlayerRecord*) record    - The player record for the leaving player
            // (bz_ApiString)         reason    - The reason for leaving, such as a kick or a ban
            // (double)               eventTime - Time of event.
        }
        break;

        default:
            break;
    }
}

bool RelativeBZDBValues::SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
    if (command == "set")
    {

        return true;
    }

    return false;
}