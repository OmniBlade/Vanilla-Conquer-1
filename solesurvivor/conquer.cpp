//
// Copyright 2020 Electronic Arts Inc.
//
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

/* $Header:   F:\projects\c&c\vcs\code\conquer.cpv   2.18   16 Oct 1995 16:50:24   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CONQUER.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 3, 1991                                                *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CC_Draw_Shape -- Custom draw shape handler.                                               *
 *   Call_Back -- Main game maintenance callback routine.                                      *
 *   Color_Cycle -- Handle the general palette color cycling.                                  *
 *   Disk_Space_Available -- returns bytes of free disk space              						  *
 *   Fading_Table_Name -- Builds a theater specific fading table name.                         *
 *   Fetch_Techno_Type -- Convert type and ID into TechnoTypeClass pointer.                    *
 *   Force_CD_Available -- Ensures that specified CD is available.                             *
 *   Get_Radar_Icon -- Builds and alloc a radar icon from a shape file                         *
 *   Handle_Team -- Processes team selection command.                                          *
 *   Handle_View -- Either records or restores the tactical view.                              *
 *   KN_To_Facing -- Converts a keyboard input number into a facing value.                     *
 *   Keyboard_Process -- Processes the tactical map input codes.                               *
 *   Language_Name -- Build filename for current language.                                     *
 *   Main_Game -- Main game startup routine.                                                   *
 *   Main_Loop -- This is the main game loop (as a single loop).                               *
 *   Map_Edit_Loop -- a mini-main loop for map edit mode only                                  *
 *   Message_Input -- allows inter-player message input processing                             *
 *   MixFileHandler -- Handles VQ file access.                                                 *
 *   Name_From_Source -- retrieves the name for the given SourceType                           *
 *   Play_Movie -- Plays a VQ movie.                                                           *
 *   Source_From_Name -- Converts ASCII name into SourceType.                                  *
 *   Sync_Delay -- Forces the game into a 15 FPS rate.                                         *
 *   Theater_From_Name -- Converts ASCII name into a theater number.                           *
 *   Trap_Object -- gets a ptr to object of given type & coord                                 *
 *   Unselect_All -- Causes all selected objects to become unselected.                         *
 *   VQ_Call_Back -- Maintenance callback used for VQ movies.                                  *
 *   Validate_Error -- prints an error message when an object fails validation                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"
#include "common/irandom.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common/framelimit.h"
#include "common/paths.h"
#include "common/vqatask.h"
#include "common/vqaloader.h"
#include "common/settings.h"
#include "common/winasm.h"
#include <time.h>

#define SHAPE_TRANS 0x40

void* Get_Shape_Header_Data(void* ptr);

/****************************************
**	Function prototypes for this module **
*****************************************/
bool Main_Loop();
void Keyboard_Process(KeyNumType& input);
#ifndef DEMO
static void Message_Input(KeyNumType& input);
#endif
bool Color_Cycle(void);
bool Map_Edit_Loop(void);
void Trap_Object(void);

#ifdef CHEAT_KEYS
void Heap_Dump_Check(const char* string);
void Dump_Heap_Pointers(void);
void Error_In_Heap_Pointers(char* string);
#endif
extern void Register_Game_Start_Time(void);
extern void Register_Game_End_Time(void);
extern void Send_Statistics_Packet(void);
extern char* __nheapbeg;
bool InMainLoop = false;

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

/***********************************************************************************************
 * Main_Game -- Main game startup routine.                                                     *
 *                                                                                             *
 *    This is the first official routine of the game. It handles game initialization and       *
 *    the main game loop control.                                                              *
 *                                                                                             *
 *    Initialization:                                                                          *
 *    - Init_Game handles one-time-only inits                                                  *
 *    - Select_Game is responsible for initializations required for each new game played       *
 *      (these may be different depending on whether a multiplayer game is selected, and       *
 *      other parameters)                                                                      *
 *    - This routine performs any un-inits required, both for each game played, and one-time   *
 *                                                                                             *
 * INPUT:   argc  -- Number of command line arguments (including program name itself).         *
 *                                                                                             *
 *          argv  -- Array of command line argument pointers.                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
extern int TotalLocks;
void Main_Game(int argc, char* argv[])
{
    bool fade = false; // don't fade title screen the first time through

    /*
    **	Perform one-time-only initializations
    */
    if (!Init_Game(argc, argv)) {
        return;
    }

    CCDebugString("SOLE - Game initialisation complete.\n");
    /*
    **	Game processing loop:
    **	1) Select which game to play, or whether to exit (don't fade the palette
    **		on the first game selection, but fade it in on subsequent calls)
    **	2) Invoke either the main-loop routine, or the editor-loop routine,
    **		until they indicate that the user wants to exit the scenario.
    */
    while (Select_Game(fade)) {
        Map.Color_List_Add_Teams();
        ColorListTimer_5586C0 = 0;
        CCDebugString("Select_Game() non-zero.\n");
        ScenarioInit = 0; // Kludge.
        // Theme.Queue_Song(THEME_PICK_ANOTHER);

        fade = true;

        /*
        **	Make the game screen visible, clear the keyboard buffer of spurious
        **	values, and then show the mouse.  This PRESUMES that Select_Game() has
        **	told the map to draw itself.
        */
        Fade_Palette_To(GamePalette, FADE_PALETTE_MEDIUM, NULL);
        Keyboard->Clear();

        /*
        ** Only show the mouse if we're not playing back a recording.
        */
        Show_Mouse();

        SpecialDialog = SDLG_NONE;

        InMainLoop = true;
        IdleTime = time(0);
        Set_Video_Cursor_Clip(true);

#ifdef SCENARIO_EDITOR
        /*
        **	Scenario-editor version of main-loop processing
        */
        for (;;) {

            /*
            **	Non-scenario-editor-mode: call the game's main loop
            */
            if (!Debug_Map) {
                TotalLocks = 0;
                if (Main_Loop()) {
                    break;
                }

                if (SpecialDialog != SDLG_NONE) {
                    Set_Video_Cursor_Clip(false);

                    switch (SpecialDialog) {
                    case SDLG_SPECIAL:
                        Map.Help_Text(TXT_NONE);
                        Map.Override_Mouse_Shape(MOUSE_NORMAL, false);
                        Special_Dialog();
                        Map.Revert_Mouse_Shape();
                        SpecialDialog = SDLG_NONE;
                        break;

                    case SDLG_OPTIONS:
                        Map.Help_Text(TXT_NONE);
                        Map.Override_Mouse_Shape(MOUSE_NORMAL, false);
                        Options.Process();
                        Map.Revert_Mouse_Shape();
                        SpecialDialog = SDLG_NONE;
                        break;

                    case SDLG_SURRENDER:
                        Map.Help_Text(TXT_NONE);
                        Map.Override_Mouse_Shape(MOUSE_NORMAL, false);
                        if (Surrender_Dialog()) {
                            OutList.Add(EventClass(EventClass::DESTRUCT));
                        }
                        SpecialDialog = SDLG_NONE;
                        Map.Revert_Mouse_Shape();
                        break;

                    default:
                        break;
                    }

                    Set_Video_Cursor_Clip(true);
                }
            } else {

                /*
                **	Scenario-editor-mode: call the editor's main loop
                */
                if (Map_Edit_Loop()) {
                    break;
                }
            }
        }
#else
        /*
        **	Non-editor version of main-loop processing
        */
        for (;;) {

            /*
            **	Call the game's main loop
            */
            TotalLocks = 0;
            if (Main_Loop()) {
                break;
            }

            /*
            **	If the SpecialDialog flag is set, invoke the given special dialog.
            **	This must be done outside the main loop, since the dialog will call
            **	Main_Loop(), allowing the game to run in the background.
            */
            if (SpecialDialog != SDLG_NONE) {
                Set_Video_Cursor_Clip(false);

                switch (SpecialDialog) {
                case SDLG_SPECIAL:
                    Map.Help_Text(TXT_NONE);
                    Map.Override_Mouse_Shape(MOUSE_NORMAL, false);
                    Special_Dialog();
                    Map.Revert_Mouse_Shape();
                    SpecialDialog = SDLG_NONE;
                    break;

                case SDLG_OPTIONS:
                    Map.Help_Text(TXT_NONE);
                    Map.Override_Mouse_Shape(MOUSE_NORMAL, false);
                    Options.Process();
                    Map.Revert_Mouse_Shape();
                    SpecialDialog = SDLG_NONE;
                    break;

                case SDLG_SURRENDER:
                    Map.Help_Text(TXT_NONE);
                    Map.Override_Mouse_Shape(MOUSE_NORMAL, false);
                    if (Surrender_Dialog()) {
                        OutList.Add(EventClass(EventClass::DESTRUCT));
                    }
                    SpecialDialog = SDLG_NONE;
                    Map.Revert_Mouse_Shape();
                    break;

                case SDLG_SECRET_CREDITS:
                    Map.Help_Text(TXT_NONE);
                    Map.Override_Mouse_Shape(MOUSE_NORMAL, false);
                    Secret_Credits_Dialog();
                    SpecialDialog = SDLG_NONE;
                    Map.Revert_Mouse_Shape();
                    break;

                case SDLG_4:
                case SDLG_5:
                default:
                    break;
                }

                Set_Video_Cursor_Clip(true);
            }
        }
#endif
        Set_Video_Cursor_Clip(false);
        InMainLoop = false;

        SquadPostAcceptanceState = 0;
        SquadGameCountdownTimer.Reset(false);
        SquadGameCountdownTimer.Set(0);
        SquadAcceptanceState = 0;
        SquadGamePasswordCountDown = 0;

        /*
        **	Scenario is done; fade palette to black
        */
        Fade_Palette_To(BlackPalette, FADE_PALETTE_SLOW, NULL);
        VisiblePage.Clear();

        if (GameToPlay == GAME_CLIENT) {
            Client_Disconnect();
        } else if (GameToPlay == GAME_HOST) {
            Host_Disconnect();
        }

        if (GameToPlay == GAME_CLIENT) {
            if (Get_WChat_Handle()) {
                Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, 0);
                VisiblePage.Clear();
                ShowWindow(MainWindow, 6);
                Tickle_WChat();
            }
        }

        /*
		** Set all variables back to normal, to return to the main menu.
		*/
        GameToPlay = GAME_NORMAL;

        ColorListTiming = WDTGameTimer.Time() / 60;
        Map.Color_List_Clear();
    }

    Options.Save_Settings();

    if (Is_Demo()) {
        Hide_Mouse();
        Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, NULL);
        Load_Title_Screen("DEMOPIC.CPS", &HidPage, Palette);
        Blit_Hid_Page_To_Seen_Buff();
        Fade_Palette_To(Palette, FADE_PALETTE_MEDIUM, NULL);
        Keyboard->Clear();
        Keyboard->Get();
        Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, NULL);
    }

    /*
    **	Free the scenario description buffers
    */
    Free_Scenario_Descriptions();

#ifndef NOMEMCHECK
    Uninit_Game();
#endif
}

/***********************************************************************************************
 * Keyboard_Process -- Processes the tactical map input codes.                                 *
 *                                                                                             *
 *    This routine is used to process the input codes while the player                         *
 *    has the tactical map displayed. It handles all the keys that                             *
 *    are appropriate to that mode.                                                            *
 *                                                                                             *
 * INPUT:   input -- Input code as returned from Input_Num().                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/21/1992 JLB : Created.                                                                 *
 *   07/04/1995 JLB : Handles team and map control hotkeys.                                    *
 *=============================================================================================*/
