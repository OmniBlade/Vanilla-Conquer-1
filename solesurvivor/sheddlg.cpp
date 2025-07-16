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

char SrverDlg_Title[80];
char SrverDlg_Scenario[80];
char SrverDlg_Directory[80];
char SrverDlg_Players[80];
char SrverDlg_Spectators[80];
char SrverDlg_AdminPlayers[80];
char SrverDlg_RemoteAdmins[80];
char SrverDlg_TimeLimit[20];
char SrverDlg_TimeRemaining[20];
char SrverDlg_ScoreLimit[20];
char SrverDlg_LifeLimit[20];
char SrverDlg_CaptureTheFlag[20];
char SrverDlg_Football[20];
char SrverDlg_FootballNumFlags[20];
char SrverDlg_IonCannon[20];
char SrverDlg_TeamCrates[20];
char SrverDlg_NumCTFStructs[20];
char SrverDlg_ResetTeamsInCTF[20];
char SrverDlg_AllowFlagSitting[20];
char SrverDlg_FreeRadarForAll[20];
char SrverDlg_HealthBars[20];
char SrverDlg_AIUnitsPerMin_Max[20];
char SrverDlg_AIBuildingsPerMin_Max[20];
char SrverDlg_LosePowerups[20];

void Fill_In_Server_Dlg_Values(void);

#ifdef WIN32

void Update_Server_Dialog(HWND hdlg)
{
    Fill_In_Server_Dlg_Values();

    SetWindowTextA(GetDlgItem(hdlg, 109), SrverDlg_Title);
    SetWindowTextA(GetDlgItem(hdlg, 103), SrverDlg_Scenario);
    SetWindowTextA(GetDlgItem(hdlg, 108), SrverDlg_Directory);
    SetWindowTextA(GetDlgItem(hdlg, 104), SrverDlg_Players);
    SetWindowTextA(GetDlgItem(hdlg, 105), SrverDlg_Spectators);
    SetWindowTextA(GetDlgItem(hdlg, 106), SrverDlg_AdminPlayers);
    SetWindowTextA(GetDlgItem(hdlg, 107), SrverDlg_RemoteAdmins);
    SetWindowTextA(GetDlgItem(hdlg, 111), SrverDlg_TimeLimit);
    SetWindowTextA(GetDlgItem(hdlg, 112), SrverDlg_TimeRemaining);
    SetWindowTextA(GetDlgItem(hdlg, 113), SrverDlg_ScoreLimit);
    SetWindowTextA(GetDlgItem(hdlg, 114), SrverDlg_LifeLimit);
    SetWindowTextA(GetDlgItem(hdlg, 116), SrverDlg_CaptureTheFlag);
    SetWindowTextA(GetDlgItem(hdlg, 117), SrverDlg_Football);
    SetWindowTextA(GetDlgItem(hdlg, 118), SrverDlg_FootballNumFlags);
    SetWindowTextA(GetDlgItem(hdlg, 115), SrverDlg_IonCannon);
    SetWindowTextA(GetDlgItem(hdlg, 120), SrverDlg_TeamCrates);
    SetWindowTextA(GetDlgItem(hdlg, 119), SrverDlg_NumCTFStructs);
    SetWindowTextA(GetDlgItem(hdlg, 122), SrverDlg_ResetTeamsInCTF);
    SetWindowTextA(GetDlgItem(hdlg, 123), SrverDlg_AllowFlagSitting);
    SetWindowTextA(GetDlgItem(hdlg, 124), SrverDlg_FreeRadarForAll);
    SetWindowTextA(GetDlgItem(hdlg, 125), SrverDlg_HealthBars);
    SetWindowTextA(GetDlgItem(hdlg, 126), SrverDlg_AIUnitsPerMin_Max);
    SetWindowTextA(GetDlgItem(hdlg, 128), SrverDlg_AIBuildingsPerMin_Max);
    SetWindowTextA(GetDlgItem(hdlg, 127), SrverDlg_LosePowerups);
}

extern long CALLBACK _export Server_Dlg_Proc(HWND hwnd, UINT message, WPARAM arg1, LPARAM arg2);
extern HWND ServerDlg;

HWND Create_Server_Window(void)
{
    if (GameToPlay == GAME_HOST) {
        ServerDlg = CreateDialogParam((HINSTANCE)hInstance, (LPCSTR)1, 0, (DLGPROC)Server_Dlg_Proc, 0);
        ShowWindow(ServerDlg, 1);
        SetForegroundWindow(ServerDlg);
        Update_Server_Dialog(ServerDlg);
    }

    return ServerDlg;
}

extern void Host_Disconnect();
extern void Prog_End();

extern TimerClass WDTGameTimer;

long CALLBACK _export Server_Dlg_Proc(HWND hdlg, UINT message, WPARAM arg1, LPARAM arg2)
{
    if (hdlg) {
        printf("hdlg is %d\n", hdlg);
    } else {
        printf("Null dlg handle\n");
    }
    fflush(stdout);

    switch (message) {

    case WM_INITDIALOG:
        return true;

    case 1025:
        Update_Server_Dialog(hdlg);
        break;

    case WM_COMMAND:
        switch (arg1) {
        case 102:
            Update_Server_Dialog(hdlg);
            break;

        case 2:
            SendMessageA(hdlg, WM_CLOSE, 0, 0);
            return true;
        }
        break;

    case WM_CLOSE:
        if (GameToPlay == GAME_HOST) {
            DestroyWindow(hdlg);
            ServerDlg = 0;
            PostQuitMessage(0);
            Host_Disconnect();
            Free_Scenario_Descriptions();
            Prog_End();
            exit(0);
        }
        return true;
    }

    return false;
}
#endif

