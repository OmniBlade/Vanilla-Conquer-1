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

/* $Header:   F:\projects\c&c\vcs\code\mplayer.cpv   1.9   16 Oct 1995 16:51:08   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : MPLAYER.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Bill Randolph                                                *
 *                                                                                             *
 *                   Start Date : April 14, 1995                                               *
 *                                                                                             *
 *                  Last Update : July 5, 1995 [BRR]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Select_MPlayer_Game -- prompts user for NULL-Modem, Modem, or Network game                *
 *   Read_MultiPlayer_Settings -- reads multi-player settings from conquer.ini                 *
 *   Write_MultiPlayer_Settings -- writes multi-player settings to conquer.ini                 *
 *   Read_Scenario_Descriptions -- reads multi-player scenario #'s # descriptions              *
 *   Free_Scenario_Descriptions -- frees memory for the scenario descriptions                  *
 *   Computer_Message -- "sends" a message from the computer                                   *
 *   Garble_Message -- "garbles" a message                                                     *
 *   Surrender_Dialog -- Prompts user for surrendering                                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"
#include "common/irandom.h"
#include "common/ini.h"
#include "common/framelimit.h"
#include "common/ini.h"

/***********************************************************************************************
 * Read_MultiPlayer_Settings -- reads multi-player settings from conquer.ini                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
void Read_MultiPlayer_Settings(void)
{
#ifndef REMASTER_BUILD
    char buf[128]; // buffer for parsing INI entry
    CELL cell;

    //	Create filename and read the file.
    INIClass ini;
    CDFileClass file(CONFIG_FILE_NAME);
    if (ini.Load(file)) {

        if(OfflineMode) {
        //	Get the player's last-used Handle
        ini.Get_String("MultiPlayer", "Handle", "Noname", MPlayerName, sizeof(MPlayerName));

        }

        MPlayerPrefColor = 0;
        CurrentVoiceTheme = ini.Get_Int("MultiPlayer", "VoiceTheme", 0);
    }
#endif
}

/***********************************************************************************************
 * Write_MultiPlayer_Settings -- writes multi-player settings to conquer.ini                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
void Write_MultiPlayer_Settings(void)
{
#ifndef REMASTER_BUILD
    INIClass ini;
    CDFileClass file(CONFIG_FILE_NAME);
    if (ini.Load(file)) {
        //	Save the player's last-used Handle & Color
        ini.Put_String("MultiPlayer", "Handle", MPlayerName);
        ini.Put_Int("MultiPlayer", "VoiceTheme", CurrentVoiceTheme);

        //	Write the INI data out to a file.
        ini.Save(file);
    }
#endif
}

/***********************************************************************************************
 * Read_Scenario_Descriptions -- reads multi-player scenario #'s # descriptions                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/14/1995 BR : Created.                                                                  *
 *=============================================================================================*/
void Read_Scenario_Descriptions(void)
{
    INIClass ini;
    CCFileClass file;
    int i;
    char fname[_MAX_FNAME + _MAX_EXT + 4];

    /*------------------------------------------------------------------------
    Clear the scenario description lists
    ------------------------------------------------------------------------*/
    MPlayerScenarios.Clear();
    MPlayerFilenum.Clear();

    /*------------------------------------------------------------------------
    Loop through all possible scenario numbers; if a file is available, add
    its number to the FileNum list.
    ------------------------------------------------------------------------*/
    for (i = 0; i < 100; i++) {
        Set_Scenario_Name(Scen.ScenarioName, i, SCEN_PLAYER_MPLAYER, SCEN_DIR_EAST, SCEN_VAR_A);
        sprintf(fname, "%s.INI", Scen.ScenarioName);
        file.Set_Name(fname);

        if (file.Is_Available()) {
            MPlayerFilenum.Add(i);
        }
    }

    /*------------------------------------------------------------------------
    Now, for every file in the FileNum list, read in the INI file, and extract
    its description.
    ------------------------------------------------------------------------*/
    for (i = 0; i < MPlayerFilenum.Count(); i++) {
        /*.....................................................................
        Fetch working pointer to the INI staging buffer. Make sure that the
        buffer is cleared out before proceeding.
        .....................................................................*/
        ini.Clear();

        /*.....................................................................
        Create filename and read the file.
        .....................................................................*/
        Set_Scenario_Name(Scen.ScenarioName, MPlayerFilenum[i], SCEN_PLAYER_MPLAYER, SCEN_DIR_EAST, SCEN_VAR_A);
        sprintf(fname, "%s.INI", Scen.ScenarioName);
        file.Set_Name(fname);
        ini.Load(file);

        /*.....................................................................
        Extract description & add it to the list.
        .....................................................................*/
        ini.Get_String("Basic", "Name", "Nulls-Ville", MPlayerDescriptions[i], 40);
        MPlayerScenarios.Add(MPlayerDescriptions[i]);
    }
}