extern int DebugColour;
void Keyboard_Process(KeyNumType& input)
{
    HousesType house;

    /*
    **	Don't do anything if there is not keyboard event.
    */
    if (input == KN_NONE) {
        return;
    }

    int player_count;
    int index;
    int admin_count;
    /*
    **	For network & modem, process user input for inter-player messages.
    */
    Message_Input(input);

    /*
    ** Use WWKEY values because KN values have WWKEY_KN_BIT or'd in with them
    ** and we need WWKEY_KN_BIT to still be set if it is.
    */
    KeyNumType plain = (KeyNumType)(input & ~(WWKEY_SHIFT_BIT | WWKEY_ALT_BIT | WWKEY_CTRL_BIT));
    KeyNumType key = KeyNumType(input & ~WWKEY_VK_BIT);

#ifdef CHEAT_KEYS

    if (Debug_Flag) {
        switch (input) {
        case (int)KN_M | (int)KN_SHIFT_BIT:
        case (int)KN_M | (int)KN_ALT_BIT:
        case (int)KN_M | (int)KN_CTRL_BIT:
            PlayerPtr->Credits += 10000;
            break;

        default:
            break;
        }
    }
#endif

#ifdef VIRGIN_CHEAT_KEYS
    if (Debug_Playtest && input == (KN_W | KN_ALT_BIT)) {
        PlayerPtr->Blockage = false;
        PlayerPtr->Flag_To_Win();
    }
#endif

#ifdef CHEAT_KEYS
    if (Debug_Playtest && input == (KN_W | KN_ALT_BIT)) {
        PlayerPtr->Blockage = false;
        PlayerPtr->Flag_To_Win();
    }

    if ((Debug_Flag || Debug_Playtest) && plain == KN_F4) {
        if (GameToPlay == GAME_NORMAL) {
            Debug_Unshroud = (Debug_Unshroud == false);
            Map.Flag_To_Redraw(true);
        }
    }

    if (Debug_Flag && input == KN_SLASH) {
        if (GameToPlay != GAME_NORMAL) {
            SpecialDialog = SDLG_SPECIAL;
            input = KN_NONE;
        } else {
            Special_Dialog();
        }
    }
#endif

    /*
    **	Process prerecorded team selection. This will be an addative select
    **	if the SHIFT key is held down. It will create the team if the
    **	CTRL or ALT key is held down.
    */
    int action = 0;
    if (input & WWKEY_SHIFT_BIT)
        action = 1;
    if (input & WWKEY_ALT_BIT)
        action = 3;
    if (input & WWKEY_CTRL_BIT)
        action = 2;

    /*
    **	If the "N" key is pressed, then select the next object.
    */
    if (key != 0 && key == Options.KeyNext) {
        if (action) {
            obj = Map.Prev_Object(CurrentObject.Count() ? CurrentObject[0] : NULL);
        } else {
            obj = Map.Next_Object(CurrentObject.Count() ? CurrentObject[0] : NULL);
        }
        if (obj != NULL) {
            Unselect_All();
            obj->Select();
            Map.Center_Map();
            Map.Flag_To_Redraw(true);
        }
        input = KN_NONE;
    }
    if (key != 0 && key == Options.KeyPrevious) {
        if (action) {
            obj = Map.Next_Object(CurrentObject.Count() ? CurrentObject[0] : NULL);
        } else {
            obj = Map.Prev_Object(CurrentObject.Count() ? CurrentObject[0] : NULL);
        }
        if (obj != NULL) {
            Unselect_All();
            obj->Select();
            Map.Center_Map();
            Map.Flag_To_Redraw(true);
        }
        input = KN_NONE;
    }

    /*
    **	All selected units will go into idle mode.
    */
    if (key != 0 && key == Options.KeyStop) {
        if (CurrentObject.Count()) {
            for (index = 0; index < CurrentObject.Count(); index++) {
                ObjectClass const* tech = CurrentObject[index];

                if (tech != NULL
                    && (tech->Can_Player_Move() || (tech->Can_Player_Fire() && tech->What_Am_I() != RTTI_BUILDING))) {
                    OutList.Add(EventClass(EventClass::IDLE, tech->As_Target()));
                }
            }
        }
        input = KN_NONE;
    }

    /*
    ** Sole keys, TODO convert to option configurable.
    */
    switch (key) {
    case KN_0:
    case KN_1:
    case KN_2:
    case KN_3:
    case KN_4:
    case KN_5:
    case KN_6:
    case KN_7:
    case KN_8:
    case KN_9:
        Messages.Add_Team_Message(key - KN_0);
        input = KN_NONE;
        break;

    /*
    **	Drops flag player's unit is carrying.
    */
    case VK_D:
        if (CurrentObject.Count()) {
            ObjectClass* obj = CurrentObject[0];
            if (obj) {
                FootClass* foot = (FootClass*)obj;
                if (foot && foot->Flagged != HOUSE_NONE && (IsServerAdmin || PlayerPtr->Class->House == obj->Owner())) {
                    OutList.Add(EventClass(EventClass::DROP_FLAG, foot->As_Target()));
                    Sound_Effect(VOC_DOWN);
                }
            }
        }
        input = KN_NONE;
        break;

    /*
    **	Toggles viewport tracking of players unit.
    */
    case VK_T: {
        char buf[100];
        if (IsTrackingCurrentObject) {
            IsTrackingCurrentObject = false;
            sprintf(buf, Text_String(TXT_TRACKING_MODE_OFF));
        } else {
            IsTrackingCurrentObject = true;
            sprintf(buf, Text_String(TXT_TRACKING_MODE_ON));
            IsTrackingRedraw = true;
        }
        Messages.Add_Message(buf, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
        Map.Flag_To_Redraw(true);

        input = KN_NONE;
        break;
    }

    /*
    **	Bumps ally unit from flag.
    */
    case VK_B:
        if (GameToPlay == GAME_CLIENT && GameParams.IsCaptureTheFlag) {
            if (!IsTrackingCurrentObject) {
                Update_CTF_Tracking();
                Map.Flag_To_Redraw(true);
            } else {
                Sound_Effect(VOC_SCOLD, VOL_FULL);
            }
        }
        input = KN_NONE;
        break;

    /*
    **	Center view on your Flag.
    */
    case VK_F:
        if (GameToPlay == GAME_CLIENT && (GameParams.IsCaptureTheFlag || GameParams.Football)) {
            if (!IsTrackingCurrentObject) {
                Update_Tracking();
                Map.Flag_To_Redraw(true);
            } else {
                Sound_Effect(VOC_SCOLD, VOL_FULL);
            }
        }
        input = KN_NONE;
        break;

    /*
    **	All selected units will attempt to go into guard area mode.
    */
    case VK_G:
        if (CurrentObject.Count()) {
            for (int index = 0; index < CurrentObject.Count(); index++) {
                ObjectClass const* tech = CurrentObject[index];

                if (tech && tech->Can_Player_Move() && tech->Can_Player_Fire()) {
                    OutList.Add(EventClass(tech->As_Target(), MISSION_GUARD_AREA));
                }
            }
        }
        input = KN_NONE;
        break;

    /*
    **	Flags player list to redraw.
    */
    case VK_L:
        Map.Color_List_Flag_To_Redraw();
        Map.Flag_To_Redraw();
        input = KN_NONE;
        break;

    /*
    **	Toggles drawing player names in viewport.
    */
    case VK_N:
        if (ShowNames == true) {
            ShowNames = false;
        } else {
            ShowNames = true;
        }
        Map.Flag_To_Redraw(true);
        input = KN_NONE;
        break;

    /*
    **	If in server mode toggles crates.
    */
    case VK_Q:
        if (GameToPlay == GAME_HOST && !OfflineMode) {
            if (CratesDisabled) {
                Enable_Crates();
            } else {
                Disable_Crates();
                CrateKeepTimer.Set(1, 1);
            }
        }
        input = KN_NONE;
        break;

    /*
    **	Prints misc info about the session.
    */
    case VK_Y:
        if (GameToPlay == GAME_HOST && !OfflineMode) {
            player_count = 0;
            for (index = 0; index < ActivePlayers.Count(); index++) {
                house = ActivePlayers[index]->HousePtr->Class->House;
                if (house != HOUSE_SPECTATOR && house != HOUSE_ADMIN) {
                    player_count++;
                }
            }

            int spec_count = 0;
            for (index = 0; index < ActivePlayers.Count(); index++) {
                house = ActivePlayers[index]->HousePtr->Class->House;
                if (house == HOUSE_SPECTATOR) {
                    spec_count++;
                }
            }

            admin_count = 0;
            for (index = 0; index < ActivePlayers.Count(); index++) {
                house = ActivePlayers[index]->HousePtr->Class->House;
                if (house == HOUSE_ADMIN) {
                    admin_count++;
                }
            }
            char buf[80];
            sprintf(buf,
                    "Players:%d Spectators:%d SuperUsers:%d Admin:%d, Duration:%s",
                    player_count,
                    spec_count,
                    admin_count,
                    RemoteAdminsComms.Count(),
                    Get_Game_Duration_As_String());
            Messages.Add_Message(buf, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
            Map.Flag_To_Redraw(false);
        }
        input = KN_NONE;
        break;

    default:
        break;
    }

    /*
    **	All selected units will attempt to scatter.
    */
    if (key != 0 && key == Options.KeyScatter) {
        if (CurrentObject.Count()) {
            for (index = 0; index < CurrentObject.Count(); index++) {
                ObjectClass const* tech = CurrentObject[index];

                if (tech != NULL && tech->Can_Player_Move()) {
                    OutList.Add(EventClass(EventClass::SCATTER, tech->As_Target()));
                }
            }
        }
        input = KN_NONE;
    }

    /*
    **	Center the map around the currently selected objects. If no
    **	objects are selected, then fall into the home case.
    */
    if (key != 0 && (key == Options.KeyHome1 || key == Options.KeyHome2)) {
        input = Options.KeyBase;
    }

    /*
    **	Center the map about the construction yard or construction vehicle
    **	if one is present.
    */
    if (key != 0 && key == Options.KeyBase) {
        if (!IsTrackingCurrentObject) {
            ++ScenarioInit;
            Map.Compute_Start_Pos();
            --ScenarioInit;
        }
        Map.Flag_To_Redraw(true);
        input = KN_NONE;
    }

    /*
    ** Toggle the status of formation for the current team
    */
    if (key != 0 && key == Options.KeyFormation) {
        /* TODO, formations not implemented in TD yet. */
    }

    if (input != 0 && input == Options.KeyResign) {
        if (!PlayerPtr->IsDefeated) {
            SpecialDialog = SDLG_SURRENDER;
        }
        input = KN_NONE;
    }

    /*
    **	Handle making and breaking alliances.
    */
    if (key != 0 && key == Options.KeyAlliance) {
        if (GameToPlay != GAME_NORMAL || Debug_Flag) {
            if (CurrentObject.Count() && !PlayerPtr->IsDefeated) {
                if (CurrentObject[0]->Owner() != PlayerPtr->Class->House) {
                    OutList.Add(EventClass(EventClass::ALLY, CurrentObject[0]->Owner()));
                }
            }
        }
        input = KN_NONE;
    }

    /*
    **	Select all the units on the current display. This is equivalent to
    **	drag selecting the whole view.
    */
    if (key != 0 && key == Options.KeySelectView) {
        Map.Select_These(0x00000000, XY_Coord(Map.TacLeptonWidth, Map.TacLeptonHeight));
        input = KN_NONE;
    }

    /*
    **	Toggles the repair state similarly to pressing the repair button.
    */
    if (key != 0 && key == Options.KeyRepair) {
        Map.Repair_Mode_Control(-1);
        input = KN_NONE;
    }

    /*
    **	Toggles the sell state similarly to pressing the sell button.
    */
    if (key != 0 && key == Options.KeySell) {
        Map.Sell_Mode_Control(-1);
        input = KN_NONE;
    }

    /*
    **	Toggles the map zoom mode similarly to pressing the map button.
    */
    if (key != 0 && key == Options.KeyMap) {
        Map.Zoom_Mode_Control();
        input = KN_NONE;
    }

    /*
    **	Scrolls the sidebar up one slot.
    */
    if (key != 0
        && (key == Options.KeySidebarUp || (Settings.Options.MouseWheelScrolling && key == KN_MOUSEWHEEL_UP))) {
        Map.SidebarClass::Scroll(true, -1);
        input = KN_NONE;
    }

    /*
    **	Scrolls the sidebar down one slot.
    */
    if (key != 0
        && (key == Options.KeySidebarDown || (Settings.Options.MouseWheelScrolling && key == KN_MOUSEWHEEL_DOWN))) {
        Map.SidebarClass::Scroll(false, -1);
        input = KN_NONE;
    }

    /*
    **	Brings up the options dialog box.
    */
    if (key != 0 && (key == Options.KeyOption1 || key == Options.KeyOption2)) {
        Map.Help_Text(TXT_NONE); // Turns off help text.
        Queue_Options();
        input = KN_NONE;
    }

    /*
    **	Scrolls the tactical map in the direction specified.
    */
    int distance = CELL_LEPTON_W;
    if (key != 0 && key == Options.KeyScrollLeft) {
        Map.Scroll_Map(DIR_W, distance, true);
        input = KN_NONE;
    }
    if (key != 0 && key == Options.KeyScrollRight) {
        Map.Scroll_Map(DIR_E, distance, true);
        input = KN_NONE;
    }
    if (key != 0 && key == Options.KeyScrollUp) {
        Map.Scroll_Map(DIR_N, distance, true);
        input = KN_NONE;
    }
    if (key != 0 && key == Options.KeyScrollDown) {
        Map.Scroll_Map(DIR_S, distance, true);
        input = KN_NONE;
    }

#if 0
    /*
    **	Teams are handled by the 10 special team keys. The manual comparison
    **	to the KN numbers is because the Windows keyboard driver can vary
    **	the base code number for the key depending on the shift or alt key
    **	state!
    */
    if (input != 0 && (plain == Options.KeyTeam1 || plain == KN_1)) {
        Handle_Team(0, action);
        input = KN_NONE;
    }
    if (input != 0 && (plain == Options.KeyTeam2 || plain == KN_2)) {
        Handle_Team(1, action);
        input = KN_NONE;
    }
    if (input != 0 && (plain == Options.KeyTeam3 || plain == KN_3)) {
        Handle_Team(2, action);
        input = KN_NONE;
    }
    if (input != 0 && (plain == Options.KeyTeam4 || plain == KN_4)) {
        Handle_Team(3, action);
        input = KN_NONE;
    }
    if (input != 0 && (plain == Options.KeyTeam5 || plain == KN_5)) {
        Handle_Team(4, action);
        input = KN_NONE;
    }
    if (input != 0 && (plain == Options.KeyTeam6 || plain == KN_6)) {
        Handle_Team(5, action);
        input = KN_NONE;
    }
    if (input != 0 && (plain == Options.KeyTeam7 || plain == KN_7)) {
        Handle_Team(6, action);
        input = KN_NONE;
    }
    if (input != 0 && (plain == Options.KeyTeam8 || plain == KN_8)) {
        Handle_Team(7, action);
        input = KN_NONE;
    }
    if (input != 0 && (plain == Options.KeyTeam9 || plain == KN_9)) {
        Handle_Team(8, action);
        input = KN_NONE;
    }
    if (input != 0 && (plain == Options.KeyTeam10 || plain == KN_0)) {
        Handle_Team(9, action);
        input = KN_NONE;
    }
#endif

    /*
    **	Handle the bookmark hotkeys.
    */
    if (input != 0 && plain == Options.KeyBookmark1 && !Debug_Map) {
        Handle_View(0, action);
        input = KN_NONE;
    }
    if (input != 0 && plain == Options.KeyBookmark2 && !Debug_Map) {
        Handle_View(1, action);
        input = KN_NONE;
    }
    if (input != 0 && plain == Options.KeyBookmark3 && !Debug_Map) {
        Handle_View(2, action);
        input = KN_NONE;
    }
    if (input != 0 && plain == Options.KeyBookmark4 && !Debug_Map) {
        Handle_View(3, action);
        input = KN_NONE;
    }

#ifdef CHEAT_KEYS
    if (Debug_Flag && input && (input & KN_RLSE_BIT) == 0) {
        Debug_Key(input);
    }
#endif
}

#ifndef DEMO
/***********************************************************************************************
 * Message_Input -- allows inter-player message input processing                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *		input		key value *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *		none. *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *		MAX_MESSAGE_LENGTH has increased over the DOS version. COMPAT_MESSAGE_LENGTH reflects   * * the length of the
 *DOS message and also the length of the message in the packet header.     * To allow transmission of longer messages I
 *split the message into COMPAT_MESSAGE_LENGTH-4  * sized chunks and use the extra space after the zero terminator to
 *specify which segment    * of the whole message this is and also to supply a crc for the string. * This allows message
 *segments to arrive out of order and still be displayed correctly.      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/22/1995 BRR : Created.                                                                 *
 *   03/26/1995  ST : Modified to break up longer messages into multiple packets               *
 *=============================================================================================*/
static void Message_Input(KeyNumType& input)
{
    char ttxt[300];
    char txt[300];
    int rc;
    int team;

    if (Messages.Is_Editing()) {
        /*
		**	Process message-system input
		*/
        rc = Messages.Input(input);

        if (Messages.Is_To_Redraw()) {
            Map.Flag_To_Redraw(false);
        }

        if (rc != 0) {

            if (GameToPlay == GAME_CLIENT) {
                Client_Process_Message();
            } else if (GameToPlay == GAME_HOST) {
                Host_Process_Message(0);
            }
        }
//should be from sidebar.h but there's a conflict
#define BUTTON_COLORLIST 106
    } else if (input == KA_RETURN || input == (WWKEY_BTN_BIT | BUTTON_COLORLIST)) {

        Sound_Effect(VOC_RELOAD, VOL_FULL);
        sprintf(txt, "%s ", Text_String(TXT_MESSAGE));
        Messages.Add_Edit(3, 464, 7, TPF_6PT_GRAD | TPF_NOSHADOW | TPF_BRIGHT_COLOR, txt);
        Map.Flag_To_Redraw(false);
        IsTeamMessage = false;

        if (input == (WWKEY_BTN_BIT | BUTTON_COLORLIST)) {
            Messages.Make_Message_Private();
        }

        input = KN_NONE;

    } else if (((WWKey_Type)KA_RETURN | WWKEY_SHIFT_BIT) == input && GameToPlay != GAME_HOST) {

        team = PlayerPtr->ActLike - 6;

        if (team >= 0 && team <= 3) {
            Sound_Effect(VOC_RELOAD, VOL_FULL);
            sprintf(ttxt, "%s ", Text_String(TXT_TEAM_MESSAGE));
            Messages.Add_Edit(3, 464, MPlayerTColors[team + 2], TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, ttxt);
            Map.Flag_To_Redraw(false);
            input = KN_NONE;
            IsTeamMessage = true;
        }
    }
}
#endif

/***********************************************************************************************
 * Color_Cycle -- Handle the general palette color cycling.                                    *
 *                                                                                             *
 *    This is a maintenance routine that handles the color cycling. It should be called as     *
 *    often as necessary to achieve smooth color cycling effects -- at least 8 times a second. *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  true if palette changed                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *   06/10/1994 JLB : Uses new cycle color values.                                             *
 *   12/21/1994 JLB : Handles text fade color.                                                 *
 *=============================================================================================*/
bool Color_Cycle(void)
{
    static CountDownTimerClass _timer(BT_SYSTEM, 0);
    static CountDownTimerClass _ftimer(BT_SYSTEM, 0);
    static bool _up = false;
    bool changed = false;

    /*
    **	Process the fading white color. It is used for the radar box and other glowing
    **	game interface elements.
    */
    if (!_ftimer.Time()) {
        _ftimer.Set(TIMER_SECOND / 8);

/*
**	Pulse the pulsing text color.
*/
#define STEP_RATE 5
        if (_up) {
            GamePalette[767] += STEP_RATE;
            GamePalette[766] += STEP_RATE;
            GamePalette[765] += STEP_RATE;
            if (GamePalette[767] > MAX_CYCLE_COLOR) {
                GamePalette[767] = MAX_CYCLE_COLOR;
                GamePalette[766] = MAX_CYCLE_COLOR;
                GamePalette[765] = MAX_CYCLE_COLOR;
                _up = false;
            }
        } else {
            GamePalette[767] -= STEP_RATE;
            GamePalette[766] -= STEP_RATE;
            GamePalette[765] -= STEP_RATE;
            if ((unsigned)GamePalette[767] < MIN_CYCLE_COLOR) {
                GamePalette[767] = MIN_CYCLE_COLOR;
                GamePalette[766] = MIN_CYCLE_COLOR;
                GamePalette[765] = MIN_CYCLE_COLOR;
                _up = true;
            }
        }
        changed = true;
    }

    /*
    **	Process the color cycling effects -- water.
    */
    if (!_timer.Time()) {
        unsigned char colors[3];

        _timer.Set(TIMER_SECOND / 4);

        memmove(colors, &GamePalette[(CYCLE_COLOR_START + CYCLE_COLOR_COUNT - 1) * 3], sizeof(colors));
        memmove(&GamePalette[(CYCLE_COLOR_START + 1) * 3],
                &GamePalette[CYCLE_COLOR_START * 3],
                (CYCLE_COLOR_COUNT - 1) * 3);
        memmove(&GamePalette[CYCLE_COLOR_START * 3], colors, sizeof(colors));
        changed = true;
    }

    /*
    **	If any of the processing functions changed the palette, then this palette must be
    **	passed to the system.
    */
    if (changed) {
        Wait_Vert_Blank();
        Set_Palette(GamePalette);
        return (true);
    }
    return (false);
}

/***********************************************************************************************
 * Call_Back -- Main game maintenance callback routine.                                        *
 *                                                                                             *
 *    This routine handles all the "real time" processing that needs to                        *
 *    occur. This includes palette fading and sound updating. It needs                         *
 *    to be called as often as possible.                                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *=============================================================================================*/
void Call_Back(void)
{
    /*
    **	Score maintenance
    */
    if (SampleType) {
        Theme.AI();
        Speak_AI();
    }
}

/***********************************************************************************************
 * Language_Name -- Build filename for current language.                                       *
 *                                                                                             *
 *    This routine attaches a language specific suffix to the base                             *
 *    filename provided. Typical use of this is when loading language                          *
 *    specific files at game initialization time.                                              *
 *                                                                                             *
 * INPUT:   basename -- Base name to append language specific                                  *
 *                      extension to.                                                          *
 *                                                                                             *
 * OUTPUT:  Returns with pointer to completed filename.                                        *
 *                                                                                             *
 * WARNINGS:   The return pointer value is valid only until the next time                      *
 *             this routine is called.                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *=============================================================================================*/
char const* Language_Name(char const* basename)
{
    static char _fullname[_MAX_FNAME + _MAX_EXT];

    if (!basename)
        return (NULL);

    sprintf(_fullname, "%s.ENG", basename);
    return (_fullname);
}

/***********************************************************************************************
 * Source_From_Name -- Converts ASCII name into SourceType.                                    *
 *                                                                                             *
 *    This routine is used to convert an ASCII name representing a                             *
 *    SourceType into the actual SourceType value. Typically, this is                          *
 *    used when processing the scenario INI file.                                              *
 *                                                                                             *
 * INPUT:   name  -- The ASCII source name to process.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the SourceType represented by the name                                *
 *          specified.                                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
SourceType Source_From_Name(char const* name)
{
    if (name) {
        for (SourceType source = SOURCE_FIRST; source < SOURCE_COUNT; source++) {
            if (stricmp(SourceName[source], name) == 0) {
                return (source);
            }
        }
    }
    return (SOURCE_NONE);
}

/***********************************************************************************************
 * Name_From_Source -- retrieves the name for the given SourceType         						  *
 *                                                                         						  *
 * INPUT:                                                                  						  *
 *		source		SourceType to get the name for															  *
 *                                                                         						  *
 * OUTPUT:                                                                 						  *
 *		name of SourceType																							  *
 *                                                                         						  *
 * WARNINGS:                                                               						  *
 *		none. *
 *                                                                         						  *
 * HISTORY:                                                                						  *
 *   11/15/1994 BR : Created.                                              						  *
 *=============================================================================================*/
char const* Name_From_Source(SourceType source)
{
    if ((unsigned)source < SOURCE_COUNT) {
        return (SourceName[source]);
    }
    return ("None");
}

/***********************************************************************************************
 * Theater_From_Name -- Converts ASCII name into a theater number.                             *
 *                                                                                             *
 *    This routine converts an ASCII representation of a theater and converts it into a        *
 *    matching theater number. If no match was found, then THEATER_NONE is returned.           *
 *                                                                                             *
 * INPUT:   name  -- Pointer to ASCII name to convert.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the name converted into a theater number.                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
TheaterType Theater_From_Name(char const* name)
{
    TheaterType index;

    if (name) {
        for (index = THEATER_FIRST; index < THEATER_COUNT; index++) {
            if (stricmp(name, Theaters[index].Name) == 0) {
                return (index);
            }
        }
    }
    return (THEATER_NONE);
}

/***********************************************************************************************
 * KN_To_Facing -- Converts a keyboard input number into a facing value.                       *
 *                                                                                             *
 *    This routine determine which compass direction is represented by the keyboard value      *
 *    provided. It is used for map scrolling and other directional control operations from     *
 *    the keyboard.                                                                            *
 *                                                                                             *
 * INPUT:   input -- The KN number to convert.                                                 *
 *                                                                                             *
 * OUTPUT:  Returns with the facing type that the keyboard number represents. If it could      *
 *          not be translated, then FACING_NONE is returned.                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
FacingType KN_To_Facing(int input)
{
    input &= ~(KN_ALT_BIT | KN_SHIFT_BIT | KN_CTRL_BIT);
    switch (input) {
    case KN_LEFT:
        return (FACING_W);

    case KN_RIGHT:
        return (FACING_E);

    case KN_UP:
        return (FACING_N);

    case KN_DOWN:
        return (FACING_S);

    case KN_UPLEFT:
        return (FACING_NW);

    case KN_UPRIGHT:
        return (FACING_NE);

    case KN_DOWNLEFT:
        return (FACING_SW);

    case KN_DOWNRIGHT:
        return (FACING_SE);
    }
    return (FACING_NONE);
}

/***********************************************************************************************
 * Sync_Delay -- Forces the game into a 15 FPS rate.                                           *
 *                                                                                             *
 *    This routine will wait until the timer for the current frame has expired before          *
 *    returning. It is called at the end of every game loop in order to force the game loop    *
 *    to run at a fixed rate.                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Will delay for up to 1/15 of a second.                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/04/1995 JLB : Created.                                                                 *
 *   03/06/1995 JLB : Fixed.                                                                   *
 *=============================================================================================*/
static void Sync_Delay(void)
{
    /*
    ** Slow down with frame limiter first.
    */
    Frame_Limiter();

    /*
    **	Delay one tick and keep a record that one tick was "wasted" here.
    **	This accumulates into a running histogram of performance.
    */
    SpareTicks += FrameTimer.Time();
    while (FrameTimer.Time()) {
        Color_Cycle();
        Call_Back();

        // TODO Unsure if this is needed with frame limiter in place anyhow?
        // if (!OfflineMode && GameToPlay == GAME_HOST && ShowServerDialog) {
        // Sleep(4);
        // }

        if (SpecialDialog == SDLG_NONE) {
            WWMouse->Erase_Mouse(&HidPage, true);
            KeyNumType input = KN_NONE;
            int x, y;
            WWMouse->Erase_Mouse(&HidPage, false);
            Map.Input(input, x, y);
            if (input) {
                Keyboard_Process(input);
            }
            Map.Render();
        }

        Frame_Limiter(FL_NONE);
    }
    Color_Cycle();
    Call_Back();
}

void Show_Squad_Game_Prescreen(int time)
{
    char str[200];
    int t3;
    int t1;
    int t2;

    if (SpecialDialog == SDLG_NONE) {
        t1 = time / 60;
        t3 = time - (t1 * 60);
        t2 = t3 * (100.0 / 60.0);
        if (SquadGamePasswordCountDown || GameToPlay == GAME_HOST) {
            sprintf(str, "Squad Game will start in %d seconds.", t1);
            Conquer_Clip_Text_Print(
                str, 240, 190, 15, 0, TPF_CENTER | TPF_6POINT | TPF_FULLSHADOW | TPF_BRIGHT_COLOR, 600, 0);
            Show_Quote();
            Map.Redraw_Objects();
        } else {
            sprintf(str, Text_String(TXT_ENTER_THE_PASSWORD), t1, t2);
            Conquer_Clip_Text_Print(
                str, 240, 190, 5, 0, TPF_CENTER | TPF_6POINT | TPF_FULLSHADOW | TPF_BRIGHT_COLOR, 600, 0);
            Map.Redraw_Objects();
        }
    }
}

/***********************************************************************************************
 * Main_Loop -- This is the main game loop (as a single loop).                                 *
 *                                                                                             *
 *    This function will perform one game loop.                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Should the game end?                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
extern void Check_For_Focus_Loss(void);
void Reallocate_Big_Shape_Buffer(void);

bool Main_Loop()
{
    KeyNumType input; // Player input.
    int x;
    int y;
    // int framedelay;

    //	InMainLoop = true;

    if (GameToPlay == GAME_CLIENT && ClientEvent1_BattleState_Was_2) {
        GameActive = false;
        ClientEvent1_BattleState_Was_2 = 0;
    }

    /*
    ** I think I'm gonna cry if this makes it work
    */
    if (Get_Mouse_State())
        Show_Mouse();

    /*
    ** Call the focus loss handler
    */
    Check_For_Focus_Loss();

    Windows_Message_Loop();

    /*
    ** Allocate extra memory for uncompressed shapes as needed
    */
    Reallocate_Big_Shape_Buffer();

    /*
    ** Sync-bug trapping code
    */
    if (Frame >= TrapFrame) {
        Trap_Object();
    }

    //
    // Initialize our AI processing timer
    //
    // ProcessTimer.Set(0, true);

#ifdef CHEAT_KEYS
    Heap_Dump_Check("After Trap");

    /*
    **	Update the running status debug display.
    */
    Self_Regulate();
#endif

    /*
    **	If there is no theme playing, but it looks like one is required, then start one
    **	playing. This is usually the symptom of there being no transition score.
    */
    if (SampleType && Theme.What_Is_Playing() == THEME_NONE) {
        Theme.Queue_Song(THEME_PICK_ANOTHER);
    }

    /*
    **	Setup the timer so that the Main_Loop function processes at the correct rate.
    */

    FrameTimer.Set(Options.GameSpeed);

    ColorListTimer_5586C0++;
    if (ColorListTimer_5586C0 == 2) {
        Map.Timer1.Set(0, 1);
    }

    static int StatTiming1;
    static int StatTiming2;

    if (GameParams.TimeLimit > 0) {
        int stat_time2;
        int stat_time1;
        stat_time1 = GameParams.TimeLimit * 3600 - WDTGameTimer.Time();
        stat_time2 = stat_time1 / 3600;

        if (stat_time2 > 0) {
            if (stat_time2 != StatTiming1) {
                StatTiming1 = stat_time2;
                StatPanel.Enable();
                Map.Flag_To_Redraw(false);
            }
        } else {
            int stat_sec = stat_time1 / 60;

            if (stat_sec != StatTiming2) {
                StatTiming2 = stat_sec;
                StatPanel.Enable();
                Map.Flag_To_Redraw(false);
            }
        }
    }

    /*
    **	Update the display, unless we're inside a dialog.
    */
    if (SpecialDialog == SDLG_NONE && GameInFocus) {

        WWMouse->Erase_Mouse(&HidPage, true);
        Map.Input(input, x, y);
        if (input) {
            Keyboard_Process(input);
        }
        //			HidPage.Lock();
        Map.Render();
        //			HidPage.Unlock();
    }

    /*
    ** Sort the map's ground layer by y-coordinate value.  This is done
    ** outside the IsToRedraw check, for the purposes of game sync'ing
    ** between machines; this way, all machines will sort the Map's
    ** layer in the same way, and any processing done that's based on
    ** the order of this layer will sync on different machines.
    */
    Map.Layer[LAYER_GROUND].Sort();

    //	Heap_Dump_Check( "Before Logic.AI" );

    if (GameToPlay == GAME_HOST) {
        Host_Timer_Check_Loop(&GameParams);
    }

    /*
    **	AI logic operations are performed here.
    */
    Logic.AI();

    //	Heap_Dump_Check( "After Logic.AI" );

    /*
    **	Manage the inter-player message list.  If Manage() returns true, it means
    **	a message has expired & been removed, and the entire map must be updated.
    */
    if (Messages.Manage()) {
        // HiddenPage.Clear();
        Map.Flag_To_Redraw(false);
    }

    //	Heap_Dump_Check( "Before Queue_AI" );

    /*
    **	Process all commands that are ready to be processed.
    */
    Queue_AI();

    // Heap_Dump_Check( "After Queue_AI" );

    /*
    **	Keep track of elapsed time in the game.
    */
    Score.ElapsedTime += TIMER_SECOND / TICKS_PER_SECOND;

    Call_Back();

    // Heap_Dump_Check( "After Call_Back" );

    /*
    **	Perform any win/lose code as indicated by the global control flags.
    */
    if (EndCountDown) {
        EndCountDown--;
    }

    if (GameToPlay == GAME_HOST) {
        Read_Server_Stats();
    }
    /*
    **	Check for player wins or loses according to global event flag.
    */

    if (PlayerWins) {
        WWMouse->Erase_Mouse(&HidPage, true);
        PlayerLoses = false;
        PlayerWins = false;
        PlayerRestarts = false;
        Map.Help_Text(TXT_NONE);
        Set_Video_Cursor_Clip(false);
        Do_Win();
        Set_Video_Cursor_Clip(true);
    }
    if (PlayerLoses) {
        WWMouse->Erase_Mouse(&HidPage, true);
        PlayerWins = false;
        PlayerLoses = false;
        PlayerRestarts = false;
        Map.Help_Text(TXT_NONE);
        Set_Video_Cursor_Clip(false);
        Do_Lose();
        Set_Video_Cursor_Clip(true);
    }
    if (PlayerRestarts) {
        WWMouse->Erase_Mouse(&HidPage, true);
        PlayerWins = false;
        PlayerLoses = false;
        PlayerRestarts = false;
        Map.Help_Text(TXT_NONE);
        Set_Video_Cursor_Clip(false);
        Do_Restart();
        Set_Video_Cursor_Clip(true);
    }

    /*
    **	The frame logic has been completed. Increment the frame
    **	counter.
    */
    Frame++;

    /*
    ** Is there a memory trasher altering the map??
    */
    if (Debug_Check_Map) {
        if (!Map.Validate()) {
            if (CCMessageBox().Process(
                    Text_String(TXT_MAP_ERROR), Text_String(TXT_MAP_ERROR_STOP), Text_String(TXT_MAP_ERROR_CONTINUE))
                == 0) {
                GameActive = false;
            }
            Map.Validate(); // give debugger a chance to catch it
        }
    }

    if (GameParams.IsSquadChannel) {
        if (GameToPlay == GAME_CLIENT) {
            int st = SquadGameCountdownTimer.Time();

            if (st <= 0) {
                if (!st) {
                    if (SquadAcceptanceState == 1) {
                        GameActive = 0;
                    } else if (SquadAcceptanceState == 2 && !SquadPostAcceptanceState) {
                        SquadPostAcceptanceState = 1;
                    }
                }
            }
        } else if (GameToPlay == GAME_HOST && !OfflineMode && SquadAcceptanceState == 1
                   && !SquadGameCountdownTimer.Time()) {
            Host_Process_Accepted_List();
            Host_Send_Scenario_Change_Packet();
            SquadAcceptanceState = 2;
        }
    }

    Sync_Delay();
    //	InMainLoop = false;
    return (!GameActive);
}

#ifdef SCENARIO_EDITOR
/***************************************************************************
 * Map_Edit_Loop -- a mini-main loop for map edit mode only                *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   10/19/1994 BR : Created.                                              *
 *=========================================================================*/
bool Map_Edit_Loop(void)
{
    /*
    **	Redraw the map.
    */
    Map.Render();

    /*
    **	Get user input (keys, mouse clicks).
    */
    KeyNumType input;

    int x;
    int y;
    Map.Input(input, x, y);

    /*
    **	Process keypress.
    */
    if (input) {
        Keyboard_Process(input);
    }

    Call_Back(); // maintains Theme.AI() for music
    Color_Cycle();
    Frame_Limiter();

    return (!GameActive);
}

/***************************************************************************
 * Go_Editor -- Enables/disables the map editor										*
 *                                                                         *
 * INPUT:                                                                  *
 *		flag		true = go into editor mode; false = go into game mode			*
 *                                                                         *
 * OUTPUT:                                                                 *
 *		none.																						*
 *                                                                         *
 * WARNINGS:                                                               *
 *		none.																						*
 *                                                                         *
 * HISTORY:                                                                *
 *   10/19/1994 BR : Created.                                              *
 *=========================================================================*/
void Go_Editor(bool flag)
{
    /*
    **	Go into Scenario Editor mode
    */
    if (flag) {
        Debug_Map = true;
        Debug_Unshroud = true;

        /*
        ** Un-select any selected objects
        */
        Unselect_All();

        /*
        ** Turn off the sidebar if it's on
        */
        Map.Activate(0);

        /*
        ** Reset the map's Button list for the new mode
        */
        Map.Init_IO();

        /*
        ** Force a complete redraw of the screen
        */
        HiddenPage.Clear();
        Map.Flag_To_Redraw(true);
        Map.Render();

    } else {

        /*
        **	Go into normal game mode
        */
        Debug_Map = false;
        Debug_Unshroud = false;

        /*
        ** Un-select any selected objects
        */
        Unselect_All();

        /*
        ** Reset the map's Button list for the new mode
        */
        Map.Init_IO();

        /*
        ** Force a complete redraw of the screen
        */
        HiddenPage.Clear();
        Map.Flag_To_Redraw(true);
        Map.Render();
    }
}

#endif

/***********************************************************************************************
 * MixFileHandler -- Handles VQ file access.                                                   *
 *                                                                                             *
 *    This routine is called from the VQ player when it needs to access the source file. By    *
 *    using this routine it is possible to virtualize the file system.                         *
 *                                                                                             *
 * INPUT:   vqa   -- Pointer to the VQA handle for this animation.                             *
 *                                                                                             *
 *          action-- The requested action to perform.                                          *
 *                                                                                             *
 *          buffer-- Optional buffer pointer as needed by the type of action.                  *
 *                                                                                             *
 *          nbytes-- The number of bytes (if needed) for this operation.                       *
 *                                                                                             *
 * OUTPUT:  Returns a value consistent with the action requested.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int MixFileHandler(VQAHandle* vqa, int action, void* buffer, int nbytes)
{
#ifdef REMASTER_BUILD
    return 0;
    ;
#else
    CCFileClass* file;
    int error;

    file = (CCFileClass*)vqa->VQAio;

    /*
    **	Perform the action specified by the stream command.
    */
    switch (action) {

    /*
    ** VQACMD_READ means read NBytes from the stream and place it in the
    ** memory pointed to by Buffer.
    **
    ** Any error code returned will be remapped by VQA library into
    ** VQAERR_READ.
    */
    case VQACMD_READ:
        error = (file->Read(buffer, (unsigned short)nbytes) != (unsigned short)nbytes);
        break;

    /*
    **	VQACMD_WRITE is analogous to VQACMD_READ.
    **
    ** Writing is not allowed to the VQA file, VQA library will remap the
    ** error into VQAERR_WRITE.
    */
    case VQACMD_WRITE:
        error = 1;
        break;

    /*
    **	VQACMD_SEEK asks that you perform a seek relative to the current
    ** position. NBytes is a signed number, indicating seek direction
    ** (positive for forward, negative for backward). Buffer has no meaning
    ** here.
    **
    ** Any error code returned will be remapped by VQA library into
    ** VQAERR_SEEK.
    */
    case VQACMD_SEEK:
        error = (file->Seek(nbytes, SEEK_CUR) == -1);
        break;

    /*
    **	VQACMD_OPEN asks that you open your stream for access.
    */
    case VQACMD_OPEN:
        file = new CCFileClass((char*)buffer);

        if (file != NULL && file->Is_Available()) {
            error = file->Open((char*)buffer, READ);

            if (error != -1) {
                vqa->VQAio = (void*)file;
                error = 0;
            } else {
                delete file;
                file = 0;
                error = 1;
            }
        } else {
            delete file;
            error = 1;
        }
        break;

    case VQACMD_CLOSE:
        file->Close();
        delete file;
        file = 0;
        vqa->VQAio = 0;
        error = 0;
        break;

    /*
    **	VQACMD_INIT means to prepare your stream for reading. This is used for
    ** certain streams that can't be read immediately upon opening, and need
    ** further preparation. This operation is allowed to fail; the error code
    ** will be returned directly to the client.
    */
    case VQACMD_INIT:

    /*
    **	IFFCMD_CLEANUP means to terminate the transaction with the associated
    ** stream. This is used for streams that can't simply be closed. This
    ** operation is not allowed to fail; any error returned will be ignored.
    */
    case VQACMD_CLEANUP:
        error = 0;
        break;
    }

    return (error);
#endif
}