void Fill_In_Server_Dlg_Values(void)
{
    char filename[260];
    int count;
    int index;
    HousesType house;

    HouseClass* hptr = NULL;

    count = 0;
    for (index = 0; index < ActivePlayers.Count(); index++) {
        hptr = ActivePlayers[index]->HousePtr;
        if (hptr && hptr->Class) {
            house = hptr->Class->House;
            if (house != HOUSE_SPECTATOR && house != HOUSE_ADMIN) {
                count++;
            }
        }
    }
    sprintf(SrverDlg_Players, "%d", count);

    count = 0;
    for (index = 0; index < ActivePlayers.Count(); index++) {
        hptr = ActivePlayers[index]->HousePtr;
        if (hptr && hptr->Class) {
            house = hptr->Class->House;
            if (house == HOUSE_SPECTATOR) {
                count++;
            }
        }
    }
    sprintf(SrverDlg_Spectators, "%d", count);
    count = 0;

    for (index = 0; index < ActivePlayers.Count(); index++) {
        hptr = ActivePlayers[index]->HousePtr;
        if (hptr && hptr->Class) {
            house = hptr->Class->House;
            if (house == HOUSE_ADMIN) {
                count++;
            }
        }
    }
    sprintf(SrverDlg_AdminPlayers, "%d", count);

    sprintf(SrverDlg_RemoteAdmins, "%d", RemoteAdminsComms.Count());
    getcwd(SrverDlg_Directory, sizeof(SrverDlg_Directory));
    getcwd(filename, 0x104u);
    strcat(filename, "\\SERVER.INI");
    CDFileClass fc(filename);
    INIClass ini;
    ini.Load(fc);
    ini.Get_String("Login", "Channel", "Unknown", SrverDlg_Title, sizeof(SrverDlg_Title));
    SrverDlg_Scenario[0] = 0;

    for (index = 0; index < MPlayerFilenum.Count(); index++) {
        if (Scen.Scenario == MPlayerFilenum[index]) {
            strcpy(SrverDlg_Scenario, MPlayerScenarios[index]);
        }
    }

    if (GameParams.TimeLimit > 0) {
        sprintf(SrverDlg_TimeLimit, "%d", GameParams.TimeLimit);
        sprintf(SrverDlg_TimeRemaining, "%d", GameParams.TimeLimit - WDTGameTimer.Time() / 3600);
    } else {
        strcpy(SrverDlg_TimeLimit, "---");
        strcpy(SrverDlg_TimeRemaining, "---");
    }

    if (GameParams.ScoreLimit > 0) {
        sprintf(SrverDlg_ScoreLimit, "%d", GameParams.ScoreLimit);
    } else {
        strcpy(SrverDlg_ScoreLimit, "---");
    }

    if (GameParams.LifeLimit > 0) {
        sprintf(SrverDlg_LifeLimit, "%d", GameParams.LifeLimit);
    } else {
        strcpy(SrverDlg_LifeLimit, "---");
    }

    if (GameParams.IsCaptureTheFlag) {
        strcpy(SrverDlg_CaptureTheFlag, "On");
    } else {
        strcpy(SrverDlg_CaptureTheFlag, "Off");
    }

    if (GameParams.Football) {
        strcpy(SrverDlg_Football, "On");
        if (GameParams.FootballNumFlags == 1) {
            strcpy(SrverDlg_FootballNumFlags, "1");
        } else {
            strcpy(SrverDlg_FootballNumFlags, "2");
        }
    } else {
        strcpy(SrverDlg_Football, "Off");
        strcpy(SrverDlg_FootballNumFlags, "---");
    }

    switch (GameParams.IonCannon) {
    case 0:
        strcpy(SrverDlg_IonCannon, "Off");
        break;

    case 1:
        strcpy(SrverDlg_IonCannon, "On");
        break;

    case 2:
        strcpy(SrverDlg_IonCannon, "FATAL!");
        break;
    }
    sprintf(SrverDlg_TeamCrates, "%d", GameParams.TeamCrates);
    sprintf(SrverDlg_NumCTFStructs, "%d", GameParams.NumCTFStructures);
    sprintf(SrverDlg_ResetTeamsInCTF, "%d", GameParams.ResetTeamsInCTF);
    sprintf(SrverDlg_AllowFlagSitting, "%d", GameParams.AllowFlagSitting);
    sprintf(SrverDlg_FreeRadarForAll, "%d", GameParams.FreeRadarForAll);
    sprintf(SrverDlg_HealthBars, "%d", GameParams.HealthBars);
    sprintf(SrverDlg_AIUnitsPerMin_Max, "%d, %d", GameParams.AIUnitsPer10min, GameParams.MaxAIUnits);
    sprintf(SrverDlg_AIBuildingsPerMin_Max, "%d, %d", GameParams.AIBuildingsPer10min, GameParams.MaxAIBuildings);
    sprintf(SrverDlg_LosePowerups, "%d", GameParams.LosePowerups);
}