/***********************************************************************************************
 * Free_Scenario_Descriptions -- frees memory for the scenario descriptions                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      none.                                                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/05/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
void Free_Scenario_Descriptions(void)
{
    /*------------------------------------------------------------------------
    Clear the scenario descriptions & filenames
    ------------------------------------------------------------------------*/
    MPlayerScenarios.Clear();
    MPlayerFilenum.Clear();
}

/***************************************************************************
 * Surrender_Dialog -- Prompts user for surrendering                       *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      0 = user cancels, 1 = user wants to surrender.                     *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   07/05/1995 BRR : Created.                                             *
 *=========================================================================*/
int Surrender_Dialog(void)
{
    int factor = (SeenBuff.Get_Width() == 320) ? 1 : 2;
    /*........................................................................
    Dialog & button dimensions
    ........................................................................*/
    int d_dialog_w = 170 * factor;                      // dialog width
    int d_dialog_h = 53 * factor;                       // dialog height
    int d_dialog_x = ((320 * factor - d_dialog_w) / 2); // centered x-coord
    int d_dialog_y = ((200 * factor - d_dialog_h) / 2); // centered y-coord
    int d_dialog_cx = d_dialog_x + (d_dialog_w / 2);    // coord of x-center

    int d_txt6_h = 6 * factor + 1; // ht of 6-pt text
    int d_margin = 5 * factor;     // margin width/height
    int d_topmargin = 20 * factor; // top margin

    int d_ok_w = 45 * factor;                                 // ok width
    int d_ok_h = 9 * factor;                                  // ok height
    int d_ok_x = d_dialog_cx - d_ok_w - 5 * factor;           // ok x
    int d_ok_y = d_dialog_y + d_dialog_h - d_ok_h - d_margin; // ok y

    int d_cancel_w = 45 * factor;                                     // cancel width
    int d_cancel_h = 9 * factor;                                      // cancel height
    int d_cancel_x = d_dialog_cx + 5 * factor;                        // cancel x
    int d_cancel_y = d_dialog_y + d_dialog_h - d_cancel_h - d_margin; // cancel y

    /*........................................................................
    Button enumerations
    ........................................................................*/
    enum
    {
        BUTTON_OK = 100,
        BUTTON_CANCEL,
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
    int retcode;

    /*........................................................................
    Buttons
    ........................................................................*/
    ControlClass* commands = NULL; // the button list

    TextButtonClass okbtn(
        BUTTON_OK, TXT_OK, TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW, d_ok_x, d_ok_y, d_ok_w, d_ok_h);

    TextButtonClass cancelbtn(BUTTON_CANCEL,
                              TXT_CANCEL,
                              TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW,
                              d_cancel_x,
                              d_cancel_y,
                              d_cancel_w,
                              d_cancel_h);

    /*
    ------------------------------- Initialize -------------------------------
    */
    Set_Logic_Page(SeenBuff);

    /*
    ......................... Create the button list .........................
    */
    commands = &okbtn;
    cancelbtn.Add_Tail(*commands);

    /*
    -------------------------- Main Processing Loop --------------------------
    */
    display = REDRAW_ALL;
    process = true;
    while (process) {

        /*
        ** If we have just received input focus again after running in the background then
        ** we need to redraw.
        */
        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = false;
            display = REDRAW_ALL;
        }

        /*
        ........................ Invoke game callback .........................
        */
        if (Main_Loop()) {
            retcode = 0;
            process = false;
        }

        /*
        ...................... Refresh display if needed ......................
        */
        if (display) {

            /*
            ...................... Display the dialog box ......................
            */
            Hide_Mouse();
            if (display >= REDRAW_BACKGROUND) {
                Dialog_Box(d_dialog_x, d_dialog_y, d_dialog_w, d_dialog_h);
                Draw_Caption(TXT_NONE, d_dialog_x, d_dialog_y, d_dialog_w);

                /*
                ....................... Draw the captions .......................
                */
                Fancy_Text_Print(Text_String(TXT_SURRENDER),
                                 d_dialog_cx,
                                 d_dialog_y + d_topmargin,
                                 CC_GREEN,
                                 TBLACK,
                                 TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_NOSHADOW);
            }

            /*
            ........................ Redraw the buttons ........................
            */
            if (display >= REDRAW_BUTTONS) {
                commands->Flag_List_To_Redraw();
            }
            Show_Mouse();
            display = REDRAW_NONE;
        }

        /*
        ........................... Get user input ............................
        */
        input = commands->Input();

        /*
        ............................ Process input ............................
        */
        switch (input) {
        case (KN_RETURN):
        case (BUTTON_OK | KN_BUTTON):
            retcode = 1;
            process = false;
            break;

        case (KN_ESC):
        case (BUTTON_CANCEL | KN_BUTTON):
            retcode = 0;
            process = false;
            break;

        default:
            break;
        }
    }

    /*
    --------------------------- Redraw the display ---------------------------
    */
    HiddenPage.Clear();
    Map.Flag_To_Redraw(true);
    Map.IsToDrawUnknown = true;
    Map.Render();

    return (retcode);
}