void Rebuild_Interpolated_Palette(unsigned char* interpal)
{
    for (int y = 0; y < 255; y++) {
        for (int x = y + 1; x < 256; x++) {
            *(interpal + (y * 256 + x)) = *(interpal + (x * 256 + y));
        }
    }
}

unsigned char* InterpolatedPalettes[100];
bool PalettesRead;
unsigned PaletteCounter;

int Load_Interpolated_Palettes(char const* filename, bool add)
{
    int num_palettes = 0;
    int i;
    int start_palette;

    if (!InterpolationTable) {
        /* DOSMode should not interpolate anything. Don't allocate memory.  */
        return 0;
    }

    PalettesRead = false;
    CCFileClass file(filename);

    if (!add) {
        for (i = 0; i < ARRAY_SIZE(InterpolatedPalettes); i++) {
            InterpolatedPalettes[i] = NULL;
        }
        start_palette = 0;
    } else {
        for (start_palette = 0; start_palette < ARRAY_SIZE(InterpolatedPalettes); start_palette++) {
            if (!InterpolatedPalettes[start_palette])
                break;
        }
    }

    if (file.Is_Available()) {
        file.Open(READ);
        file.Read(&num_palettes, 4);

        num_palettes = le32toh(num_palettes);

        for (i = 0; i < num_palettes; i++) {
            InterpolatedPalettes[i + start_palette] = (unsigned char*)malloc(65536);
            memset(InterpolatedPalettes[i + start_palette], 0, 65536);
            for (int y = 0; y < 256; y++) {
                file.Read(InterpolatedPalettes[i + start_palette] + y * 256, y + 1);
            }

            Rebuild_Interpolated_Palette(InterpolatedPalettes[i + start_palette]);
        }

        PalettesRead = true;
        file.Close();
    }

    PaletteCounter = 0;
    return (num_palettes);
}

