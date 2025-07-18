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
#include <comms.h>
//#include <io.h>
#include <common/filepcx.h>

//extern bool ServerConnectionLost;
//extern int ColorListTiming;
//extern int CurrentVoiceTheme;
//extern bool SpawnedFromWChat;
//int Read_Game_Options(char* name);
//HWND Get_WChat_Handle(void);
//void Draw_Choice_Entry(RTTIType rtti, int type, int xpos, int ypos, HousesType house, int unused, bool draw_name);
//
//#define ARRAY_SIZE(x) int(sizeof(x) / sizeof(x[0]))
//extern bool DebugLogTeams;      // CONQUER.CPP
//extern TimerClass WDTGameTimer; // GLOBALS.CPP
//extern int PacketLength[];      // TEMP.CPP
//extern RTTIType Chosen_RTTI;
//extern int Chosen_Type;
//extern long PlanetWestwoodPortNumber;
//extern int OfflinePoints;
//extern int OfflineDeathCount;
//extern int WDT_Random_Pick(int minval, int maxval);

int Client_Enter_Host_Address_Dialog(char* host_address)
{
    typedef enum
    {
        REDRAW_NONE = 0,
        REDRAW_BUTTONS,
        REDRAW_BACKGROUND,
        REDRAW_ALL = REDRAW_BACKGROUND
    } RedrawType;

    enum
    {
        BUTTON_OK = 100,
        BUTTON_CANCEL,
        BUTTON_ADDR,
        BUTTON_PORT,
    };

    RedrawType display;
    bool process;
    KeyNumType input;
    int rc = 0;
    int first = true;
    char address[80] = {0};
    char port[5] = {0};

    //memcpy(address, HostAddress, sizeof(address));
    //memcpy(port, PortBuffer, sizeof(port));
    GadgetClass* commands = NULL; // button list
    EditClass edit_addr(BUTTON_ADDR,
                        address,
                        sizeof(address) / 2,
                        TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                        220,
                        177,
                        200,
                        18,
                        EditClass::ALPHANUMERIC);
    EditClass edit_port(BUTTON_PORT,
                        port,
                        sizeof(port),
                        TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                        270,
                        209,
                        100,
                        18,
                        EditClass::NUMERIC);
    TextButtonClass ok_btn(
        BUTTON_OK, TXT_OK, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW, 225, 241, 90, 18);
    TextButtonClass cancel_btn(
        BUTTON_CANCEL, TXT_CANCEL, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW, 325, 241, 90, 18);

    Set_Logic_Page(UnknownViewport2);
    strcpy(address, host_address);
    edit_addr.Set_Text(address, sizeof(address) / 2);
    sprintf(port, "%d", PlanetWestwoodPortNumber);
    edit_port.Set_Text(port, 5);
    commands = &edit_addr;
    edit_port.Add_Tail(*commands);
    ok_btn.Add_Tail(*commands);
    cancel_btn.Add_Tail(*commands);
    Hide_Mouse();
    Load_Title_Screen("HTITLE.PCX", &UnknownViewport2, Palette);
    Set_Palette(Palette);
    UnknownViewport2.Blit(UnknownViewport1);
    Show_Mouse();
    display = REDRAW_ALL;
    process = true;

    while (process) {
        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = false;
            display = REDRAW_ALL;
        }

        Call_Back();

        if (display) {
            Hide_Mouse();

            if (display >= REDRAW_BACKGROUND) {
                Dialog_Box(206, 127, 228, 146);
                Draw_Caption(TXT_NONE, 206, 127, 228);

                if (InMainLoop) {
                    Fancy_Text_Print(TXT_ENTER_HOST_ADDRESS,
                                     320u,
                                     141u,
                                     GREEN,
                                     TBLACK,
                                     TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
                } else {

                    Fancy_Text_Print(TXT_ENTER_HOST_ADDRESS,
                                     320u,
                                     141u,
                                     GREEN,
                                     TBLACK,
                                     TPF_BRIGHT_COLOR | TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
                }
            }

            if (display >= REDRAW_BUTTONS) {
                commands->Draw_All();
            }

            Show_Mouse();
            display = REDRAW_NONE;
        }

        input = commands->Input();

        if (first) {
            edit_addr.Set_Focus();
            edit_addr.Flag_To_Redraw();
            input = commands->Input();
            first = false;
        }

        switch (input) {
        case (KN_RETURN): // fallthrough
        case (BUTTON_OK | KN_BUTTON):
            rc = 0;
            process = false;
            break;
        case (KN_ESC): // fallthrough
        case (BUTTON_CANCEL | KN_BUTTON):
            rc = 1;
            process = false;
            break;
        default:
            break;
        }
    }

    Hide_Mouse();
    Load_Title_Screen("HTITLE.PCX", &UnknownViewport2, Palette);
    Set_Palette(Palette);
    UnknownViewport2.Blit(UnknownViewport1);
    Show_Mouse();

    if (rc) {
        return 0;
    }

    strcpy(host_address, address);
    PlanetWestwoodPortNumber = atoi(port);

    return 1;
}

// Match apart from retval stack
bool Client_Wait_For_WDT_Connection(void)
{
    // Not quite matching?
    typedef enum
    {
        REDRAW_NONE = 0,
        REDRAW_BUTTONS,
        REDRAW_BACKGROUND,
        REDRAW_ALL = REDRAW_BACKGROUND
    } RedrawType;

    enum
    {
        BUTTON_CANCEL = 100,
    };

    RedrawType display;
    WDTPacketStruct destination;
    unsigned short bufx[16]; // TODO unused?
    int rc = 0;
    GadgetClass* commands;
    char str[80] = {0};
    int conn_sent;
    WDTPacketStruct* packet_1;
    bool process;
    KeyNumType input;

    conn_sent = false;
    commands = NULL;
    TextButtonClass cancelbtn(
        BUTTON_CANCEL, TXT_CANCEL, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW, 275, 209, 90, 18);
    Set_Logic_Page(UnknownViewport1);
    commands = &cancelbtn;
    Hide_Mouse();
    Load_Title_Screen("HTITLE.PCX", &UnknownViewport2, Palette);
    Set_Palette(Palette);
    UnknownViewport2.Blit(UnknownViewport1);
    Show_Mouse();
    display = REDRAW_ALL;
    process = true;

    while (process) {
        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = false;
            display = REDRAW_BACKGROUND;
        }

        Call_Back();

        if (display) {
            Hide_Mouse();

            if (display >= REDRAW_BACKGROUND) {
                Dialog_Box(206, 159, 228, 82);
                Draw_Caption(TXT_NONE, 206, 159, 228);
                sprintf(str, Text_String(TXT_SOLE_CONNECTING));
                Fancy_Text_Print(str,
                                 320,
                                 173,
                                 GREEN,
                                 TBLACK,
                                 TPF_BRIGHT_COLOR | TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
            }

            if (display >= REDRAW_BUTTONS) {
                commands->Draw_All();
            }

            Show_Mouse();
            display = REDRAW_NONE;
        }

        input = commands->Input();

        switch (input) {
        case (KN_ESC): // fallthrough
        case (BUTTON_CANCEL | KN_BUTTON):
            rc = 1;
            process = false;
            break;
        default:
            break;
        }

        if (ReliableProtocols[0]->ConnectionState == -1) {
            CCMessageBox().Process(TXT_UNABLE_TO_CONNECT);
            process = false;
            rc = 1;
            break;
        }

        switch (ReliableProtocols[0]->ConnectionState) {
        case 0:
            if (conn_sent) {
                if (GameParams.IsSquadChannel) {
                    CCMessageBox().Process("Connection refused.\r\rThis probably means that a squad game is in "
                                           "progress,\rand entry is closed until it finishes.");
                } else {
                    CCMessageBox().Process("Connection refused!");
                }

                process = false;
                rc = 1;
                continue;
            } else {
                break;
            }
        case 1:
            if (!conn_sent) {
                strcpy((char*)bufx, ReliableComms[0]->Host.DotAddr);
                bufx[15] = PlanetWestwoodPortNumber;
                conn_sent = true;
                destination.Header.Type = (PacketType)101;
                strcpy(destination.Connection.PlayerName, MPlayerName);
                destination.Connection.Side = MPlayerHouse;
                destination.Connection.ChosenRTTI = Chosen_RTTI;
                destination.Connection.ChosenType = Chosen_Type;
                destination.Connection.field_15 = 1;
                destination.Connection.field_19 = 0;
                destination.Connection.VersionNumber = Version_Number();
                ReliableProtocols[0]->Queue->Queue_Send(&destination, PacketLength[PACKET_CONNECTION]);
                ReliableComms[0]->Send();
            }
            break;
        case -1:
            CCMessageBox().Process(TXT_UNABLE_TO_CONNECT);
            process = false;
            rc = 1;
            continue;
        default:
            break;
        }

        if (conn_sent && ReliableProtocols[0]->Queue->Num_Receive() > 0) {
            int i;
            packet_1 = (WDTPacketStruct*)ReliableProtocols[0]->Queue->Get_Receive(0)->Buffer;

            switch (packet_1->Header.Type) {
            case PACKET_GAME_OPTIONS:
                ScenPlayer = SCEN_PLAYER_MPLAYER;
                ScenDir = SCEN_DIR_FIRST;
                MPlayerHouse = packet_1->GameOptions.House;
                MPlayerLocalID = packet_1->GameOptions.LocalID;
                MPlayerPrefColor = packet_1->GameOptions.PrefColor; // useless
                MPlayerColorIdx = packet_1->GameOptions.PrefColor;  // useless
                Scenario = packet_1->GameOptions.Scenario;
                MPlayerCredits = packet_1->GameOptions.Credits; // useless
                MPlayerBases = packet_1->GameOptions.Bases;
                MPlayerTiberium = packet_1->GameOptions.Tiberium;
                MPlayerGoodies = packet_1->GameOptions.Goodies; // useless
                MPlayerGhosts = 0;                              // useless
                BuildLevel = packet_1->GameOptions.BuildLevel;
                MPlayerUnitCount = packet_1->GameOptions.UnitCount; // useless?
                Special = *((SpecialClass*)&packet_1->GameOptions.Special);
                GameParams = packet_1->GameOptions.GameParams;
                WDTGameTimer.Set(packet_1->GameOptions.GameTime, TRUE);
                GameOptionsBitfield = packet_1->GameOptions.Bit2_1;

                for (i = 0; i < 4; i++) {
                    TeamScores[i] = packet_1->GameOptions.TeamScore[i];
                }

                if (DebugLogTeams) {
                    char buff[300];
                    sprintf(buff, "*Receiving TeamScore in PACKET_GAME_OPTIONS during Wait_For_WDT_Connection\n");
                    CCDebugString(buff);
                }

                for (i = 0; i < 25; i++) {
                    char buff[300];
                    Weapons[i].Attack = packet_1->GameOptions.WeaponAttack[i];
                    Weapons[i].ROF = packet_1->GameOptions.WeaponROF[i];
                    Weapons[i].Range = packet_1->GameOptions.WeaponRange[i];
                    sprintf(
                        buff, "W %02d:\t\t%d\t\t%d\t\t%d\n", i, Weapons[i].Attack, Weapons[i].ROF, Weapons[i].Range);
                    CCDebugString(buff);
                }

                if (MPlayerTiberium) {
                    Special.IsTGrowth = true;
                    Special.IsTSpread = true;
                } else {
                    Special.IsTGrowth = false;
                    Special.IsTSpread = false;
                }

                ScenarioIdx = -1;

                for (i = 0; i < MPlayerFilenum.Count(); i++) {
                    if (packet_1->GameOptions.Scenario == MPlayerFilenum[i]) {
                        ScenarioIdx = i;
                    }
                }

                if (ScenarioIdx == -1) {
                    CCMessageBox().Process(TXT_SCENARIO_NOT_FOUND);
                    rc = 1;
                }

                process = false;
                ReliableProtocols[0]->Queue->UnQueue_Receive(0, 0, 0);
                break;

            default:
                break;
            }
        }
    }

    Hide_Mouse();
    Load_Title_Screen("HTITLE.PCX", &UnknownViewport2, Palette);
    Set_Palette(Palette);
    UnknownViewport2.Blit(UnknownViewport1);
    Show_Mouse();

    if (rc) {
        return false;
    }

    return true;
}

#if 1
bool Unit_Choice_Dialog(void)
{
    enum
    {
        BTN_WDITH = 90,
        BTN_HEIGHT = 18,

        D_NAME_X = 270,
        D_NAME_Y = 42,
        D_NAME_W = 100,
        D_NAME_H = 18,

        /*
		** Side button dimensions
		*/
        D_GDI_X = 185,
        D_GDI_Y = 60,
        D_GDI_W = BTN_WDITH,
        D_GDI_H = BTN_HEIGHT,

        D_NOD_X = D_GDI_X + BTN_WDITH,
        D_NOD_Y = D_GDI_Y,
        D_NOD_W = BTN_WDITH,
        D_NOD_H = BTN_HEIGHT,

        D_SPEC_X = D_NOD_X + BTN_WDITH,
        D_SPEC_Y = D_GDI_Y,
        D_SPEC_W = BTN_WDITH,
        D_SPEC_H = BTN_HEIGHT,

        D_TEAM1_X = 140,
        D_TEAM1_Y = 78,
        D_TEAM1_W = BTN_WDITH,
        D_TEAM1_H = BTN_HEIGHT,

        D_TEAM2_X = D_TEAM1_X + BTN_WDITH,
        D_TEAM2_Y = D_TEAM1_Y,
        D_TEAM2_W = BTN_WDITH,
        D_TEAM2_H = BTN_HEIGHT,

        D_TEAM3_X = D_TEAM2_X + BTN_WDITH,
        D_TEAM3_Y = D_TEAM1_Y,
        D_TEAM3_W = BTN_WDITH,
        D_TEAM3_H = BTN_HEIGHT,

        D_TEAM4_X = D_TEAM3_X + BTN_WDITH,
        D_TEAM4_Y = D_TEAM1_Y,
        D_TEAM4_W = BTN_WDITH,
        D_TEAM4_H = BTN_HEIGHT,

        D_OK_X = 225,
        D_OK_Y = 368,
        D_OK_W = BTN_WDITH,
        D_OK_H = BTN_HEIGHT,

        D_CANCEL_X = D_OK_X + BTN_WDITH + 10,
        D_CANCEL_Y = D_OK_Y,
        D_CANCEL_W = BTN_WDITH,
        D_CANCEL_H = BTN_HEIGHT,
    };

    int cameos_per_row = 6;
    int pixels_per_cameo = (640 - (cameos_per_row << 6)) / (cameos_per_row + 1);
    int offset = 14;
    int choice_start_x = pixels_per_cameo + 64;
    int choice_start_y = offset + 48;

    bool cancelled = false;
    bool set_focus = true;

    int choice_rect_x1 = 0;
    int choice_rect_y1 = 0;
    int choice_rect_x2 = 0;
    int choice_rect_y2 = 0;

    int index;

    int mouse_x;
    int mouse_y;

    struct UnitChoice
    {
        RTTIType RTTI;
        int Type;
        int Unk;
    };

    UnitChoice gdi_units[16] = {
        {RTTI_INFANTRY, INFANTRY_E1, 1},
        {RTTI_INFANTRY, INFANTRY_E2, 1},
        {RTTI_INFANTRY, INFANTRY_E3, 1},
        {RTTI_INFANTRY, INFANTRY_E5, 1},
        {RTTI_INFANTRY, INFANTRY_RAMBO, 1},
        {RTTI_UNIT, UNIT_HTANK, 1},
        {RTTI_UNIT, UNIT_MTANK, 1},
        {RTTI_UNIT, UNIT_APC, 1},
        {RTTI_UNIT, UNIT_MLRS, 1},
        {RTTI_UNIT, UNIT_JEEP, 1},
        {RTTI_UNIT, UNIT_MSAM, 1},
        {RTTI_UNIT, UNIT_VICE, 1},
        {RTTI_UNIT, UNIT_TRIC, 1},
        {RTTI_UNIT, UNIT_TREX, 1},
        {RTTI_UNIT, UNIT_RAPT, 1},
        {RTTI_UNIT, UNIT_STEG, 1},
    };

    UnitChoice nod_units[17] = {
        {RTTI_INFANTRY, INFANTRY_E1, 1},
        {RTTI_INFANTRY, INFANTRY_E3, 1},
        {RTTI_INFANTRY, INFANTRY_E4, 1},
        {RTTI_INFANTRY, INFANTRY_E5, 1},
        {RTTI_INFANTRY, INFANTRY_RAMBO, 1},
        {RTTI_UNIT, UNIT_LTANK, 1},
        {RTTI_UNIT, UNIT_STANK, 1},
        {RTTI_UNIT, UNIT_FTANK, 1},
        {RTTI_UNIT, UNIT_BUGGY, 1},
        {RTTI_UNIT, UNIT_ARTY, 1},
        {RTTI_UNIT, UNIT_MSAM, 1},
        {RTTI_UNIT, UNIT_BIKE, 1},
        {RTTI_UNIT, UNIT_VICE, 1},
        {RTTI_UNIT, UNIT_TRIC, 1},
        {RTTI_UNIT, UNIT_TREX, 1},
        {RTTI_UNIT, UNIT_RAPT, 1},
        {RTTI_UNIT, UNIT_STEG, 1},
    };

    UnitChoice all_choices[23] = {
        {RTTI_INFANTRY, INFANTRY_E1, 1}, {RTTI_INFANTRY, INFANTRY_E2, 1}, {RTTI_INFANTRY, INFANTRY_E3, 1},
        {RTTI_INFANTRY, INFANTRY_E4, 1}, {RTTI_INFANTRY, INFANTRY_E5, 1}, {RTTI_INFANTRY, INFANTRY_RAMBO, 1},
        {RTTI_UNIT, UNIT_HTANK, 1},      {RTTI_UNIT, UNIT_MTANK, 1},      {RTTI_UNIT, UNIT_LTANK, 1},
        {RTTI_UNIT, UNIT_STANK, 1},      {RTTI_UNIT, UNIT_FTANK, 1},      {RTTI_UNIT, UNIT_APC, 1},
        {RTTI_UNIT, UNIT_MLRS, 1},       {RTTI_UNIT, UNIT_JEEP, 1},       {RTTI_UNIT, UNIT_MSAM, 1},
        {RTTI_UNIT, UNIT_BUGGY, 1},      {RTTI_UNIT, UNIT_ARTY, 1},       {RTTI_UNIT, UNIT_BIKE, 1},
        {RTTI_UNIT, UNIT_VICE, 1},       {RTTI_UNIT, UNIT_TRIC, 1},       {RTTI_UNIT, UNIT_TREX, 1},
        {RTTI_UNIT, UNIT_RAPT, 1},       {RTTI_UNIT, UNIT_STEG, 1},
    };

    /*........................................................................
	Button enumerations:
	........................................................................*/
    enum
    {
        BUTTON_NAME = 100,
        BUTTON_AI_SLIDER,
        BUTTON_GDI,
        BUTTON_NOD,
        BUTTON_SPECTATOR,
        BUTTON_TEAM1,
        BUTTON_TEAM2,
        BUTTON_TEAM3,
        BUTTON_TEAM4,
        BUTTON_OK,
        BUTTON_CANCEL,
    };

    enum RedrawType
    {
        REDRAW_NONE = 0,
        REDRAW_CHOICES,
        REDRAW_BUTTONS,
        REDRAW_BACKGROUND,
        REDRAW_ALL = REDRAW_BACKGROUND
    };

    UnitChoice* choices;
    int choice_count;

    SliderClass ai_slider(BUTTON_AI_SLIDER, 212, 89, 216, 15);

    if (OfflineMode) {
        choice_count = ARRAY_SIZE(all_choices);
        choices = all_choices;
        MPlayerHouse = HOUSE_BLUE_TEAM;
        ai_slider.Set_Maximum(11);
        ai_slider.Set_Thumb_Size(1);
        ai_slider.Set_Value(Options.AISlider);
        GameParams.AIUnitsPer10min = 20 * Options.AISlider;

        if (GameParams.AIUnitsPer10min > 120) {
            GameParams.AIUnitsPer10min = 120;
        }

        GameParams.MaxAIUnits = Options.AISlider;
        GameParams.AIBuildingsPer10min = GameParams.AIUnitsPer10min;
        GameParams.MaxAIBuildings = Options.AISlider / 3;
    }

    /*........................................................................
	Buttons
	........................................................................*/
    ControlClass* commands = NULL; // the button list

    EditClass name_edt(BUTTON_NAME,
                       MPlayerName,
                       10,
                       TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                       D_NAME_X,
                       D_NAME_Y,
                       D_NAME_W,
                       D_NAME_H,
                       EditClass::ALPHANUMERIC);

    TextButtonClass gdibtn(BUTTON_GDI,
                           TXT_G_D_I,
                           TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                           D_GDI_X,
                           D_GDI_Y,
                           D_GDI_W,
                           D_GDI_H);

    TextButtonClass nodbtn(BUTTON_NOD,
                           TXT_N_O_D,
                           TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                           D_NOD_X,
                           D_NOD_Y,
                           D_NOD_W,
                           D_NOD_H);

    TextButtonClass obsbtn(BUTTON_SPECTATOR,
                           TXT_SPECTATOR,
                           TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                           D_SPEC_X,
                           D_SPEC_Y,
                           D_SPEC_W,
                           D_SPEC_H);

    TextButtonClass team1btn(BUTTON_TEAM1,
                             TXT_TEAM1,
                             TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                             D_TEAM1_X,
                             D_TEAM1_Y,
                             D_TEAM1_W,
                             D_TEAM1_H);

    TextButtonClass team2btn(BUTTON_TEAM2,
                             TXT_TEAM2,
                             TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                             D_TEAM2_X,
                             D_TEAM2_Y,
                             D_TEAM2_W,
                             D_TEAM2_H);

    TextButtonClass team3btn(BUTTON_TEAM3,
                             TXT_TEAM3,
                             TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                             D_TEAM3_X,
                             D_TEAM3_Y,
                             D_TEAM3_W,
                             D_TEAM3_H);

    TextButtonClass team4btn(BUTTON_TEAM4,
                             TXT_TEAM4,
                             TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                             D_TEAM4_X,
                             D_TEAM4_Y,
                             D_TEAM4_W,
                             D_TEAM4_H);

    TextButtonClass okbtn(
        BUTTON_OK, TXT_OK, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW, D_OK_X, D_OK_Y, D_OK_W, D_OK_H);

    TextButtonClass cancelbtn(BUTTON_CANCEL,
                              TXT_CANCEL,
                              TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                              D_CANCEL_X,
                              D_CANCEL_Y,
                              D_CANCEL_W,
                              D_CANCEL_H);

    Set_Logic_Page(UnknownViewport1);
    Read_MultiPlayer_Settings();
    name_edt.Set_Text(MPlayerName, 10);

#if 0
	switch (MPlayerHouse) {
		case HOUSE_GOOD:
			gdibtn.Turn_On();
			break;
		case HOUSE_BAD:
			nodbtn.Turn_On();
			break;
		case HOUSE_SPECTATOR:
			obsbtn.Turn_On();
			break;
		case HOUSE_BLUE_TEAM:
			team1btn.Turn_On();
			break;
		case HOUSE_ORANGE_TEAM:
			team2btn.Turn_On();
			break;
		case HOUSE_GREEN_TEAM:
			team3btn.Turn_On();
			break;
		case HOUSE_GREY_TEAM:
			team4btn.Turn_On();
			break;
	}
#endif
    if (MPlayerHouse == HOUSE_GOOD) {
        gdibtn.Turn_On();
    } else if (MPlayerHouse == HOUSE_BAD) {
        nodbtn.Turn_On();
    } else if (MPlayerHouse == HOUSE_SPECTATOR) {
        obsbtn.Turn_On();
    } else if (MPlayerHouse == HOUSE_BLUE_TEAM) {
        team1btn.Turn_On();
    } else if (MPlayerHouse == HOUSE_ORANGE_TEAM) {
        team2btn.Turn_On();
    } else if (MPlayerHouse == HOUSE_GREEN_TEAM) {
        team3btn.Turn_On();
    } else if (MPlayerHouse == HOUSE_GREY_TEAM) {
        team4btn.Turn_On();
    }

    commands = &name_edt;

    if (!OfflineMode) {
        gdibtn.Add_Tail(*commands);
        nodbtn.Add_Tail(*commands);
        obsbtn.Add_Tail(*commands);
        team1btn.Add_Tail(*commands);
        team2btn.Add_Tail(*commands);
        team3btn.Add_Tail(*commands);
        team4btn.Add_Tail(*commands);
    } else {
        ai_slider.Add_Tail(*commands);
    }

    okbtn.Add_Tail(*commands);
    cancelbtn.Add_Tail(*commands);

    Hide_Mouse();
    Set_Palette(BlackPalette);
    VisiblePage.Clear();
    Mem_Copy(MixFileClass::Retrieve("TEMPERAT.PAL"), GamePalette, 768);
    Set_Palette(GamePalette);
    InMainLoop = true;
    Show_Mouse();

    if (Map.Theater != THEATER_TEMPERATE) {
        Reset_Theater_Shapes();
    }

    Map.Theater = THEATER_TEMPERATE;
    Map.Init(THEATER_TEMPERATE);
    TerrainTypeClass::Init(THEATER_TEMPERATE);
    TemplateTypeClass::Init(THEATER_TEMPERATE);
    OverlayTypeClass::Init(THEATER_TEMPERATE);
    UnitTypeClass::Init(THEATER_TEMPERATE);
    InfantryTypeClass::Init(THEATER_TEMPERATE);
    BuildingTypeClass::Init(THEATER_TEMPERATE);
    BulletTypeClass::Init(THEATER_TEMPERATE);
    AnimTypeClass::Init(THEATER_TEMPERATE);
    AircraftTypeClass::Init(THEATER_TEMPERATE);
    SmudgeTypeClass::Init(THEATER_TEMPERATE);
    LastTheater = THEATER_TEMPERATE;

    static int _current_choice;

    RedrawType display = REDRAW_ALL;
    bool process = true;

    while (process) {
        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = false;
            display = REDRAW_BACKGROUND;
        }

        Call_Back();
        Wait_Vert_Blank();
        Set_Palette(GamePalette);
        Sleep(20);

        /*
		** .................... Refresh display if needed ......................
		*/
        if (display) {
            Hide_Mouse();

            if (display >= REDRAW_BACKGROUND) {
                Dialog_Box(0, 0, 640, 400);
                Draw_Caption(TXT_NONE, 0, 0, 640);
                Fancy_Text_Print(TXT_CHOOSE_IDENTITY,
                                 320,
                                 14,
                                 CC_GREEN,
                                 TBLACK,
                                 TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

                if (OfflineMode) {
                    Fancy_Text_Print(Text_String(TXT_AI_OPPONENTS_COLON),
                                     320,
                                     70,
                                     CC_GREEN,
                                     TBLACK,
                                     TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
                    Fancy_Text_Print(Text_String(TXT_MIN_ZERO),
                                     207,
                                     89,
                                     CC_GREEN,
                                     TBLACK,
                                     TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
                    Fancy_Text_Print(Text_String(TXT_MAX_10),
                                     String_Pixel_Width(Text_String(TXT_MAX_DOT)) + 454,
                                     89,
                                     CC_GREEN,
                                     TBLACK,
                                     TPF_RIGHT | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
                }
            }

            if (display >= REDRAW_CHOICES) {
#if 0
				switch (MPlayerHouse) {
					case HOUSE_GOOD:
						choice_count = ARRAY_SIZE(gdi_units);
						units = gdi_units;
						break;
					case HOUSE_BAD:
						choice_count = ARRAY_SIZE(nod_units);
						units = nod_units;
						break;
					case HOUSE_SPECTATOR:
						choice_count = 0;
						units = NULL;
						break;
					case HOUSE_BLUE_TEAM:
					case HOUSE_ORANGE_TEAM:
					case HOUSE_GREEN_TEAM:
					case HOUSE_GREY_TEAM: // Fallthrough for teams
						choice_count = ARRAY_SIZE(all_units);
						units = all_units;
						break;
				}
#endif
                if (MPlayerHouse == HOUSE_GOOD) {
                    choice_count = ARRAY_SIZE(gdi_units);
                    choices = gdi_units;
                } else if (MPlayerHouse == HOUSE_BAD) {
                    choice_count = ARRAY_SIZE(nod_units);
                    choices = nod_units;
                } else if (MPlayerHouse == HOUSE_SPECTATOR) {
                    choice_count = 0;
                    choices = NULL;
                } else if (MPlayerHouse == HOUSE_BLUE_TEAM || MPlayerHouse == HOUSE_ORANGE_TEAM
                           || MPlayerHouse == HOUSE_GREEN_TEAM || MPlayerHouse == HOUSE_GREY_TEAM) {
                    choice_count = ARRAY_SIZE(all_choices);
                    choices = all_choices;
                }

                if (choice_rect_x2 > choice_rect_x1) {
                    LogicPage->Fill_Rect(
                        choice_rect_x1 - 3, choice_rect_y1 - 3, choice_rect_x2 + 3, choice_rect_y2 + 3, BLACK);
                }

                if (choice_count > 0) {
                    if (MPlayerHouse == HOUSE_ADMIN) {
                        cameos_per_row = 8;
                    } else {
                        cameos_per_row = 6;
                    }
                    pixels_per_cameo = (640 - (cameos_per_row * 64)) / (cameos_per_row + 1);
                    offset = 14;
                    choice_start_x = pixels_per_cameo + 64;
                    choice_start_y = offset + 48;
                    int row_count = ((cameos_per_row - 1) + choice_count) / cameos_per_row;
                    choice_rect_x1 = 320 - cameos_per_row * choice_start_x / 2;
                    choice_rect_x2 = cameos_per_row * choice_start_x + choice_rect_x1;
                    choice_rect_y1 = 214 - choice_start_y * row_count / 2;
                    choice_rect_y1 += 32;
                    choice_rect_y2 = choice_start_y * row_count + choice_rect_y1;

                    for (index = 0; index < choice_count; index++) {
                        mouse_x = choice_start_x * (index % cameos_per_row) + choice_rect_x1 + pixels_per_cameo / 2;
                        mouse_y = choice_start_y * (index / cameos_per_row) + choice_rect_y1;
                        Draw_Choice_Entry(choices[index].RTTI,
                                          choices[index].Type,
                                          mouse_x,
                                          mouse_y,
                                          MPlayerHouse,
                                          choices[index].Unk,
                                          false);
                    }

                    mouse_x =
                        choice_start_x * (_current_choice % cameos_per_row) + choice_rect_x1 + pixels_per_cameo / 2;
                    mouse_y = choice_start_y * (_current_choice / cameos_per_row) + choice_rect_y1;
                    LogicPage->Draw_Rect(mouse_x - 3, mouse_y - 3, mouse_x + 66, mouse_y + 50, WHITE);
                    LogicPage->Draw_Rect(mouse_x - 2, mouse_y - 2, mouse_x + 65, mouse_y + 49, LTGREY);
                    LogicPage->Draw_Rect(mouse_x - 1, mouse_y - 1, mouse_x + 64, mouse_y + 48, GREY);
                } else {
                    choice_rect_x1 = 0;
                    choice_rect_y1 = 0;
                    choice_rect_x2 = 0;
                    choice_rect_y2 = 0;
                }
            }

            if (display >= REDRAW_BUTTONS) {
                commands->Draw_All();
            }

            Show_Mouse();
            display = REDRAW_NONE;
        }

        /*
		........................... Get user input ............................
		*/
        int input = commands->Input();

        if (set_focus) {
            name_edt.Set_Focus();
            name_edt.Flag_To_Redraw();
            input = commands->Input();
            set_focus = false;
        }

        switch (input) {
        case KN_DOWN:
            break;

        case KN_UP:
            break;

        case (BUTTON_GDI | KN_BUTTON):
            MPlayerHouse = HOUSE_GOOD;
            gdibtn.Turn_On();
            nodbtn.Turn_Off();
            obsbtn.Turn_Off();
            team1btn.Turn_Off();
            team2btn.Turn_Off();
            team3btn.Turn_Off();
            team4btn.Turn_Off();

            //bug?
            if ((unsigned)_current_choice > ARRAY_SIZE(gdi_units) - 1) {
                _current_choice = ARRAY_SIZE(gdi_units) - 1;
            }

            display = REDRAW_CHOICES;
            break;

        case (BUTTON_NOD | KN_BUTTON):
            MPlayerHouse = HOUSE_BAD;
            gdibtn.Turn_Off();
            nodbtn.Turn_On();
            obsbtn.Turn_Off();
            team1btn.Turn_Off();
            team2btn.Turn_Off();
            team3btn.Turn_Off();
            team4btn.Turn_Off();

            //bug?
            if ((unsigned)_current_choice > ARRAY_SIZE(nod_units) - 1) {
                _current_choice = ARRAY_SIZE(nod_units) - 1;
            }

            display = REDRAW_CHOICES;
            break;

        case (BUTTON_SPECTATOR | KN_BUTTON):
            MPlayerHouse = HOUSE_SPECTATOR;
            gdibtn.Turn_Off();
            nodbtn.Turn_Off();
            obsbtn.Turn_On();
            team1btn.Turn_Off();
            team2btn.Turn_Off();
            team3btn.Turn_Off();
            team4btn.Turn_Off();
            _current_choice = 0;
            display = REDRAW_CHOICES;
            break;

        case (BUTTON_TEAM1 | KN_BUTTON):
            MPlayerHouse = HOUSE_BLUE_TEAM;
            gdibtn.Turn_Off();
            nodbtn.Turn_Off();
            obsbtn.Turn_Off();
            team1btn.Turn_On();
            team2btn.Turn_Off();
            team3btn.Turn_Off();
            team4btn.Turn_Off();
            _current_choice = 0;
            display = REDRAW_CHOICES;
            break;

        case (BUTTON_TEAM2 | KN_BUTTON):
            MPlayerHouse = HOUSE_ORANGE_TEAM;
            gdibtn.Turn_Off();
            nodbtn.Turn_Off();
            obsbtn.Turn_Off();
            team1btn.Turn_Off();
            team2btn.Turn_On();
            team3btn.Turn_Off();
            team4btn.Turn_Off();
            _current_choice = 0;
            display = REDRAW_CHOICES;
            break;

        case (BUTTON_TEAM3 | KN_BUTTON):
            MPlayerHouse = HOUSE_GREEN_TEAM;
            gdibtn.Turn_Off();
            nodbtn.Turn_Off();
            obsbtn.Turn_Off();
            team1btn.Turn_Off();
            team2btn.Turn_Off();
            team3btn.Turn_On();
            team4btn.Turn_Off();
            _current_choice = 0;
            display = REDRAW_CHOICES;
            break;

        case (BUTTON_TEAM4 | KN_BUTTON):
            MPlayerHouse = HOUSE_GREY_TEAM;
            gdibtn.Turn_Off();
            nodbtn.Turn_Off();
            obsbtn.Turn_Off();
            team1btn.Turn_Off();
            team2btn.Turn_Off();
            team3btn.Turn_Off();
            team4btn.Turn_On();
            _current_choice = 0;
            display = REDRAW_CHOICES;
            break;

        case (BUTTON_AI_SLIDER | KN_BUTTON):
            Options.AISlider = ai_slider.Get_Value();
            GameParams.AIUnitsPer10min = 20 * Options.AISlider;
            if (GameParams.AIUnitsPer10min > 120) {
                GameParams.AIUnitsPer10min = 120;
            }
            GameParams.MaxAIUnits = Options.AISlider;
            GameParams.AIBuildingsPer10min = GameParams.AIUnitsPer10min;
            GameParams.MaxAIBuildings = Options.AISlider / 3;
            break;

        case (BUTTON_OK | KN_BUTTON):
        case KN_RETURN: // Fallthough
            cancelled = false;
            process = false;
            break;

        case (BUTTON_CANCEL | KN_BUTTON):
        case KN_ESC:
            Options.Save_Settings();
            cancelled = true;
            process = false;
            break;

        case (KN_LMOUSE | WWKEY_RLS_BIT): {
            mouse_x = Get_Mouse_X();
            mouse_y = Dialog_Get_Mouse_Y();

            if (mouse_x > choice_rect_x1 && mouse_x < choice_rect_x2 && mouse_y > choice_rect_y1
                && mouse_y < choice_rect_y2) {
                index = _current_choice;
                _current_choice = (mouse_y - choice_rect_y1) / choice_start_y * cameos_per_row
                                  + (mouse_x - choice_rect_x1) / choice_start_x;

                if (_current_choice < 0) {
                    _current_choice = 0;
                } else if (choice_count - 1 < _current_choice) {
                    _current_choice = choice_count - 1;
                }

                if (_current_choice != index) {
                    Hide_Mouse();
                    mouse_x = choice_start_x * (index % cameos_per_row) + choice_rect_x1 + pixels_per_cameo / 2;
                    mouse_y = choice_start_y * (index / cameos_per_row) + choice_rect_y1;
                    LogicPage->Draw_Rect(mouse_x - 3, mouse_y - 3, mouse_x + 66, mouse_y + 50, 12);
                    LogicPage->Draw_Rect(mouse_x - 2, mouse_y - 2, mouse_x + 65, mouse_y + 49, 12);
                    LogicPage->Draw_Rect(mouse_x - 1, mouse_y - 1, mouse_x + 64, mouse_y + 48, 12);
                    mouse_x =
                        choice_start_x * (_current_choice % cameos_per_row) + choice_rect_x1 + pixels_per_cameo / 2;
                    mouse_y = choice_start_y * (_current_choice / cameos_per_row) + choice_rect_y1;
                    LogicPage->Draw_Rect(mouse_x - 3, mouse_y - 3, mouse_x + 66, mouse_y + 50, 15);
                    LogicPage->Draw_Rect(mouse_x - 2, mouse_y - 2, mouse_x + 65, mouse_y + 49, 14);
                    LogicPage->Draw_Rect(mouse_x - 1, mouse_y - 1, mouse_x + 64, mouse_y + 48, 13);
                    Show_Mouse();
                    //break;
                }
            }
            break;
        }
        }
    }

    Hide_Mouse();
    Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);
    Set_Palette(BlackPalette);
    VisiblePage.Clear();

    if (cancelled) {
        Load_Title_Screen("HTITLE.PCX", &HidPage, Palette);
        Set_Palette(Palette);
        Mem_Copy(Palette, GamePalette, 768);
        UnknownViewport2.Blit(UnknownViewport1);
    }

    InMainLoop = false;
    Show_Mouse();

    if (cancelled) {
        return false;
    }

    if (choice_count > 0) {
        Chosen_RTTI = choices[_current_choice].RTTI;
        Chosen_Type = choices[_current_choice].Type;
    } else {
        Chosen_RTTI = RTTI_NONE;
        Chosen_Type = 0;
    }
    if (Chosen_RTTI == RTTI_AIRCRAFTTYPE && (Chosen_Type == AIRCRAFT_ORCA || Chosen_Type == AIRCRAFT_HELICOPTER)) {
        Chosen_RTTI = RTTI_BUILDINGTYPE;
        Chosen_Type = STRUCT_HELIPAD;
    }
    Write_MultiPlayer_Settings();
    return true;
}
#endif

void Draw_Choice_Entry(RTTIType rtti, int type, int xpos, int ypos, HousesType house, int unused, bool draw_name)
{

    ShapeFlags_Type flags = SHAPE_FADING;
    const TechnoTypeClass* techno = Fetch_Techno_Type(rtti, type);
    if (techno) {
        if (draw_name) {
            Fancy_Text_Print(techno->Full_Name(),
                             xpos + 32,
                             ypos + 50,
                             7,
                             0,
                             TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
        }

        int frame = 0;
        const void* image = techno->Get_Cameo_Data();

        int draw_X;
        int draw_Y;

        if (!image) {
            image = techno->Get_Image_Data();
            draw_X = xpos + 32;
            draw_Y = ypos + 24;
            flags |= SHAPE_CENTER;
            frame = 0;
        } else {
            draw_X = xpos;
            draw_Y = ypos;
        }

        CC_Draw_Shape(image, frame, draw_X, draw_Y, WINDOW_MAIN, flags, HouseTypeClass::As_Reference(house).RemapTable);
    }
}

bool Read_WDT_Settings(void)
{
    Read_MultiPlayer_Settings();
    return true;
}

bool Wait_For_Options(void)
{
    typedef enum
    {
        REDRAW_NONE = 0,
        REDRAW_BUTTONS,
        REDRAW_BACKGROUND,
        REDRAW_ALL = REDRAW_BACKGROUND
    } RedrawType;

    enum
    {
        BUTTON_CANCEL = 100,
    };

    RedrawType display;
    bool process;
    KeyNumType input;
    int rc = 0;
    GadgetClass* commands = NULL;
    HWND handle;

    TextButtonClass cancel_btn(
        BUTTON_CANCEL, TXT_CANCEL, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW, 275, 209, 90, 18);
    Read_MultiPlayer_Settings();
    Set_Logic_Page(UnknownViewport1);
    commands = &cancel_btn;
    Hide_Mouse();
    Load_Title_Screen("HTITLE.PCX", &UnknownViewport2, Palette);
    Set_Palette(Palette);
    UnknownViewport2.Blit(UnknownViewport1);
    Show_Mouse();
    display = REDRAW_ALL;
    process = true;

    while (process) {
        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = false;
            display = REDRAW_BACKGROUND;
        }

        Call_Back();

        if (display) {
            Hide_Mouse();

            if (display >= REDRAW_BACKGROUND) {
                Dialog_Box(56, 159, 528, 82);
                Draw_Caption(TXT_NONE, 56, 159, 528);
                Fancy_Text_Print(TXT_WAITING_FOR_OPTIONS,
                                 320,
                                 168,
                                 GREEN,
                                 TBLACK,
                                 TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
                Fancy_Text_Print("Please quit Westwood OnLine first if you wish to Cancel.",
                                 320,
                                 183,
                                 GREEN,
                                 TBLACK,
                                 TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
            }

            if (display >= REDRAW_BUTTONS) {
                commands->Draw_All();
            }

            Show_Mouse();
            display = REDRAW_NONE;
        }

        input = commands->Input();
        handle = Get_WChat_Handle();

        if (handle == NULL) {
            rc = 1;
            process = false;
            SpawnedFromWChat = false;
        }

        switch (input) {
        case (KN_ESC): // fallthrough
        case (BUTTON_CANCEL | KN_BUTTON):
            rc = 1;
            process = false;
            break;
        default:
            break;
        }

        if (Read_Game_Options(NULL) != 0) {
            break;
        }
    }

    Set_Palette(BlackPalette);
    VisiblePage.Clear();
    ShowWindow(MainWindow, 9);
    Hide_Mouse();
    Load_Title_Screen("HTITLE.PCX", &UnknownViewport2, Palette);
    Set_Palette(Palette);
    Show_Mouse();

    if (rc) {
        return false;
    }

    return true;
}

bool Send_Post_Game_Stats(void)
{
#if 0
	struct StatsPacket {
		int version;
		char player_name[12];
		int unused1;
		int unused2;
		int score;
		int rtti;
		int type;
		int connection_lost;
		int timing;
	};

	static int stats_packet_size;
	
	PacketClass stats;
	
	StatsPacket stat;
	stat.version = Version_Number();
	strcpy(stat.player_name, MPlayerName);
	stat.score = PlayerPtr->Int2;
	stat.rtti = Chosen_RTTI;
	stat.type = Chosen_Type;
	stat.connection_lost = ServerConnectionLost;
	stat.timing = ColorListTiming;

	stats.Add_Field("STAT", &stat, sizeof(StatsPacket));
	char *data;
	
	data = stats.Create_Comms_Packet(stats_packet_size);
	Send_Data_To_DDE_Server(data, stats_packet_size, DDEServerClass::DDE_PACKET_GAME_RESULTS);
	delete[] data;
#endif

    return true;
}

void Wait_For_Player_Dialog(int is_waiting, char* waiting_on, int time_allowed)
{
    static int XPos;
    static int YPos;
    static int Width;
    static int Height;

    char wait_str[40] = {0};
    char time_str[40] = {0};
    int factor = UnknownViewport1.Get_Width() == 320 ? 1 : 2;
    int y = 6 * factor + 1;
    int x = 5 * factor;

    if (is_waiting) {
        Fancy_Text_Print("", 0, 0, GREEN, TBLACK, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
        sprintf(wait_str, Text_String(TXT_WAITING_ON), waiting_on);
        sprintf(time_str, Text_String(TXT_TIME_ALLOWED_D), time_allowed + 1);
        Width = MAX(String_Pixel_Width(wait_str), String_Pixel_Width(time_str));
        Width += 2 * x;
        Height = (y * 3) + (x * 6);
        XPos = 160 * factor - Width / 2;
        YPos = 100 * factor - Height / 2;
        Hide_Mouse();
        Set_Logic_Page(UnknownViewport1);
        Dialog_Box(XPos, YPos, Width, Height);
        Fancy_Text_Print(wait_str,
                         160 * factor,
                         YPos + 2 * x,
                         GREEN,
                         BLACK,
                         TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
        Fancy_Text_Print(time_str,
                         160 * factor,
                         2 * x + YPos + y + x,
                         GREEN,
                         BLACK,
                         TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
        Show_Mouse();
    } else {
        Hide_Mouse();
        Set_Logic_Page(UnknownViewport1);
        sprintf(time_str, Text_String(TXT_TIME_ALLOWED_D), time_allowed + 1);
        Fancy_Text_Print(time_str,
                         160 * factor,
                         2 * x + YPos + y + x,
                         GREEN,
                         BLACK,
                         TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
        Show_Mouse();
    }
}

void Select_Voice_Dialog(void)
{
    // Not quite matching, stack not right.
    typedef enum
    {
        REDRAW_NONE = 0,
        REDRAW_BUTTONS,
        REDRAW_BACKGROUND,
        REDRAW_ALL = REDRAW_BACKGROUND
    } RedrawType;

    enum
    {
        BUTTON_OK = 100,
        BUTTON_CANCEL,
        BUTTON_TEST,
        BUTTON_VOICE_LIST,
    };

    RedrawType display;
    bool process;
    KeyNumType input;
    int rc = 0;
    int i;
    char* str;
    int current_theme;

    GadgetClass* commands = NULL;

    TextButtonClass ok_btn(
        BUTTON_OK, TXT_OK, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW, 225, 278, 90, 18);
    TextButtonClass cancel_btn(
        BUTTON_CANCEL, TXT_CANCEL, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW, 325, 278, 90, 18);
    TextButtonClass test_btn(
        BUTTON_TEST, TXT_TEST, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW, 275, 226, 90, 18);
    CheckListClass voice_list(BUTTON_VOICE_LIST,
                              220,
                              140,
                              200,
                              72,
                              TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                              Hires_Retrieve("BTN-UP.SHP"),
                              Hires_Retrieve("BTN-DN.SHP"));

    int voicetheme_tabs[1] = {20};

    Set_Logic_Page(UnknownViewport1);
    voice_list.Set_Tabs(voicetheme_tabs);

    for (i = 0; i < VoiceThemes.Count(); i++) {
        str = new char[0x50];
        sprintf(str, " \t%s", VoiceThemes[i]->Get_Theme_Name());
        voice_list.Add_Item(str);
    }

    if (CurrentVoiceTheme >= VoiceThemes.Count()) {
        CurrentVoiceTheme = VoiceThemes.Count() - 1;
    }

    if (CurrentVoiceTheme < 0) {
        CurrentVoiceTheme = 0;
    }

    voice_list.Check_Item(CurrentVoiceTheme, true);
    voice_list.Set_Selected_Index(CurrentVoiceTheme);
    current_theme = CurrentVoiceTheme;
    commands = &ok_btn;
    cancel_btn.Add_Tail(*commands);
    test_btn.Add_Tail(*commands);
    voice_list.Add_Tail(*commands);
    display = REDRAW_ALL;
    process = true;

    while (process) {
        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = false;
            display = REDRAW_BACKGROUND;
        }

        if (GameToPlay == GAME_NORMAL || OfflineMode) {
            Call_Back();
        } else if (Main_Loop()) {
            process = 0;
        }

        if (display) {
            Hide_Mouse();

            if (display >= REDRAW_BACKGROUND) {
                Dialog_Box(120, 90, 400, 220);
                Draw_Caption(TXT_NONE, 120, 90, 400);
                Fancy_Text_Print(TXT_VOICE_THEMES,
                                 320,
                                 104,
                                 GREEN,
                                 TBLACK,
                                 TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
            }

            if (display >= REDRAW_BUTTONS) {
                commands->Draw_All();
            }

            Show_Mouse();
            display = REDRAW_NONE;
        }

        input = commands->Input();

        switch (input) {
        case (BUTTON_VOICE_LIST | KN_BUTTON):
            voice_list.Check_Item(current_theme, false);
            current_theme = voice_list.Current_Index();
            voice_list.Check_Item(current_theme, true);
            voice_list.Flag_To_Redraw();
            display = REDRAW_BUTTONS;
            break;
        case (BUTTON_TEST | KN_BUTTON):
            VoiceThemes[current_theme]->Play(VOX_THEME_SND_TEST);
            break;
        case (KN_ESC):    // fallthrough
        case (KN_RETURN): // fallthrough
        case (BUTTON_OK | KN_BUTTON):
            rc = 0;
            process = false;
            break;
        case (BUTTON_CANCEL | KN_BUTTON):
            rc = 1;
            process = false;
            break;
        default:
            break;
        }
    }

    if (rc) {
        return;
    }

    CurrentVoiceTheme = current_theme;
    Write_MultiPlayer_Settings();
    return;
}

void Victory_Dialog(HousesType team)
{
    /*........................................................................
	Dialog & button dimensions
	........................................................................*/
    int d_dialog_w = 400;
    int d_dialog_h;
    if (GameParams.NumTeams > 1) {
        d_dialog_h = 230;
    } else {
        d_dialog_h = 120;
    }

    int d_dialog_x = ((640 - d_dialog_w - 160) / 2);
    int d_dialog_y = ((400 - d_dialog_h) / 2);
    int d_dialog_cx = d_dialog_x + (d_dialog_w / 2);

    int d_var2 = 22;
    int d_var1 = 14;

    int d_srnshot_w = 200;
    int d_srnshot_h = 18;
    int d_srnshot_x = d_dialog_cx - (d_srnshot_w / 2);
    int d_srnshot_y = ((d_dialog_y + d_dialog_h) - d_srnshot_h) - d_var1;

    enum
    {
        BUTTON_SCREENSHOT = 100,
        NUM_OF_BUTTONS,
    };

    /*........................................................................
	Redraw values: in order from "top" to "bottom" layer of the dialog
	........................................................................*/
    typedef enum
    {
        REDRAW_NONE = 0,
        REDRAW_BUTTONS,    // includes map interior & coord values
        REDRAW_BACKGROUND, // includes box, map bord, key, coord labels, btns
        REDRAW_ALL = REDRAW_BACKGROUND
    } RedrawType;

    /*........................................................................
	Dialog variables:
	........................................................................*/
    CountDownTimerClass delay;
    KeyNumType input;   // input from user
    bool process;       // loop while true
    RedrawType display; // true = re-draw everything
    char scorestr[200];
    char buf[80];
    int fore;
    char hname[80];

    ControlClass* commands = NULL; // the button list

    TextButtonClass srnshotbtn(BUTTON_SCREENSHOT,
                               Text_String(TXT_TAKE_SCREENSHOT),
                               TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                               d_srnshot_x,
                               d_srnshot_y,
                               d_srnshot_w,
                               d_srnshot_h);

    Set_Logic_Page(UnknownViewport1);
    delay.Set(TICKS_PER_SECOND * 20);
    Sound_Effect(VOC_XPLOBIG7, (VolType)180);

    commands = &srnshotbtn;

    display = REDRAW_ALL;
    process = true;

    while (process) {

        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = FALSE;
            display = REDRAW_ALL;
        }

        Call_Back();

        if (display) {
            Hide_Mouse();
            if (display >= REDRAW_BACKGROUND) {

                Dialog_Box(d_dialog_x, d_dialog_y, d_dialog_w, d_dialog_h);
                Draw_Caption(TXT_NONE, d_dialog_x, d_dialog_y, d_dialog_w);

                Fancy_Text_Print(TXT_GAME_OVER,
                                 d_dialog_cx,
                                 d_dialog_y + d_var1,
                                 GREEN,
                                 TBLACK,
                                 TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

                if (team == HOUSE_NONE) {
                    Fancy_Text_Print(TXT_WAS_NO_WINNER,
                                     d_dialog_cx,
                                     2 * d_var1 + d_dialog_y + d_var2,
                                     GREEN,
                                     TBLACK,
                                     TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
                } else if (team == HOUSE_COUNT) {
                    Fancy_Text_Print(TXT_ITS_A_TIE,
                                     d_dialog_cx,
                                     2 * d_var1 + d_dialog_y + d_var2,
                                     GREEN,
                                     TBLACK,
                                     TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
                } else {
                    if (team == HOUSE_BLUE_TEAM) {
                        strcpy(hname, Text_String(TXT_BLUE_TEAM));
                        fore = 219;
                    } else if (team == HOUSE_ORANGE_TEAM) {
                        strcpy(hname, Text_String(TXT_ORANGE_TEAM));
                        fore = 25;
                    } else if (team == HOUSE_GREEN_TEAM) {
                        strcpy(hname, Text_String(TXT_GREEN_TEAM));
                        fore = 3;
                    } else if (team == HOUSE_GREY_TEAM) {
                        strcpy(hname, Text_String(TXT_GREY_TEAM));
                        fore = 210;
                    } else {
                        strcpy(hname, HouseClass::As_Pointer(team)->Name);
                        fore = 3;
                    }

                    if (OfflineMode) {
                        if (OfflinePoints > 0) {
                            sprintf(buf, Text_String(TXT_CONGRATS_YOU_MADE_POINTS), OfflinePoints);
                        } else {
                            sprintf(buf, Text_String(TXT_MADE_NO_POINTS));
                        }

                        Fancy_Text_Print(buf,
                                         d_dialog_cx,
                                         d_dialog_y + d_var1 + d_var2,
                                         3,
                                         0,
                                         TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);

                        if (OfflineDeathCount > 0) {
                            sprintf(buf, Text_String(TXT_KILLED_X_TIMES), OfflineDeathCount);
                        } else {
                            sprintf(buf, Text_String(TXT_NEVER_KILLED));
                        }

                        Fancy_Text_Print(buf,
                                         d_dialog_cx,
                                         2.5 * d_var1 + d_dialog_y + d_var2,
                                         3,
                                         0,
                                         TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
                    } else {
                        sprintf(buf, Text_String((WDT_Random_Pick(0, 4) + TXT_IS_VICTORIOUS)), hname);
                        Fancy_Text_Print(buf,
                                         d_dialog_cx,
                                         2 * d_var1 + d_dialog_y + d_var2,
                                         fore,
                                         0,
                                         TPF_CENTER | TPF_6POINT | TPF_BRIGHT_COLOR | TPF_NOSHADOW);
                    }
                }
            }

            if (GameParams.NumTeams > 1) {
                for (int i = 0; i < GameParams.NumTeams; i++) {
                    Fancy_Text_Print(Text_String((i + TXT_BLUE_TEAM)),
                                     d_dialog_x + 100,
                                     (d_dialog_y + 90) + 23 * i,
                                     MPlayerTColors[i + 2],
                                     0,
                                     TPF_6POINT | TPF_BRIGHT_COLOR | TPF_NOSHADOW);
                    sprintf(scorestr, "%d", TeamScores[i]);
                    Fancy_Text_Print(scorestr,
                                     d_dialog_x + 230,
                                     (d_dialog_y + 90) + 23 * i,
                                     MPlayerTColors[i + 2],
                                     0,
                                     TPF_6POINT | TPF_BRIGHT_COLOR | TPF_NOSHADOW);
                }
            }

            if (display >= REDRAW_BUTTONS) {
                commands->Draw_All();
            }
            Show_Mouse();
            display = REDRAW_NONE;
        }

        input = commands->Input();

        switch (input) {
        case KN_RETURN:
        case KN_ESC: // fallthrough
        case (BUTTON_SCREENSHOT | KN_BUTTON):
            if (Frame) {
                char buf[200];
                char name[32];

                GraphicBufferClass temp(UnknownViewport1.Get_Width(),
                                        UnknownViewport1.Get_Height(),
                                        0,
                                        UnknownViewport1.Get_Width() * UnknownViewport1.Get_Height());
                UnknownViewport1.Blit(temp);
                for (int i = 0; i < 99; i++) {
                    sprintf(name, "SOLE%02d.PCX", i);
                    if (access(name, 0) == -1) {
                        break;
                    }
                }

                Write_PCX_File(name, temp, (unsigned char*)CurrentPalette);
                Sound_Effect(VOC_RAMBO_NOPROB, VOL_FULL);
            }
            break;
        }

        if (!delay.Time()) {
            process = false;
        }
    }
    return;
}

void Secret_Credits_Dialog(void)
{

    /*........................................................................
	Button enumerations
	........................................................................*/
    enum
    {
        BUTTON_OK = 100,
    };

    /*........................................................................
	Redraw values: in order from "top" to "bottom" layer of the dialog
	........................................................................*/
    typedef enum
    {
        REDRAW_NONE = 0,
        REDRAW_BUTTONS,
        REDRAW_BACKGROUND,
        REDRAW_ALL = REDRAW_BACKGROUND
    } RedrawType;

    /*........................................................................
	Dialog variables
	........................................................................*/
    RedrawType display; // requested redraw level
    bool process;       // loop while true
    KeyNumType input;
    int y;
    char str[80];
    int i;

    /*........................................................................
	Buttons
	........................................................................*/
    ControlClass* commands = NULL; // the button list

    TextButtonClass okbtn(
        BUTTON_OK, TXT_OK, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW, 275, 368, 90, 18);

    Set_Logic_Page(UnknownViewport1);

    commands = &okbtn;

    display = REDRAW_ALL;
    process = true;
    while (process) {

        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = FALSE;
            display = REDRAW_ALL;
        }

        if (GameToPlay == GAME_NORMAL || OfflineMode) {
            Call_Back();
        } else {
            if (Main_Loop()) {
                process = false;
            }
        }

        if (display) {

            Hide_Mouse();
            if (display >= REDRAW_BACKGROUND) {
                Dialog_Box(0, 0, 640, 400);
                Draw_Caption(TXT_NONE, 0, 0, 640);
                y = 14;
                for (i = 0; i < 16; i++) {
                    Decode_Credits_Entry(i, str);
                    Fancy_Text_Print(str,
                                     320,
                                     y,
                                     GREEN,
                                     0,
                                     TPF_BRIGHT_COLOR | TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
                    y += 22;
                }
            }

            if (display >= REDRAW_BUTTONS) {
                commands->Draw_All();
            }
            Show_Mouse();
            display = REDRAW_NONE;
        }

        input = commands->Input();

        switch (input) {
        case (KN_RETURN):
        case (BUTTON_OK | KN_BUTTON):
            process = false;
            break;

        default:
            break;
        }
    }

    Map.Flag_To_Redraw(true);
    Map.IsToDrawUnknown = true;
    Map.Render();
    return;
}

void Decode_Credits_Entry(int entry, char* str)
{
    static unsigned char RealSoleCredits[16][80] = {
        0xD3, 0xE7, 0xE4, 0x9F, 0xD1, 0xE4, 0xE0, 0xEB, 0x9F, 0xD2, 0xEE, 0xEB, 0xE4, 0x9F, 0xD2, 0xF4, 0xF1, 0xF5,
        0xE8, 0xF5, 0xEE, 0xF1, 0x9F, 0xC2, 0xF1, 0xE4, 0xE3, 0xE8, 0xF3, 0xF2, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC2, 0xEE, 0xED, 0xE2, 0xE4, 0xEF, 0xF3, 0xB9, 0x9F, 0xC1,
        0xF1, 0xE4, 0xF3, 0xF3, 0x9F, 0xA7, 0xCE, 0xF5, 0xE4, 0xF1, 0xEB, 0xEE, 0xF1, 0xE3, 0xA8, 0x9F, 0xD2, 0xEF,
        0xE4, 0xF1, 0xF1, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCB, 0xE4,
        0xE0, 0xE3, 0x9F, 0xCF, 0xF1, 0xEE, 0xE6, 0xF1, 0xE0, 0xEC, 0xEC, 0xE4, 0xF1, 0x9F, 0xAE, 0x9F, 0xD2, 0xF4,
        0xE5, 0xE5, 0xE4, 0xF1, 0xE4, 0xF1, 0xB9, 0x9F, 0xC1, 0xE8, 0xEB, 0xEB, 0x9F, 0xA7, 0xC1, 0xEB, 0xE0, 0xE2,
        0xEA, 0xC1, 0xEE, 0xEB, 0xF3, 0xA8, 0x9F, 0xD1, 0xE0, 0xED, 0xE3, 0xEE, 0xEB, 0xEF, 0xE7, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCF, 0xF1, 0xEE, 0xE6, 0xF1, 0xE0, 0xEC, 0xEC, 0xE4, 0xF1, 0x9F, 0xAE,
        0x9F, 0xD2, 0xF4, 0xE5, 0xE5, 0xE4, 0xF1, 0xE4, 0xF1, 0xB9, 0x9F, 0xCC, 0xE0, 0xF3, 0xF3, 0x9F, 0xA7, 0xC1,
        0xE4, 0xE0, 0xF2, 0xF3, 0xA8, 0x9F, 0xD3, 0xE7, 0xEE, 0xF1, 0xED, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD6, 0xEE, 0xEE, 0xC2,
        0xE7, 0xE0, 0xF3, 0xC3, 0xF4, 0xE3, 0xE4, 0x9F, 0xAE, 0x9F, 0xCB, 0xE0, 0xF4, 0xE6, 0xE7, 0x9F, 0xD3, 0xF1,
        0xE0, 0xE2, 0xEA, 0xB9, 0x9F, 0xC3, 0xE0, 0xF5, 0xE4, 0x9F, 0xA7, 0xC3, 0xA6, 0xC0, 0xEB, 0xE3, 0xF1, 0xE8,
        0xE3, 0xE6, 0xE4, 0xA8, 0x9F, 0xC0, 0xEB, 0xE3, 0xF1, 0xE8, 0xE3, 0xE6, 0xE4, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xC1, 0xE4, 0xF3, 0xE0, 0x9F, 0xD3, 0xE4, 0xF2, 0xF3, 0xE4, 0xF1, 0x9F, 0xD1, 0xEE,
        0xE3, 0xE4, 0xEE, 0x9F, 0xC2, 0xEB, 0xEE, 0xF6, 0xED, 0xB9, 0x9F, 0xD3, 0xE4, 0xE3, 0x9F, 0xA7, 0xD3, 0xC1,
        0xEE, 0xED, 0xE4, 0xA8, 0x9F, 0xCC, 0xEE, 0xF1, 0xF1, 0xE8, 0xF2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCF, 0xF1, 0xEE, 0xE3, 0xF4, 0xE2,
        0xE4, 0xF1, 0x9F, 0xAE, 0x9F, 0xC2, 0xE7, 0xE4, 0xE4, 0xF1, 0xEB, 0xE4, 0xE0, 0xE3, 0xE4, 0xF1, 0xB9, 0x9F,
        0xCA, 0xE4, 0xED, 0x9F, 0xA7, 0xCE, 0xF8, 0xE0, 0xE1, 0xF4, 0xED, 0xA8, 0x9F, 0xCC, 0xF4, 0xF1, 0xEF, 0xE7,
        0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xC3, 0xE4, 0xF2, 0xE8, 0xE6, 0xED, 0xE4, 0xF1, 0xA7, 0xF2, 0xA8, 0xB9, 0x9F, 0xC9, 0xEE, 0xE7,
        0xED, 0x9F, 0xA7, 0xC9, 0xEE, 0xED, 0xF8, 0xD0, 0xF4, 0xE4, 0xF2, 0xF3, 0xA8, 0x9F, 0xC0, 0xF1, 0xE2, 0xE7,
        0xE4, 0xF1, 0xAB, 0x9F, 0xC1, 0xE8, 0xEB, 0xEB, 0x9F, 0xD1, 0xE0, 0xED, 0xE3, 0xEE, 0xEB, 0xEF, 0xE7, 0xAB,
        0x9F, 0xCC, 0xE0, 0xF3, 0xF3, 0x9F, 0xD3, 0xE7, 0xEE, 0xF1, 0xED, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF1, 0xF3, 0x9F, 0xD2, 0xF4, 0xEF, 0xEF,
        0xEE, 0xF1, 0xF3, 0x9F, 0xAE, 0x9F, 0xD2, 0xE0, 0xF1, 0xE2, 0xE0, 0xF2, 0xEC, 0xB9, 0x9F, 0xCC, 0xE0, 0xF3,
        0xF3, 0x9F, 0xC7, 0xE0, 0xED, 0xF2, 0xE4, 0xEB, 0xAB, 0x9F, 0xC9, 0xEE, 0xF2, 0xE4, 0xEF, 0xE7, 0x9F, 0xC7,
        0xE4, 0xF6, 0xE8, 0xF3, 0xF3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xC0, 0xF4, 0xE3, 0xE8, 0xEE, 0x9F, 0xAE, 0x9F, 0xD2, 0xEE, 0xF4, 0xED, 0xE3, 0xF2, 0x9F, 0xAE, 0x9F, 0xCD,
        0xEE, 0xE8, 0xF2, 0xE4, 0xB9, 0x9F, 0xC3, 0xF6, 0xE8, 0xE6, 0xE7, 0xF3, 0x9F, 0xCE, 0xEA, 0xE0, 0xE7, 0xE0,
        0xF1, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC6, 0xE0, 0xEC, 0xE4, 0x9F, 0xD2, 0xE7, 0xEE, 0xF6, 0x9F,
        0xC7, 0xEE, 0xF2, 0xF3, 0xB9, 0x9F, 0xCC, 0xE8, 0xEA, 0xE4, 0x9F, 0xCB, 0xE4, 0xE6, 0xE6, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC5, 0xE0,
        0xED, 0xF3, 0xE0, 0xF2, 0xF8, 0x9F, 0xCC, 0xE8, 0xF2, 0xF3, 0xF1, 0xE4, 0xF2, 0xF2, 0x9F, 0xAE, 0x9F, 0xC8,
        0xED, 0xF2, 0xF3, 0xE0, 0xEB, 0xEB, 0x9F, 0xD0, 0xF4, 0xE4, 0xE4, 0xED, 0xB9, 0x9F, 0xCC, 0xE0, 0xF1, 0xE8,
        0xE0, 0x9F, 0xCB, 0xE4, 0xE6, 0xE6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC2, 0xEE, 0xEC, 0xEC, 0xE0, 0xED, 0xE3, 0xEE, 0x9F, 0xAE, 0x9F, 0xCC,
        0xE0, 0xE4, 0xF2, 0xF3, 0xF1, 0xEE, 0xB9, 0x9F, 0xC5, 0xF1, 0xE0, 0xED, 0xEA, 0x9F, 0xCA, 0xEB, 0xE4, 0xEF,
        0xE0, 0xE2, 0xEA, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC4, 0xF5, 0xE0, 0xB9,
        0x9F, 0xCA, 0xE8, 0xE0, 0x9F, 0xA7, 0xF6, 0xE7, 0xEE, 0x9F, 0xE4, 0xEB, 0xF2, 0xE4, 0xA8, 0x9F, 0xC7, 0xF4,
        0xED, 0xF3, 0xF9, 0xE8, 0xED, 0xE6, 0xE4, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xD0, 0xAE, 0xC0, 0xB9, 0x9F, 0xC6, 0xEB, 0xE4, 0xED, 0xED, 0x9F, 0xD2, 0xEF, 0xE4,
        0xF1, 0xF1, 0xF8, 0xAB, 0x9F, 0xCC, 0xE8, 0xEA, 0xE4, 0x9F, 0xD2, 0xEC, 0xE8, 0xF3, 0xE7, 0xAB, 0x9F, 0xD3,
        0xF1, 0xEE, 0xF8, 0x9F, 0xCB, 0xE4, 0xEE, 0xED, 0xE0, 0xF1, 0xE3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC4, 0xED, 0xE2, 0xEE, 0xF4, 0xF1,
        0xE0, 0xE6, 0xE4, 0xEC, 0xE4, 0xED, 0xF3, 0xB9, 0x9F, 0xC9, 0xEE, 0xED, 0xE0, 0xF3, 0xE7, 0xE0, 0xED, 0x9F,
        0xCB, 0xE0, 0xED, 0xE8, 0xE4, 0xF1, 0xAB, 0x9F, 0xC6, 0xF1, 0xE4, 0xE6, 0x9F, 0xC7, 0xE9, 0xE4, 0xEB, 0xF2,
        0xF3, 0xF1, 0xEE, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00,
    };
    int count;
    int i;
    count = 16;
    if (entry < count) {
        strcpy(str, (char*)RealSoleCredits[entry]);
        for (i = 0; i < strlen(str); i++) {
            str[i] -= 0x7F;
        }
    } else {
        *str = 0;
    }
}
