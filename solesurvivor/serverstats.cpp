// TiberianDawn.DLL and RedAlert.dll and corresponding source code is free
// software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// TiberianDawn.DLL and RedAlert.dll and corresponding source code is distributed
// in the hope that it will be useful, but with permitted additional restrictions
// under Section 7 of the GPL. See the GNU General Public License in LICENSE.TXT
// distributed with this program. You should have received a copy of the
// GNU General Public License along with permitted additional restrictions
// with this program. If not, see https://github.com/electronicarts/CnC_Remastered_Collection
#include "function.h"
#include "time.h"
#include "comm/comms.h"
//#include <io.h>				// for unlink

void Read_Server_Stats(void)
{
    static int LastStatsWriteTime = 0;
    static int StatsCounter = 0;
    static bool Stats_INI_Read = false;
    static char StatsFileName[260];
    static int StatsInterval;
    static int ClientStats;

    int unused1;
    int unused2;
    FILE* stream;
    CommStatsClass commsapi1(1);
    CommStatsClass commsapi2(0);

    if (!Stats_INI_Read) {
        char FileName[260];
        Stats_INI_Read = 1;
        CDFileClass fc("SERVER.INI");
        INIClass ini;
        ini.Load(fc);

        StatsInterval = ini.Get_Int("Stats", "StatsInterval", 0);
        ClientStats = ini.Get_Int("Stats", "ClientStats", 0);
        ini.Get_String("Stats", "StatsFile", "SRVRSTAT.OUT", StatsFileName, sizeof(StatsFileName));

        if (GameToPlay != GAME_HOST && ClientStats <= 0) {
            return;
        }

        if (ini.Get_Int("Stats", "StatsAppend", 0) <= 0) {
            CDFileClass df(StatsFileName);
            df.Delete();
        }

        if (GameToPlay != GAME_HOST && ClientStats <= 0) {
            return;
        }

        if (StatsInterval < 0) {
            StatsInterval = 0;
        }

        if (StatsInterval < 5 && StatsInterval) {
            StatsInterval = 5;
        }

        commsapi1.Set2(StatsInterval);
        commsapi2.Set2(StatsInterval);
    }

    if (!StatsInterval) {
        return;
    }

    if (time(0) - LastStatsWriteTime >= StatsInterval) {
        LastStatsWriteTime = time(0);
        stream = fopen(StatsFileName, "a");

        if (!stream) {
            stream = fopen(StatsFileName, "w");
            if (!stream) {
                StatsInterval = 0;
                return;
            }
        }

        HousesType house;
        int players = 0;

        for (house = HOUSE_BLUE_TEAM; house < HOUSE_COUNT; house++) {
            HouseClass* hptr = HouseClass::As_Pointer(house);
            if (house > HOUSE_GREY_TEAM && hptr->IsHuman) {
                players++;
            }
        }

        fprintf(stream, "%d,%d,%d;", players, commsapi1.Get1(), commsapi2.Get1());
        StatsCounter++;
        if (3600 / StatsInterval == StatsCounter) {
            fprintf(stream, "\n");
            StatsCounter = 0;
        }
        fclose(stream);
    }
}