void Free_Interpolated_Palettes(void)
{
    if (!InterpolationTable) {
        /* DOSMode should not interpolate anything.  */
        return;
    }

    for (int i = 0; i < ARRAY_SIZE(InterpolatedPalettes); i++) {
        if (InterpolatedPalettes[i]) {
            free(InterpolatedPalettes[i]);
            InterpolatedPalettes[i] = NULL;
        }
    }
}

/***********************************************************************************************
 * Play_Movie -- Plays a VQ movie.                                                             *
 *                                                                                             *
 *    Use this routine to play a VQ movie. It will disptach the specified movie to the         *
 *    VQ player. The routine will not return until the movie has finished playing.             *
 *                                                                                             *
 * INPUT:   file  -- The file object that contains the movie.                                  *
 *                                                                                             *
 *          anim  -- The anim control and configuration structure that controls how the        *
 *                   movie will be played.                                                     *
 *                                                                                             *
 *          clrscrn -- Set to 1 to clear the screen when the movie is over                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
extern bool InMovie;
extern bool VQPaletteChange;
extern void Suspend_Audio_Thread(void);
extern void Resume_Audio_Thread(void);

// Play
extern void Play_Movie_GlyphX(const char* movie_name, ThemeType theme);

void Play_Movie(char const* name, ThemeType theme, bool clrscrn, bool unk)
{
    if (Options.NoMovies) {
        return;
    }
    
    /*
    ** Don't play movies in editor mode
    */
    if (Debug_Map) {
        return;
    }

    /*
    ** Don't play movies in multiplayer mode
    */
    if (GameToPlay != GAME_NORMAL) {
        return;
    }

    if (InterpolationTable)
        memset(&InterpolationTable->PaletteInterpolationTable[0][0], 0, 65536);

    if (name) {
        char fullname[_MAX_FNAME + _MAX_EXT];
        char palname[_MAX_FNAME + _MAX_EXT];

        _makepath(fullname, NULL, NULL, name, ".VQA");
        _makepath(palname, NULL, NULL, name, ".VQP");
#ifdef CHEAT_KEYS
        Mono_Set_Cursor(0, 0);
        Mono_Printf("[%s]", fullname);
#endif

        /*
        **	Reset the anim control structure.
        */
        Anim_Init(unk);
        VQPaletteChange = false;

        /*
        **	Prepare to play a movie. First hide the mouse and stop any score that is playing.
        **	While the score (if any) is fading to silence, fade the palette to black as well.
        **	When the palette has finished fading, wait until the score has finished fading
        **	before launching the movie.
        */
        Hide_Mouse();
        // Theme.Stop();
        // Theme.AI();
        Theme.Queue_Song(theme);
        if (PreserveVQAScreen == 0) {
            Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);
            VisiblePage.Clear();
            memset(BlackPalette, 0x01, 768);
            Set_Palette(BlackPalette);
            memset(BlackPalette, 0x00, 768);
        }
        PreserveVQAScreen = 0;
        Keyboard->Clear();

        VQAHandle* vqa = NULL;

        if (!Debug_Quiet && Get_Digi_Handle() != -1) {
            AnimControl.OptionFlags |= VQAOPTF_AUDIO;
        } else {
            AnimControl.OptionFlags &= ~VQAOPTF_AUDIO;
        }

        if ((vqa = VQA_Alloc()) != NULL) {
            VQA_Init(vqa, MixFileHandler);
            if (VQA_Open(vqa, fullname, &AnimControl) == 0) {
                Brokeout = false;
                // Suspend_Audio_Thread();

#if (FRENCH | GERMAN | JAPANESE)
                /*
                ** Kludge to use the old palette interpolation table for CC2TEASE
                ** unless the covert CD is inserted.
                */
                if (!stricmp(palname, "CC2TEASE.VQP")) {
                    int cd_index = Get_CD_Index(CCFileClass::Get_CD_Drive(), 1 * 60);
                    /*
                    ** If cd_index == 2 then its a covert CD
                    */
                    if (cd_index != 2) {
                        strcpy(palname, "OLDCC2T.VQP");
                    }
                }
#endif //(FRENCH | GERMAN)

#if (GERMAN)
                /*
                ** Kludge to use a different palette interpolation table for RETRO.VQA
                ** if the covert CD is inserted.
                */
                if (!stricmp(palname, "RETRO.VQP")) {
                    int cd_index = Get_CD_Index(CCFileClass::Get_CD_Drive(), 1 * 60);
                    /*
                    ** If cd_index == 2 then its a covert CD
                    */
                    if (cd_index == 2) {
                        strcpy(palname, "RETROGER.VQP");
                    }
                }

#endif // GERMAN

                Load_Interpolated_Palettes(palname);
                // Set_Palette(BlackPalette);
                SysMemPage.Clear();
                InMovie = true;
                VQA_Play(vqa, VQAMODE_RUN);
                VQA_Close(vqa);
                // Resume_Audio_Thread();
                InMovie = false;
                Free_Interpolated_Palettes();
                Set_Primary_Buffer_Format();

                /*
                **	Any movie that ends prematurely must have the screen
                **	cleared to avoid any unexpected palette glitches.
                */
                if (Brokeout) {
                    clrscrn = true;
                    VisiblePage.Clear();
                    Brokeout = false;
                }
            }

            VQA_Free(vqa);
        }

        /*
        **	Presume that the screen is left in a garbage state as well as the palette
        **	being in an unknown condition. Recover from this by clearing the screen and
        **	forcing the palette to black.
        */
        if (clrscrn) {
            VisiblePage.Clear();
            memset(BlackPalette, 0x01, 768);
            Set_Palette(BlackPalette);
            memset(BlackPalette, 0x00, 768);
            Set_Palette(BlackPalette);
        }
        Show_Mouse();
    }
}

