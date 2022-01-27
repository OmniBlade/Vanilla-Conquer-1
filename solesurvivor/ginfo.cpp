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
#include "ginfo.h"
#include "function.h"
#include "soleglobals.h"
#include "solewdt.h"

extern TimerClass GameTimer;
int16_t GameInfoClass::TeamCounters[4];

void GameInfoClass::Render(int w, int h)
{
    int entries = 0;

    for (int i = 0; i < SidebarClass::ColorList1->Count(); ++i) {
        if (SidebarClass::ColorList1->Colors[i] != GREY) {
            ++entries;
        }
    }

    if (!GameParams.IsMaxNumAIsScaled || GameParams.NumTeams != 1 || GameParams.AllowNoTeam) {
        entries -= GameParams.NumTeams;
    }

    for (int i = 0; i < MAX_TEAMS /* Max teams */; ++i) {
        TeamCounters[i] = 0;
    }

    for (int i = 0; i < SidebarClass::ColorList1->Count(); ++i) {
        if (SidebarClass::ColorList1->Colors[i] == '\xDB') {
            ++TeamCounters[0];
        }
        if (SidebarClass::ColorList1->Colors[i] == '\x19') {
            ++TeamCounters[1];
        }
        if (SidebarClass::ColorList1->Colors[i] == '\x03') {
            ++TeamCounters[2];
        }
        if (SidebarClass::ColorList1->Colors[i] == '\xD2') {
            ++TeamCounters[3];
        }
    }

    if (!GameParams.IsMaxNumAIsScaled || GameParams.NumTeams != 1 || GameParams.AllowNoTeam) {
        for (unsigned i = 0; i < GameParams.NumTeams; ++i) {
            --TeamCounters[i];
        }
    }

    int y_pos = 401;
    char buffer[80];

    /*
    ** Print counts of players per team
    */
    if (!OfflineMode) {
        int total = TeamCounters[3] + TeamCounters[2] + TeamCounters[1] + TeamCounters[0];
        sprintf(buffer, Text_String(total <= 0 ? TXT_D_PLAYERS : TXT_D_PLAYERS_COLON), entries);
        Conquer_Clip_Text_Print(buffer, 482, y_pos, WHITE, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160);
        if (GameParams.NumTeams > 0) {
            sprintf(buffer, "%2d", TeamCounters[0]);
            Conquer_Clip_Text_Print(
                buffer, 557, y_pos, BLUE, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160);
        }
        if (GameParams.NumTeams > 1) {
            sprintf(buffer, "%2d", TeamCounters[1]);
            Conquer_Clip_Text_Print(
                buffer, 578, y_pos, LTCYAN, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160);
        }
        if (GameParams.NumTeams > 2) {
            sprintf(buffer, "%2d", TeamCounters[2]);
            Conquer_Clip_Text_Print(
                buffer, 599, y_pos, GREEN, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160);
        }
        if (GameParams.NumTeams > 3) {
            sprintf(buffer, "%2d", TeamCounters[3]);
            Conquer_Clip_Text_Print(
                buffer, 620, y_pos, CYAN, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160);
        }
    }

    y_pos += 12;

    /*
    ** Print time remaining if there is a time limit
    */
    if (GameParams.TimeLimit > 0) {
        int seconds_remaining = (TIMER_SECOND * GameParams.TimeLimit) - (GameTimer.Time() / TIMER_SECOND);
        if (seconds_remaining <= 60) {
            if (seconds_remaining < 0) {
                seconds_remaining = 0;
            }

            sprintf(buffer, Text_String(TXT_TIME_LEFT_SECS), seconds_remaining);
            Conquer_Clip_Text_Print(
                buffer, 482, y_pos, YELLOW, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160);

            if (OfflineMode) {
                static int _last_remaining = -1;
                if (seconds_remaining <= 5 && seconds_remaining != _last_remaining) {
                    if (seconds_remaining > 0) {
                        Sound_Effect(VOC_TARGET);
                    }
                    _last_remaining = seconds_remaining;
                }
            }
        } else {
            sprintf(buffer, Text_String(TXT_TIME_LEFT_MINS), ((seconds_remaining - 1) / TIMER_SECOND) + 1);
            Conquer_Clip_Text_Print(
                buffer, 482, y_pos, WHITE, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160);
        }

        y_pos += 12;
    }

    /*
    ** Print number of lives remaining
    */
    if (GameParams.LifeLimit > 0) {
        sprintf(buffer, Text_String(TXT_LIVES_LEFT), GameParams.LifeLimit - (PlayerPtr->LivesSpent - 1));
        Conquer_Clip_Text_Print(buffer, 482, y_pos, WHITE, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160);
        y_pos += 12;
    }

    /*
    ** Print score limit
    */
    if (GameParams.ScoreLimit > 0) {
        sprintf(buffer, Text_String(TXT_SCORE_LIMIT), GameParams.ScoreLimit);
        Conquer_Clip_Text_Print(buffer, 482, y_pos, WHITE, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160);
        y_pos += 12;
    }

    /*
    ** Print ion cannon risk for next crate
    */
    if (GameParams.IsCrates && CurrentObject.Count() > 0) {
        int object_stats = Calc_Object_Stats(CurrentObject[0]);
        int ion_danger = 100 * object_stats / IonFactor;
        sprintf(buffer, Text_String(TXT_ION_DANGER), ion_danger);
        char color = 0;

        if (GameParams.IonCannon && ion_danger >= 40 && CurrentObject[0]->What_Am_I() != RTTI_INFANTRY) {
            if (ion_danger >= 55) {
                const char* string;
                if (ion_danger >= 70) {
                    color = RED;
                    string = Text_String(TXT_DANGER_HIGH);
                } else {
                    color = 25; /* Orange? */
                    string = Text_String(TXT_DANGER_MED);
                }

                sprintf(&buffer[strlen(buffer)], string);
            } else {
                color = YELLOW;
                sprintf(&buffer[strlen(buffer)], Text_String(TXT_DANGER_LOW));
            }
        } else {
            color = LTGREEN;
            sprintf(&buffer[strlen(buffer)], Text_String(TXT_DANGER_NONE));
        }
        Conquer_Clip_Text_Print(buffer, 482, y_pos, color, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160);
    }
}