/***********************************************************************************************
 * Unselect_All -- Causes all selected objects to become unselected.                           *
 *                                                                                             *
 *    This routine will unselect all objects that are currently selected.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void Unselect_All(void)
{
    // while (CurrentObject.Count()) {
    //	CurrentObject[0]->Unselect();
    //}

    // Added some error handling incase there was an issue removing the object - JAS 6/28/2019
    while (CurrentObject.Count()) {

        int count_before = CurrentObject.Count();
        CurrentObject[0]->Unselect();

        if (count_before <= CurrentObject.Count()) {
            GlyphX_Debug_Print("Unselect_All failed to remove an object");
            CurrentObject.Delete(CurrentObject[0]);
        }
    }
    // End of change - JAS 6/28/2019
}

void Unselect_All_Except(ObjectClass* object)
{
    int index = 0;
    while (index < CurrentObject.Count()) {

        if (CurrentObject[index] == object) {
            index++;
            continue;
        }

        int count_before = CurrentObject.Count();
        CurrentObject[index]->Unselect();

        if (count_before <= CurrentObject.Count()) {
            GlyphX_Debug_Print("Unselect_All failed to remove an object");
            CurrentObject.Delete(CurrentObject[index]);
        }
    }
}

/***********************************************************************************************
 * Fading_Table_Name -- Builds a theater specific fading table name.                           *
 *                                                                                             *
 *    This routine builds a standard fading table name. This name is dependant on the theater  *
 *    being played, since each theater has its own palette.                                    *
 *                                                                                             *
 * INPUT:   base  -- The base name of this fading table. The base name can be no longer than   *
 *                   seven characters.                                                         *
 *                                                                                             *
 *          theater  -- The theater that this fading table is specific to.                     *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the constructed fading table filename. This pointer is   *
 *          valid until this function is called again.                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
char const* Fading_Table_Name(char const* base, TheaterType theater)
{
    static char _buffer[_MAX_FNAME + _MAX_EXT];
    char root[_MAX_FNAME];

    sprintf(root, "%1.1s%s", Theaters[theater].Root, base);
    _makepath(_buffer, NULL, NULL, root, ".MRF");
    return (_buffer);
}

/***********************************************************************************************
 * Get_Radar_Icon -- Builds and alloc a radar icon from a shape file                           *
 *                                                                                             *
 * INPUT:      void const * shapefile - pointer to a key framed shapefile                      *
 *             int shapenum          - shape to extract from shapefile                         *
 *                                                                                             *
 * OUTPUT:     void const *           - 3/3 icon set of shape from file                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/12/1995 PWG : Created.                                                                 *
 *   05/10/1995 JLB : Handles a null shapefile pointer.                                        *
 *=============================================================================================*/
void const* Get_Radar_Icon(void const* shapefile, int shapenum, int frames, int zoomfactor)
{
    static int _offx[] = {0, 0, -1, 1, 0, -1, 1, -1, 1};
    static int _offy[] = {0, 0, -1, 1, 0, -1, 1, -1, 1};
    int lp, framelp;
    char pixel;

    char* retval = NULL;
    char* buffer = NULL;
    void* ptr;

    /*
    **	If there is no shape file, then there can be no radar icon imagery.
    */
    if (!shapefile)
        return (NULL);

    /*
    ** Get the pixel width and height of the frame we built.  This will
    ** be used to extract icons and build pixels.
    */
    int pixel_width = Get_Build_Frame_Width(shapefile);
    int pixel_height = Get_Build_Frame_Height(shapefile);

    /*
    ** Find the width and height in icons, adjust these by half an
    ** icon because the artists may be sloppy and miss the edge of an
    ** icon one way or the other.
    */
    int icon_width = (pixel_width + 12) / 24;
    int icon_height = (pixel_height + 12) / 24;

    /*
    ** If we have been told to build as many frames as possible, then
    ** find out how many frames there are to build.
    */
    if (frames == -1)
        frames = Get_Build_Frame_Count(shapefile);

    /*
    ** Allocate a position to store our icons.  If the alloc fails then
    ** we dont add these icons to the set.
    **/
    buffer = new char[(icon_width * icon_height * 9 * frames) + 2];
    if (!buffer)
        return (NULL);

    /*
    ** Save off the return value so that we can return it to the calling
    ** function.
    */
    retval = (char*)buffer;
    *buffer++ = (char)icon_width;
    *buffer++ = (char)icon_height;
    int val = 24 / zoomfactor;

    for (framelp = 0; framelp < frames; framelp++) {
        /*
        ** Build the current frame.  If the frame can not be built then we
        ** just need to skip past this set of icons and try to build the
        ** next frame.
        */
        if ((ptr = (void*)(Build_Frame(shapefile, shapenum + framelp, SysMemPage.Get_Buffer()))) != NULL) {
            ptr = Get_Shape_Header_Data(ptr);
            /*
            ** Loop through the icon width and the icon height building icons
            ** into the buffer pointer.  When the getx or gety falls outside of
            ** the width and height of the shape, just insert transparent pixels.
            */
            for (int icony = 0; icony < icon_height; icony++) {
                for (int iconx = 0; iconx < icon_width; iconx++) {
                    for (int y = 0; y < zoomfactor; y++) {
                        for (int x = 0; x < zoomfactor; x++) {
                            int getx = (iconx * 24) + (x * val) + (zoomfactor / 2);
                            int gety = (icony * 24) + (y * val) + (zoomfactor / 2);
                            if ((getx < pixel_width) && (gety < pixel_height)) {
                                for (lp = 0; lp < 9; lp++) {
                                    pixel = *((char*)(Add_Long_To_Pointer(
                                        ptr, ((gety - _offy[lp]) * pixel_width) + getx - _offx[lp])));
                                    if (pixel == LTGREEN)
                                        pixel = 0;
                                    if (pixel) {
                                        break;
                                    }
                                }
                                *buffer++ = pixel;
                            } else {
                                *buffer++ = 0;
                            }
                        }
                    }
                }
            }
        } else {
            buffer += icon_width * icon_height * 9;
        }
    }
    return (retval);
}

void CC_Texture_Fill(void const* shapefile, int shapenum, int xpos, int ypos, int width, int height)
{
    unsigned char* shape_pointer;
    // unsigned char	*shape_save;
    uintptr_t shape_size;
    // int x,y;

    if (shapefile && shapenum != -1) {

        /*
        ** Build frame returns a pointer now instead of the shapes length
        */
        shape_size = Build_Frame(shapefile, shapenum, _ShapeBuffer);
        if (Get_Last_Frame_Length() > _ShapeBufferSize) {
            Mono_Printf("Attempt to use shape buffer for size %d buffer is only size %d", shape_size, _ShapeBufferSize);
            Keyboard->Get();
        }

        if (shape_size) {
            shape_pointer = (unsigned char*)Get_Shape_Header_Data((void*)shape_size);
            int source_width = Get_Build_Frame_Width(shapefile);
            int source_height = Get_Build_Frame_Height(shapefile);

            LogicPage->Texture_Fill_Rect(xpos, ypos, width, height, shape_pointer, source_width, source_height);
#if (0)
            if (LogicPage->Lock()) {

                for (y = ypos; y < ypos + MIN(source_height, height); y++) {

                    shape_save = shape_pointer;

                    for (x = xpos; x < xpos + MIN(source_width, width); x++) {
                        LogicPage->Put_Pixel(x, y, *shape_pointer++);
                    }

                    shape_pointer = shape_save + source_width;
                }

                LogicPage->Unlock();
            }
#endif
        }
    }
}

extern void DLL_Draw_Intercept(int shape_number,
                               int x,
                               int y,
                               int width,
                               int height,
                               int flags,
                               ObjectClass* object,
                               const char* shape_file_name,
                               char override_owner,
                               int scale);
extern void DLL_Draw_Pip_Intercept(const ObjectClass* object, int pip);
extern void DLL_Draw_Line_Intercept(int x, int y, int x1, int y1, unsigned char color, int frame);

void CC_Draw_Shape(ObjectClass* object,
                   void const* shapefile,
                   int shapenum,
                   int x,
                   int y,
                   WindowNumberType window,
                   ShapeFlags_Type flags,
                   void const* fadingdata,
                   void const* ghostdata,
                   int scale,
                   int width,
                   int height)
{
#ifdef REMASTER_BUILD
    if (window == WINDOW_VIRTUAL) {
        if (width == 0)
            width = Get_Build_Frame_Width(shapefile);
        if (height == 0)
            height = Get_Build_Frame_Height(shapefile);
        DLL_Draw_Intercept(shapenum, x, y, width, height, (int)flags, object, NULL, -1, scale);
        return;
    }
#endif
    CC_Draw_Shape(shapefile, shapenum, x, y, window, flags, fadingdata, ghostdata);
}

void CC_Draw_Shape(ObjectClass* object,
                   const char* shape_file_name,
                   void const* shapefile,
                   int shapenum,
                   int x,
                   int y,
                   WindowNumberType window,
                   ShapeFlags_Type flags,
                   void const* fadingdata,
                   void const* ghostdata,
                   char override_owner)
{
#ifdef REMASTER_BUILD
    if (window == WINDOW_VIRTUAL) {
        int width = Get_Build_Frame_Width(shapefile);
        int height = Get_Build_Frame_Height(shapefile);
        DLL_Draw_Intercept(shapenum, x, y, width, height, (int)flags, object, shape_file_name, override_owner, 0x100);
        return;
    }
#endif
    CC_Draw_Shape(shapefile, shapenum, x, y, window, flags, fadingdata, ghostdata);
}

void CC_Draw_Pip(ObjectClass* object,
                 void const* shapefile,
                 int shapenum,
                 int x,
                 int y,
                 WindowNumberType window,
                 ShapeFlags_Type flags,
                 void const* fadingdata,
                 void const* ghostdata)
{
#ifdef REMASTER_BUILD
    if (window == WINDOW_VIRTUAL) {
        DLL_Draw_Pip_Intercept(object, shapenum);
        return;
    }
#endif
    CC_Draw_Shape(shapefile, shapenum, x, y, window, flags, fadingdata, ghostdata);
}

void CC_Draw_Line(int x, int y, int x1, int y1, unsigned char color, int frame, WindowNumberType window)
{
#ifdef REMASTER_BUILD
    if (window == WINDOW_VIRTUAL) {
        DLL_Draw_Line_Intercept(x, y, x1, y1, color, frame);
        return;
    }
#endif
    LogicPage->Draw_Line(x, y, x1, y1, color);
}
/***********************************************************************************************
 * CC_Draw_Shape -- Custom draw shape handler.                                                 *
 *                                                                                             *
 *    All draw shape calls will route through this function. It handles all draws for          *
 *    C&C. Such draws always occur to the logical page and assume certain things about         *
 *    the parameters passed.                                                                   *
 *                                                                                             *
 * INPUT:   shapefile   -- Pointer to the shape data file. This data file contains all the     *
 *                         embedded shapes.                                                    *
 *                                                                                             *
 *          shapenum    -- The shape number within the shapefile that will be drawn.           *
 *                                                                                             *
 *          x,y         -- The pixel coordinates to draw the shape.                            *
 *                                                                                             *
 *          window      -- The clipping window to use.                                         *
 *                                                                                             *
 *          flags       -- The custom draw shape flags. This controls how the parameters       *
 *                         are used (if any).                                                  *
 *                                                                                             *
 *          fadingdata  -- If SHAPE_FADING is desired, then this points to the fading          *
 *                         data table.                                                         *
 *                                                                                             *
 *          ghostdata   -- If SHAPE_GHOST is desired, then this points to the ghost remap      *
 *                         table.                                                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
//#pragma off(unreferenced)
void CC_Draw_Shape(void const* shapefile,
                   int shapenum,
                   int x,
                   int y,
                   WindowNumberType window,
                   ShapeFlags_Type flags,
                   void const* fadingdata,
                   void const* ghostdata)
{
#if true
    int predoffset;
    char* shape_pointer;
    uintptr_t shape_size;

    if (shapefile && shapenum != -1) {

        /*
        ** Build frame returns a pointer now instead of the shapes length
        */
        shape_size = Build_Frame(shapefile, shapenum, _ShapeBuffer);
        if (Get_Last_Frame_Length() > _ShapeBufferSize) {
            Mono_Printf("Attempt to use shape buffer for size %d buffer is only size %d", shape_size, _ShapeBufferSize);
            Keyboard->Get();
        }

        if (shape_size) {
            GraphicViewPortClass draw_window(LogicPage->Get_Graphic_Buffer(),
                                             WindowList[window][WINDOWX] + LogicPage->Get_XPos(),
                                             WindowList[window][WINDOWY] + LogicPage->Get_YPos(),
                                             WindowList[window][WINDOWWIDTH],
                                             WindowList[window][WINDOWHEIGHT]);

            shape_pointer = (char*)shape_size;

            /*
            **	Special shadow drawing code (used for aircraft and bullets).
            */
            if ((flags & (SHAPE_FADING | SHAPE_PREDATOR)) == (SHAPE_FADING | SHAPE_PREDATOR)) {
                flags = flags & ~(SHAPE_FADING | SHAPE_PREDATOR);
                flags = flags | SHAPE_GHOST;
                ghostdata = Map.SpecialGhost;
            }

            predoffset = Frame;

            if (x > WindowList[window][WINDOWWIDTH] >> 1) {
                predoffset = -predoffset;
            }

            if (draw_window.Lock()) {
                if ((flags & (SHAPE_GHOST | SHAPE_FADING)) == (SHAPE_GHOST | SHAPE_FADING)) {
                    Buffer_Frame_To_Page(x,
                                         y,
                                         Get_Build_Frame_Width(shapefile),
                                         Get_Build_Frame_Height(shapefile),
                                         shape_pointer,
                                         draw_window,
                                         flags | SHAPE_TRANS,
                                         ghostdata,
                                         fadingdata,
                                         1,
                                         predoffset);
                } else {
                    if (flags & SHAPE_FADING) {
                        Buffer_Frame_To_Page(x,
                                             y,
                                             Get_Build_Frame_Width(shapefile),
                                             Get_Build_Frame_Height(shapefile),
                                             shape_pointer,
                                             draw_window,
                                             flags | SHAPE_TRANS,
                                             fadingdata,
                                             1,
                                             predoffset);
                    } else {
                        if (flags & SHAPE_PREDATOR) {
                            Buffer_Frame_To_Page(x,
                                                 y,
                                                 Get_Build_Frame_Width(shapefile),
                                                 Get_Build_Frame_Height(shapefile),
                                                 shape_pointer,
                                                 draw_window,
                                                 flags | SHAPE_TRANS,
                                                 predoffset);
                        } else {
                            Buffer_Frame_To_Page(x,
                                                 y,
                                                 Get_Build_Frame_Width(shapefile),
                                                 Get_Build_Frame_Height(shapefile),
                                                 shape_pointer,
                                                 draw_window,
                                                 flags | SHAPE_TRANS,
                                                 ghostdata,
                                                 predoffset);
                        }
                    }
                }
            }
            draw_window.Unlock();
            //		} else {
            //			Mono_Printf( "Overrun ShapeBuffer!!!!!!!!!\n" );
        }
    }
#endif
}

/***********************************************************************************************
 * Fetch_Techno_Type -- Convert type and ID into TechnoTypeClass pointer.                      *
 *                                                                                             *
 *    This routine will convert the supplied RTTI type number and the ID value into a valid    *
 *    TechnoTypeClass pointer. If there is an error in conversion, then NULL is returned.      *
 *                                                                                             *
 * INPUT:   type  -- RTTI type of the techno class object.                                     *
 *                                                                                             *
 *          id    -- Integer representation of the techno sub type number.                     *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the techno type class object specified or NULL if the    *
 *          conversion could not occur.                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TechnoTypeClass const* Fetch_Techno_Type(RTTIType type, int id)
{
    switch (type) {
    case RTTI_UNITTYPE:
    case RTTI_UNIT:
        return (&UnitTypeClass::As_Reference((UnitType)id));

    case RTTI_BUILDINGTYPE:
    case RTTI_BUILDING:
        return (&BuildingTypeClass::As_Reference((StructType)id));

    case RTTI_INFANTRYTYPE:
    case RTTI_INFANTRY:
        return (&InfantryTypeClass::As_Reference((InfantryType)id));

    case RTTI_AIRCRAFTTYPE:
    case RTTI_AIRCRAFT:
        return (&AircraftTypeClass::As_Reference((AircraftType)id));
    }
    return (NULL);
}

/***************************************************************************
 * Trap_Object -- gets a ptr to object of given type & coord               *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   06/02/1995 BRR : Created.                                             *
 *=========================================================================*/
void Trap_Object(void)
{
    int i;

    TrapObject.Ptr.All = NULL;

    switch (TrapObjType) {
    case RTTI_AIRCRAFT:
        for (i = 0; i < Aircraft.Count(); i++) {
            if (Aircraft.Ptr(i)->Coord == TrapCoord || Aircraft.Ptr(i) == TrapThis) {
                TrapObject.Ptr.Aircraft = Aircraft.Ptr(i);
                break;
            }
        }
        break;

    case RTTI_ANIM:
        for (i = 0; i < Anims.Count(); i++) {
            if (Anims.Ptr(i)->Coord == TrapCoord || Anims.Ptr(i) == TrapThis) {
                TrapObject.Ptr.Anim = Anims.Ptr(i);
                break;
            }
        }
        break;

    case RTTI_BUILDING:
        for (i = 0; i < Buildings.Count(); i++) {
            if (Buildings.Ptr(i)->Coord == TrapCoord || Buildings.Ptr(i) == TrapThis) {
                TrapObject.Ptr.Building = Buildings.Ptr(i);
                break;
            }
        }
        break;

    case RTTI_BULLET:
        for (i = 0; i < Bullets.Count(); i++) {
            if (Bullets.Ptr(i)->Coord == TrapCoord || Bullets.Ptr(i) == TrapThis) {
                TrapObject.Ptr.Bullet = Bullets.Ptr(i);
                break;
            }
        }
        break;

    case RTTI_INFANTRY:
        for (i = 0; i < Infantry.Count(); i++) {
            if (Infantry.Ptr(i)->Coord == TrapCoord || Infantry.Ptr(i) == TrapThis) {
                TrapObject.Ptr.Infantry = Infantry.Ptr(i);
                break;
            }
        }
        break;

    case RTTI_UNIT:
        for (i = 0; i < Units.Count(); i++) {
            if (Units.Ptr(i)->Coord == TrapCoord || Units.Ptr(i) == TrapThis) {
                TrapObject.Ptr.Unit = Units.Ptr(i);
                break;
            }
        }
        break;

    /*
    ** Last-ditch find-the-object-right-now-darnit loop
    */
    case RTTI_NONE:
        for (i = 0; i < Aircraft.Count(); i++) {
            if (Aircraft.Raw_Ptr(i)->Coord == TrapCoord || Aircraft.Raw_Ptr(i) == TrapThis) {
                TrapObject.Ptr.Aircraft = Aircraft.Raw_Ptr(i);
                TrapObjType = RTTI_AIRCRAFT;
                return;
            }
        }
        for (i = 0; i < Anims.Count(); i++) {
            if (Anims.Raw_Ptr(i)->Coord == TrapCoord || Anims.Raw_Ptr(i) == TrapThis) {
                TrapObject.Ptr.Anim = Anims.Raw_Ptr(i);
                TrapObjType = RTTI_ANIM;
                return;
            }
        }
        for (i = 0; i < Buildings.Count(); i++) {
            if (Buildings.Raw_Ptr(i)->Coord == TrapCoord || Buildings.Raw_Ptr(i) == TrapThis) {
                TrapObject.Ptr.Building = Buildings.Raw_Ptr(i);
                TrapObjType = RTTI_BUILDING;
                return;
            }
        }
        for (i = 0; i < Bullets.Count(); i++) {
            if (Bullets.Raw_Ptr(i)->Coord == TrapCoord || Bullets.Raw_Ptr(i) == TrapThis) {
                TrapObject.Ptr.Bullet = Bullets.Raw_Ptr(i);
                TrapObjType = RTTI_BULLET;
                return;
            }
        }
        for (i = 0; i < Infantry.Count(); i++) {
            if (Infantry.Raw_Ptr(i)->Coord == TrapCoord || Infantry.Raw_Ptr(i) == TrapThis) {
                TrapObject.Ptr.Infantry = Infantry.Raw_Ptr(i);
                TrapObjType = RTTI_INFANTRY;
                return;
            }
        }
        for (i = 0; i < Units.Count(); i++) {
            if (Units.Raw_Ptr(i)->Coord == TrapCoord || Units.Raw_Ptr(i) == TrapThis) {
                TrapObject.Ptr.Unit = Units.Raw_Ptr(i);
                TrapObjType = RTTI_UNIT;
                return;
            }
        }

    default:
        break;
    }
}

/***********************************************************************************************
 * VQ_Call_Back -- Maintenance callback used for VQ movies.                                    *
 *                                                                                             *
 *    This routine is called every frame of the VQ movie as it is being played. If this        *
 *    routine returns non-zero, then the movie will stop.                                      *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the image buffer for the current frame.                     *
 *                                                                                             *
 *          frame    -- The frame number about to be displayed.                                *
 *                                                                                             *
 * OUTPUT:  Should the movie be stopped?                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void Check_VQ_Palette_Set(void);

int VQ_Call_Back(unsigned char*, int)
{
    int key = 0;
    if (Keyboard->Check()) {
        key = Keyboard->Get();
        Keyboard->Clear();
    }

    Check_VQ_Palette_Set();

    if (AnimControl.ImageHeight == 240) {
		Interpolate_2X_Scale(&SysMemPage, &UnknownViewport1, NULL, 1);
	} else {
		Interpolate_2X_Scale(&SysMemPage, &UnknownViewport1, NULL, 0);
	}

    Frame_Limiter();

    if ((BreakoutAllowed || Debug_Flag) && key == KN_ESC) {
        Keyboard->Clear();
        Brokeout = true;
        return (true);
    }

    if (!GameInFocus) {
        VQA_PauseAudio();
        while (!GameInFocus) {
            Keyboard->Check();
            Check_For_Focus_Loss();
        }
    }
    return (false);
}

/***********************************************************************************************
 * Handle_Team -- Processes team selection command.                                            *
 *                                                                                             *
 *    This routine will handle creation and selection of pseudo teams that the player can      *
 *    create or control. A team in this sense is an arbitrary grouping of units such that      *
 *    rapid selection control is allowed.                                                      *
 *                                                                                             *
 * INPUT:   team  -- The logical team number to process.                                       *
 *                                                                                             *
 *          action-- The action to perform on this team:                                       *
 *                   0 - Toggle the select state for all members of this team.                 *
 *                   1 - Select the members of this team.                                      *
 *                   2 - Make all selected objects members of this team.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/27/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void Handle_Team(int team, int action)
{
    int index;

    AllowVoice = true;
    switch (action) {

    /*
    **	Toggle the team selection. If the team is selected, then merely unselect it. If the
    **	team is not selected, then unselect all others before selecting this team.
    */
    case 3:
    case 0:

        /*
        **	If a non team member is currently selected, then deselect all objects
        **	before selecting this team.
        */
        for (index = 0; index < CurrentObject.Count(); index++) {
            ObjectClass* obj = CurrentObject[index];
            if (obj->What_Am_I() == RTTI_UNIT || obj->What_Am_I() == RTTI_INFANTRY
                || obj->What_Am_I() == RTTI_AIRCRAFT) {
                if (((FootClass*)obj)->Group != team) {
                    Unselect_All();
                    break;
                }
            }
        }
        for (index = 0; index < Units.Count(); index++) {
            UnitClass* obj = Units.Ptr(index);
            if (obj && !obj->IsInLimbo && obj->Group == team && obj->House == PlayerPtr) {
                if (!obj->Is_Selected_By_Player()) {
                    obj->Select();
                    AllowVoice = false;
                }
            }
        }
        for (index = 0; index < Infantry.Count(); index++) {
            InfantryClass* obj = Infantry.Ptr(index);
            if (obj && !obj->IsInLimbo && obj->Group == team && obj->House == PlayerPtr) {
                if (!obj->Is_Selected_By_Player()) {
                    obj->Select();
                    AllowVoice = false;
                }
            }
        }
        for (index = 0; index < Aircraft.Count(); index++) {
            AircraftClass* obj = Aircraft.Ptr(index);
            if (obj && !obj->IsInLimbo && obj->Group == team && obj->House == PlayerPtr) {
                if (!obj->Is_Selected_By_Player()) {
                    obj->Select();
                    AllowVoice = false;
                }
            }
        }

        /*
        **	Center the map around the team if the ALT key was pressed too.
        */
        if (action == 3) {
            Map.Center_Map();
            Map.Flag_To_Redraw(true);
        }
        break;

    /*
    **	Additive selection of team.
    */
    case 1:
        for (index = 0; index < Units.Count(); index++) {
            UnitClass* obj = Units.Ptr(index);
            if (obj && !obj->IsInLimbo && obj->Group == team && obj->House == PlayerPtr) {
                if (!obj->Is_Selected_By_Player()) {
                    obj->Select();
                    AllowVoice = false;
                }
            }
        }
        for (index = 0; index < Infantry.Count(); index++) {
            InfantryClass* obj = Infantry.Ptr(index);
            if (obj && !obj->IsInLimbo && obj->Group == team && obj->House == PlayerPtr) {
                if (!obj->Is_Selected_By_Player()) {
                    obj->Select();
                    AllowVoice = false;
                }
            }
        }
        for (index = 0; index < Aircraft.Count(); index++) {
            AircraftClass* obj = Aircraft.Ptr(index);
            if (obj && !obj->IsInLimbo && obj->Group == team && obj->House == PlayerPtr) {
                if (!obj->Is_Selected_By_Player()) {
                    obj->Select();
                    AllowVoice = false;
                }
            }
        }
        break;

    /*
    **	Create the team.
    */
    case 2:
        for (index = 0; index < Units.Count(); index++) {
            UnitClass* obj = Units.Ptr(index);
            if (obj && !obj->IsInLimbo && obj->House == PlayerPtr) {
                if (obj->Group == team)
                    obj->Group = -1;
                if (obj->Is_Selected_By_Player())
                    obj->Group = team;
            }
        }
        for (index = 0; index < Infantry.Count(); index++) {
            InfantryClass* obj = Infantry.Ptr(index);
            if (obj && !obj->IsInLimbo && obj->House == PlayerPtr) {
                if (obj->Group == team)
                    obj->Group = -1;
                if (obj->Is_Selected_By_Player())
                    obj->Group = team;
            }
        }
        for (index = 0; index < Aircraft.Count(); index++) {
            AircraftClass* obj = Aircraft.Ptr(index);
            if (obj && !obj->IsInLimbo && obj->House == PlayerPtr) {
                if (obj->Group == team)
                    obj->Group = -1;
                if (obj->Is_Selected_By_Player())
                    obj->Group = team;
            }
        }
        break;
    }
    AllowVoice = true;
}

/***********************************************************************************************
 * Handle_View -- Either records or restores the tactical view.                                *
 *                                                                                             *
 *    This routine is used to record or restore the current map tactical view.                 *
 *                                                                                             *
 * INPUT:   view  -- The view number to work with.                                             *
 *                                                                                             *
 *          action-- The action to perform with this view number.                              *
 *                   0  =  Restore the view to this previously remembered location.            *
 *                   1  =  Record the current view location.                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void Handle_View(int view, int action)
{
    if ((unsigned)view < sizeof(Scen.Views) / sizeof(Scen.Views[0])) {
        if (action == 0) {
            Map.Set_Tactical_Position(Coord_Whole(Cell_Coord(Scen.Views[view])));
            Map.Flag_To_Redraw(true);
        } else {
            Scen.Views[view] = Coord_Cell(Map.TacticalCoord);
        }
    }
}

#ifdef CHEAT_KEYS
void Heap_Dump_Check(const char* string)
{
#if 0
	struct _heapinfo h_info;
	int heap_status;
#endif

    if (!Debug_Trap_Check_Heap) { // check the heap?
        return;
    }

    //	Debug_Heap_Dump = true;

    //Smart_Printf("%s\n", string);

    Dump_Heap_Pointers();

#if 0
	heap_status = _heapset( 0xee );

#if (1)
	if ( heap_status == _HEAPOK ||
		heap_status == _HEAPEMPTY ) {
		Debug_Heap_Dump = false;
		return;
	}

	h_info._pentry = NULL;

	for(;;) {
		heap_status = _heapwalk( &h_info );

		if ( heap_status != _HEAPOK )
			break;
	}

	if (heap_status != _HEAPEND &&
		heap_status != _HEAPEMPTY) {
#endif
		h_info._pentry = NULL;

		for(;;) {
			heap_status = _heapwalk( &h_info );

			if ( heap_status != _HEAPOK )
				break;

			Smart_Printf( " %s block at %Fp of size %4.4X\n",
				(h_info._useflag == _USEDENTRY ? "USED" : "FREE"),
				h_info._pentry, h_info._size );
		}

		Smart_Printf( " %d block at %Fp of size %4.4X\n",
			h_info._useflag, h_info._pentry, h_info._size );

		switch ( heap_status ) {
//			case _HEAPEND:
//				Smart_Printf( "OK - end of heap\n" );
//				break;

//			case _HEAPEMPTY:
//				Smart_Printf( "OK - heap is empty\n" );
//				break;

			case _HEAPBADBEGIN:
				Smart_Printf( "ERROR - heap is damaged\n" );
				break;

			case _HEAPBADPTR:
				Smart_Printf( "ERROR - bad pointer to heap\n" );
				break;

			case _HEAPBADNODE:
				Smart_Printf( "ERROR - bad node in heap\n" );
				break;
		}
#if (1)
	}
#endif
#endif

    //	Debug_Heap_Dump = false;
}

void Dump_Heap_Pointers(void)
{
// ST - 1/2/2019 4:04PM
#if (0)
    char *ptr, *lptr, *nptr, *cptr, *dptr, *wlptr, *nlptr, *aptr, *clptr;
    int numallocs, numfrees, sizefree;
    static char _freeorused[2][5] = {"FREE", "USED"};

    ptr = (char*)__nheapbeg;

    while (ptr) {

        if (Debug_Heap_Dump) {
            Smart_Printf("%p pre header\n", (ptr - 8));
            Hex_Dump_Data((ptr - 8), 0x08);

            Smart_Printf("%p header\n", ptr);
            Hex_Dump_Data(ptr, 0x30);
        }

        dptr = (char*)*(int*)(ptr + 0x0c);

        sizefree = *(int*)(ptr + 0x14);
        numallocs = *(int*)(ptr + 0x18);
        numfrees = *(int*)(ptr + 0x1c);

        cptr = (char*)*(int*)(ptr + 0x24);
        lptr = (char*)*(int*)(ptr + 0x28);

        if (((int)cptr & 0xff000000) || ((int)dptr & 0xff000000) || ((int)lptr & 0xff000000)) {
            Error_In_Heap_Pointers("local free heap ptrs too large");
        }

        if (Debug_Heap_Dump) {
            if (lptr != dptr || lptr != cptr || cptr != dptr) {
                Smart_Printf("The pointers are different!!\n");
            }
        }

        nptr = (char*)*(int*)(ptr + 8); // next block

        if (((int)nptr & 0xFF000000)) {
            Error_In_Heap_Pointers("next block ptr too large");
        }

        if (lptr != (ptr + 0x20)) {

            if (!(*((int*)(ptr + 0x20)))) { // allocated
                aptr = (ptr + 0x2c);

                while (aptr < lptr) {

                    if ((*(int*)(aptr)) == -1) {
                        //						Smart_Printf( "end alloc chain %p.\n", aptr );
                        //						Hex_Dump_Data( aptr, 0x10);
                        break;
                    }

                    if (Debug_Heap_Dump) {
                        Smart_Printf("%p chain %s, size %X.\n",
                                     aptr,
                                     _freeorused[((*aptr) & 1)],
                                     ((*(int*)(aptr)) & 0xfffffffe));
                        Hex_Dump_Data(aptr, 0x10);
                    }

                    if (((*(int*)(aptr)) & 0xff000000)) {
                        Error_In_Heap_Pointers("alloc block size way too large");
                    }

                    aptr += ((*(int*)(aptr)) & 0xfffffffe);

                    if (((int)aptr & 0xff000000)) {
                        Error_In_Heap_Pointers("next alloc block ptr way too large");
                    }

                    numallocs--;

                    if (aptr > lptr) {
                        Error_In_Heap_Pointers("next alloc block ptr too large");
                    }
                }
            } else {
                if (sizefree != -1) {
                    sizefree -= ((*(int*)(ptr + 0x20)) & 0xfffffffe);
                }
                numfrees--;
            }

            wlptr = lptr;

            while (wlptr != (ptr + 0x20)) {

                if (Debug_Heap_Dump) {
                    Smart_Printf(
                        "%p link %s, size %X.\n", wlptr, _freeorused[((*wlptr) & 1)], ((*(int*)(wlptr)) & 0xfffffffe));
                    Hex_Dump_Data(wlptr, 0x10);
                }

                nlptr = (char*)*(int*)(wlptr + 8);

                if (!(*((int*)(wlptr)))) { // allocated
                    aptr = (wlptr + 0x0c);
                } else {
                    if (sizefree != -1) {
                        sizefree -= ((*(int*)(wlptr)) & 0xfffffffe);
                    }
                    numfrees--;

                    aptr = (wlptr + ((*(int*)(wlptr)) & 0xfffffffe));
                }

                if (nlptr == (ptr + 0x20)) {
                    clptr = nptr;
                } else {
                    clptr = nlptr;
                }

                while (aptr < clptr) {

                    if ((*(int*)(aptr)) == -1) {
                        //						Smart_Printf( "end alloc chain %p.\n", aptr );
                        //						Hex_Dump_Data( aptr, 0x10);
                        break;
                    }

                    if (Debug_Heap_Dump) {
                        Smart_Printf("%p chain %s, size %X.\n",
                                     aptr,
                                     _freeorused[((*aptr) & 1)],
                                     ((*(int*)(aptr)) & 0xfffffffe));
                        Hex_Dump_Data(aptr, 0x10);
                    }

                    if (((*(int*)(aptr)) & 0xff000000)) {
                        Error_In_Heap_Pointers("alloc block size way too large");
                    }

                    aptr += ((*(int*)(aptr)) & 0xfffffffe);

                    if (((int)aptr & 0xff000000)) {
                        Error_In_Heap_Pointers("next alloc block ptr way too large");
                    }

                    numallocs--;

                    if (aptr > clptr) {
                        Error_In_Heap_Pointers("next alloc block ptr too large");
                    }
                }

                wlptr = nlptr;
            }
        } else {
            //			Smart_Printf( "only link %s, size %X.\n",
            //				_freeorused[ ((*lptr) & 1) ],
            //				((*(int *)(lptr)) & 0xfffffffe) );

            if (!(*((int*)(lptr)))) { // allocated
                aptr = (ptr + 0x2c);

                while (aptr < nptr) {

                    if ((*(int*)(aptr)) == -1) {
                        //						Smart_Printf( "end alloc chain %p.\n", aptr );
                        //						Hex_Dump_Data( aptr, 0x10);
                        break;
                    }

                    if (Debug_Heap_Dump) {
                        Smart_Printf("%p chain %s, size %X.\n",
                                     aptr,
                                     _freeorused[((*aptr) & 1)],
                                     ((*(int*)(aptr)) & 0xfffffffe));
                        Hex_Dump_Data(aptr, 0x10);
                    }

                    if (((*(int*)(aptr)) & 0xff000000)) {
                        Error_In_Heap_Pointers("alloc block size way too large");
                    }

                    aptr += ((*(int*)(aptr)) & 0xfffffffe);

                    if (((int)aptr & 0xff000000)) {
                        Error_In_Heap_Pointers("next alloc block ptr way too large");
                    }

                    numallocs--;

                    if (aptr > nptr) {
                        Error_In_Heap_Pointers("next alloc block ptr too large");
                    }
                }
            } else {
                if (sizefree != -1) {
                    sizefree -= ((*(int*)(ptr + 0x20)) & 0xfffffffe);
                }
                numfrees--;
            }
        }

        if (sizefree != 0 && sizefree != -1) {
            Smart_Printf("sizefree left over %X.\n", sizefree);
        }

        if (numallocs != 0) {
            Smart_Printf("numallocs unaccounted for %d.\n", numallocs);
        }

        if (numfrees != 0) {
            Smart_Printf("numfrees unaccounted for %d.\n", numfrees);
        }

        ptr = nptr;
    }
#endif
}

void Error_In_Heap_Pointers(char* string)
{
    //Smart_Printf("Error in Heap for %s\n", string);
}
#endif

#ifndef ROR_NOT_READY
#define ROR_NOT_READY 21
#endif

typedef enum
{
    CD_LOCAL = -2,
    CD_ANY = -1,
    CD_SOLE = 0,
    CD_DATADIR,
    CD_COUNT
} CD_VOLUME;

static void Reinit_Secondary_Mixfiles()
{
    static bool in_progress = false;

    /*
    ** Demo loads only once and this would unload DEMOM.MIX.
    */
    if (Is_Demo()) {
        return;
    }

    if (GeneralMix != nullptr && !in_progress) {
        in_progress = true;

        delete MoviesMix;
        delete GeneralMix;
        delete ScoreMix;

        MoviesMix = new MFCD("MOVIES.MIX");
        GeneralMix = new MFCD("GENERAL.MIX");
        ScoreMix = new MFCD("SCORES.MIX");

        in_progress = false;
    }
}

/**
 * Checks for local folders containing data from the various discs.
 */
static int LastCD = -1;

static bool Change_Local_Dir(int cd)
{
    static bool _initialised = false;
    static unsigned _detected = 0;
    static const char* _vol_labels[CD_COUNT] = {"sole", "."};
    std::string paths[3] = {Paths.User_Path(), Paths.Data_Path(), Paths.Program_Path()};

    // Detect which if any of the discs have had their data copied to an appropriate local folder.
    if (!_initialised) {
        for (int i = 0; i < CD_COUNT; ++i) {
            for (int j = 0; j < 3; ++j) {
                std::string path = Paths.Concatenate_Paths(paths[j].c_str(), _vol_labels[i]);
                RawFileClass vol(path.c_str());

                if (vol.Is_Directory()) {
                    CDFileClass::Refresh_Search_Drives();
                    path += PathsClass::SEP;
                    CDFileClass::Add_Search_Drive(path.c_str());
                    CCFileClass fc("GENERAL.MIX");

                    // Populate _detected as a bitfield for which discs we found a local copy of.
                    if (fc.Is_Available()) {
                        _detected |= 1 << i;
                    }

                    break;
                }
            }
        }

        _initialised = true;
    }

    // No local folders with cd data dectected so we can't load any.
    if (_detected == 0) {
        return false;
    }

    // This condition just does a CD check to make sure we have at least one disc available.
    // If we reached this point we must have detected at least one stored locally.
    if (cd == CD_ANY) {
        // If the last CD isn't set to CD_ANY, we already did detection and set a CD path so just return true.
        if (LastCD != CD_ANY) {
            return true;
        }

        // Set current disc to most recent expansion, not counting if we found CD files in the data dir.
        for (int i = CD_COUNT - 2; i >= 0; --i) {
            if (_detected & (1 << i)) {
                cd = i;
                break;
            }
        }
    }

    // Prevent unneeded changes.
    if (LastCD == cd) {
        return true;
    }

    // If we only detected CD files locally, act like that handles all discs.
    if (_detected == (1 << CD_DATADIR)) {
        cd = CD_DATADIR;
    }

    // If the data from the CD we want was detected, then double check it and set it as though we used the -CD command line.
    if (_detected & (1 << cd)) {
        for (int j = 0; j < 3; ++j) {
            std::string path = Paths.Concatenate_Paths(paths[j].c_str(), _vol_labels[cd]);
            RawFileClass vol(path.c_str());

            if (vol.Is_Directory()) {
                CDFileClass::Refresh_Search_Drives();
                path += PathsClass::SEP;
                CDFileClass::Add_Search_Drive(path.c_str());

                // The file should be available if we reached this point.
                assert(CCFileClass("GENERAL.MIX").Is_Available());

                LastCD = cd;
                Theme.Stop();
                Reinit_Secondary_Mixfiles();
                ThemeClass::Scan();

                return true;
            }
        }
    }

    return false;
}

/***********************************************************************************************
 * Force_CD_Available -- Ensures that specified CD is available.                               *
 *                                                                                             *
 *    Call this routine when you need to ensure that the specified CD is actually in the       *
 *    CD-ROM drive.                                                                            *
 *                                                                                             *
 * INPUT:   cd    -- The CD that must be available. This will either be "0" for the GDI CD, or *
 *                   "1" for the Nod CD. If either CD will qualify, then pass in "-1".         *
 *                                                                                             *
 * OUTPUT:  Is the CD inserted and available? If false is returned, then this indicates that   *
 *          the player pressed <CANCEL>.                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/11/1995 JLB : Created.                                                                 *
 *   05/22/1996  ST : Handles multiple CD drives / CD changers                                 *
 *=============================================================================================*/
bool Force_CD_Available(int cd)
{
#ifndef DEMO
    int open_failed;
    int file;
#endif
    static char _palette[768];
    static char _hold[256];
    static void* font;
    static const char* _volid[] = {"CNC_SOLE_CD"};

    int drive;

    char volume_name[100];
    unsigned filename_length;
    unsigned misc_dword;
    int new_cd_drive = 0;
    int cd_index;
    char buffer[128];
    int cd_drive;
    int current_drive;
    int drive_search_timeout;
    bool old_in_main_loop;

    ThemeType theme_playing = THEME_NONE;

    /*
    ** If the required CD is set to -2 then it means that the file is present
    ** on the local hard drive and we shouldn't have to worry about it.
    */
    if (cd == CD_LOCAL) {
        return (true);
    }

    /*
    ** Search for the data in local folder first.
    */
    if (Change_Local_Dir(cd)) {
        return (true);
    }

    return (false);
}

/***************************************************************************
 * DISK_SPACE_AVAILABLE -- returns bytes of free disk space                *
 *                                                                         *
 * INPUT:		none                                                        *
 *                                                                         *
 * OUTPUT:     returns amount of free disk space                           *
 *                                                                         *
 * HISTORY:                                                                *
 *   08/11/1995 PWG : Created.                                             *
 *=========================================================================*/
unsigned int Disk_Space_Available(void)
{

    return 0x7fffffff;

#if (0)
    struct diskfree_t diskdata;
    unsigned drive;

    _dos_getdrive(&drive);
    _dos_getdiskfree(drive, &diskdata);

    return (diskdata.avail_clusters * diskdata.sectors_per_cluster * diskdata.bytes_per_sector);
#endif
}

/***********************************************************************************************
 * Validate_Error -- prints an error message when an object fails validation                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *		name		name of object type that failed																  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *		none. *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *		none. *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/15/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
void Validate_Error(const char* name)
{
    GlyphX_Debug_Print("Validate_Error");
    GlyphX_Debug_Print(name);
#ifdef CHEAT_KEYS
    Prog_End(NULL, true);
    printf("%s object error!\n", name);
    if (!RunningAsDLL) {
        exit(0);
    }
#else
    name = name;
#endif
}

/***************************************************************************
 * HIRES_RETRIEVE -- retrieves a resolution dependant file						*
 *                                                                         *
 * INPUT:		char * file name of the file to retrieve							*
 *                                                                         *
 * OUTPUT:     none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/25/1996     : Created.                                             *
 *=========================================================================*/
void const* Hires_Retrieve(const char* name)
{
    char filename[30];

    if (SeenBuff.Get_Width() != 320) {
        sprintf(filename, "H%s", name);
    } else {
        strcpy(filename, name);
    }
    return (MFCD::Retrieve(filename));
}

int Get_Resolution_Factor(void)
{
    return ((SeenBuff.Get_Width() == 320) ? 0 : 1);
}
/***********************************************************************************************
 * Shake_The_Screen -- Dispatcher that shakes the screen.                                      *
 *                                                                                             *
 *    This routine will shake the game screen the number of shakes requested.                  *
 *                                                                                             *
 * INPUT:   shakes   -- The number of shakes to shake the screen.                              *
 *          house    -- House to perform the shake for (or HOUSE_NONE if all players).         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/04/1996 BWG : Created.                                                                 *
 *=============================================================================================*/
void cdecl Shake_The_Screen(/*int shakes*/)
{
#if 0
	for (char h = HOUSE_FIRST; h < HOUSE_COUNT; ++h) {
		if ((house != HOUSE_NONE) && (h != house)) {
			continue;
		}
		HouseClass* hptr = HouseClass::As_Pointer((HousesType)h);
		if ((hptr != nullptr) && hptr->IsActive && hptr->IsHuman) {
			hptr->ScreenShakeTime = hptr->ScreenShakeTime + shakes + shakes;
		}
	}
#endif
}



void Get_Or_Set_Current_Directory(bool get)
{
	static char _path[260];
	static unsigned int _dnum;
	static char _buffer[264];

	unsigned num;

// TODO cross platform if needed?
#if 0
	if (get) {
		_dos_getdrive(&_dnum);
		getcwd(_path, sizeof(_path));
	} else {
		_dos_setdrive(_dnum, &num);
		chdir(_path);
		getcwd(_buffer, sizeof(_path));
	}
#endif
}

unsigned char Scale_Value_Down(int div, int mult)
{
	if (div== 0) {
		return 0;
	}

	unsigned char r = 255 * mult / div;
	return r;
}

int Scale_Value_Up(int value, unsigned char mult)
{
	if (value == 0) {
		return 0;
	}

	int r = value * mult / 255;
	return r;
}

void Clear_Team_Scores()
{
	for (int i = 0; i < 4; i++) {
		TeamScores[i] = 0;
	}
}

/**************************************************************************************************
 * Blit_Hid_Page_To_Seen_Buff -- Intercept for when the game refreshes the visible page
 *
 * In:   Command line
 *
 * Out:
 *
 *
 *
 * History: 1/3/2019 11:33AM - ST
 **************************************************************************************************/
void Blit_Hid_Page_To_Seen_Buff(void)
{
    HidPage.Blit(SeenBuff);
}

/***********************************************************************************************
 * Shake_The_Screen -- Dispatcher that shakes the screen.                                      *
 *                                                                                             *
 *    This routine will shake the game screen the number of shakes requested.                  *
 *                                                                                             *
 * INPUT:   shakes   -- The number of shakes to shake the screen.                              *
 *          house    -- House to perform the shake for (or HOUSE_NONE if all players).         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/04/1996 BWG : Created.                                                                 *
 *=============================================================================================*/
void Shake_The_Screen(int shakes, HousesType house)
{
#ifdef REMASTER_BUILD
    for (char h = HOUSE_FIRST; h < HOUSE_COUNT; ++h) {
        if ((house != HOUSE_NONE) && (h != house)) {
            continue;
        }
        HouseClass* hptr = HouseClass::As_Pointer((HousesType)h);
        if ((hptr != nullptr) && hptr->IsActive && hptr->IsHuman) {
            hptr->ScreenShakeTime = hptr->ScreenShakeTime + shakes + shakes;
        }
    }
#else
    shakes += shakes;

    Hide_Mouse();
    SeenBuff.Blit(HidPage);
    int oldyoff = 0;
    int newyoff = 0;
    while (shakes--) {
        do {
            newyoff = Sim_Random_Pick(0, 2) - 1;
        } while (newyoff == oldyoff);
        switch (newyoff) {
        case -1:
            HidPage.Blit(SeenBuff, 0, 2, 0, 0, ScreenWidth, ScreenHeight - 2);
            break;
        case 0:
            HidPage.Blit(SeenBuff);
            break;
        case 1:
            HidPage.Blit(SeenBuff, 0, 0, 0, 2, ScreenWidth, ScreenHeight - 2);
            break;
        }
        Frame_Limiter();
    }
    HidPage.Blit(SeenBuff);
    Show_Mouse();
#endif
}

/***********************************************************************************************
 * Is_Demo -- Function to determine if we are running with demo files                          *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   true if we seem to have demo files                                                *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 *=============================================================================================*/
bool Is_Demo(void)
{
    static bool bAlreadyChecked = false;
    static bool bDemo = false;

    if (!bAlreadyChecked) {
        CCFileClass file("DEMO.MIX");
        bDemo = file.Is_Available();
        bAlreadyChecked = true;
    }

    return bDemo;
}

/***********************************************************************************************
 * Is_DOS_Files -- Function to determine if we are running with DOS files                      *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   true if we seem to have DOS files                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 *=============================================================================================*/
bool Is_DOS_Files(void)
{
    static bool already_checked = false;
    static bool is_dos = false;

    if (!already_checked) {
        CCFileClass local("LOCAL.MIX");
        CCFileClass cclocal("CCLOCAL.MIX");
        is_dos = local.Is_Available() && !cclocal.Is_Available();
        already_checked = true;
    }

    return is_dos;
}
