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

/* $Header:   F:\projects\c&c\vcs\code\ini.cpv   2.18   16 Oct 1995 16:48:50   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : INI.CPP                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : July 30, 1995 [BRR]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Assign_Houses -- Assigns multiplayer houses to various players                            *
 *   Clear_Flag_Spots -- Clears flag overlays off the map                                      *
 *   Clip_Move -- moves in given direction from given cell; clips to map                       *
 *   Clip_Scatter -- randomly scatters from given cell; won't fall off map                     *
 *   Create_Units -- Creates infantry & units, for non-base multiplayer                        *
 *   Furthest_Cell -- Finds cell furthest from a group of cells                                *
 *   Place_Flags -- Places flags for multiplayer games                                         *
 *   Read_Scenario_Ini -- Read specified scenario INI file.                                    *
 *   Remove_AI_Players -- Removes the computer AI houses & their units                         *
 *   Scan_Place_Object -- places an object >near< the given cell                               *
 *   Set_Scenario_Name -- Creates the INI scenario name string.                                *
 *   Sort_Cells -- sorts an array of cells by distance                                         *
 *   Write_Scenario_Ini -- Write the scenario INI file.                                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"
#include "common/irandom.h"
#include "ccini.h"

/************************************* Prototypes *********************************************/
static void Sort_Cells(CELL* cells, int numcells, CELL* outcells);
static int Furthest_Cell(CELL* cells, int numcells, CELL* tcells, int numtcells);
static CELL Clip_Scatter(CELL cell, int maxdist);
static CELL Clip_Move(CELL cell, FacingType facing, int dist);

/***********************************************************************************************
 * Set_Scenario_Name -- Creates the INI scenario name string.                                  *
 *                                                                                             *
 *    This routine is used by the scenario loading and saving code. It generates the scenario  *
 *    INI root file name for the specified scenario parameters.                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *         buf         buffer to store filename in; must be long enough for root.ext           *
 *       scenario      scenario number                                                         *
 *       player      player type for this game (GDI, NOD, multi-player, ...)                   *
 *       dir         directional parameter for this game (East/West)                           *
 *       var         variation of this game (Lose, A/B/C/D, etc)                               *
 *                                                                                             *
 * OUTPUT:  none.                                                                              *
 *                                                                                             *
 * WARNINGS:   none.                                                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *   05/01/1995 BRR : 2-player scenarios use same names as multiplayer                         *
 *=============================================================================================*/
void Set_Scenario_Name(char* buf, int scenario, ScenarioPlayerType player, ScenarioDirType dir, ScenarioVarType var)
{
    char c_player; // character representing player type
    char c_dir;    // character representing direction type
    char c_var;    // character representing variation type
    ScenarioVarType i;
    char fname[_MAX_FNAME + _MAX_EXT];

    /*
    ** Set the player-type value.
    */
    switch (player) {
    case SCEN_PLAYER_GDI:
        c_player = HouseTypeClass::As_Reference(HOUSE_GOOD).Prefix;
        //			c_player = 'G';
        break;

    case SCEN_PLAYER_NOD:
        c_player = HouseTypeClass::As_Reference(HOUSE_BAD).Prefix;
        //			c_player = 'B';
        break;

    case SCEN_PLAYER_JP:
        c_player = HouseTypeClass::As_Reference(HOUSE_JP).Prefix;
        //			c_player = 'J';
        break;

    /*
    **	Multi player scenario.
    */
    default:
        //c_player = HouseTypeClass::As_Reference(HOUSE_MULTI1).Prefix;
        c_player = 'S';
        break;
    }

    /*
    ** Set the directional character value.
    ** If SCEN_DIR_NONE is specified, randomly pick a direction; otherwise, use 'E' or 'W'
    */
    switch (dir) {
    case SCEN_DIR_EAST:
        c_dir = 'E';
        break;

    case SCEN_DIR_WEST:
        c_dir = 'W';
        break;

    default:
    case SCEN_DIR_NONE:
        c_dir = (Random_Pick(0, 1) == 0) ? 'W' : 'E';
        break;
    }

    /*
    ** Set the variation value.
    */
    if (var == SCEN_VAR_NONE) {

        /*
        ** Find which variations are available for this scenario
        */
        for (i = SCEN_VAR_FIRST; i < SCEN_VAR_COUNT; i++) {
            sprintf(fname, "SC%c%02d%c%c.INI", c_player, scenario, c_dir, 'A' + i);
            if (!CCFileClass(fname).Is_Available()) {
                break;
            }
        }

        if (i == SCEN_VAR_FIRST) {
            c_var = 'X'; // indicates an error
        } else {
            c_var = 'A' + Random_Pick(0, i - 1);
        }
    } else {
        switch (var) {
        case SCEN_VAR_A:
            c_var = 'A';
            break;

        case SCEN_VAR_B:
            c_var = 'B';
            break;

        case SCEN_VAR_C:
            c_var = 'C';
            break;

        case SCEN_VAR_D:
            c_var = 'D';
            break;

        default:
            c_var = 'L';
            break;
        }
    }

    /*
    ** generate the filename
    */
    sprintf(buf, "SC%c%02d%c%c", c_player, scenario, c_dir, c_var);
}

extern void GlyphX_Assign_Houses(void); // ST - 6/25/2019 11:08AM
extern void Assign_Houses(void);
/***********************************************************************************************
 * Read_Scenario_Ini -- Read specified scenario INI file.                                      *
 *                                                                                             *
 *    Read in the scenario INI file. This routine only sets the game                           *
 *    globals with that data that is explicitly defined in the INI file.                       *
 *    The remaining necessary interpolated data is generated elsewhere.                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *          root      root filename for scenario file to read                                  *
 *                                                                                             *
 *          fresh      true = should the current scenario be cleared?                          *
 *                                                                                             *
 * OUTPUT:  bool; Was the scenario read successful?                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Read_Scenario_Ini(char* root, bool fresh)
{
    char fname[_MAX_FNAME + _MAX_EXT]; // full INI filename
    char buf[128];                     // Working string staging buffer.
    int len;
    unsigned char val;

    ScenarioInit++;

    char tmp[128];
    sprintf(tmp, "Read_Scenario_Ini: %s\n", root);
    CCDebugString(tmp);

    if (fresh) {
        Clear_Scenario();
    }

    /*
    ** If we are not dealing with scenario 1, or a multi player scenario
    ** then make sure the correct disk is in the drive.
    */
    if (RequiredCD != -2) {
        if (Scen.Scenario >= 20 && Scen.Scenario < 60 && GameToPlay == GAME_NORMAL) {
            RequiredCD = 2;
        } else {
            if (Scen.Scenario != 1) {
                if (Scen.Scenario >= 60) {
                    RequiredCD = -1;
                } else {
                    switch (ScenPlayer) {
                    case SCEN_PLAYER_GDI:
                        RequiredCD = 0;
                        break;
                    case SCEN_PLAYER_NOD:
                        RequiredCD = 1;
                        break;
                    default:
                        RequiredCD = -1;
                        break;
                    }
                }
            } else {
                RequiredCD = -1;
            }
        }
    }
    if (!Force_CD_Available(RequiredCD)) {
        Prog_End("Read_Scenario_Ini - CD not found", true);
        if (!RunningAsDLL) {
            exit(EXIT_FAILURE);
        }
    }

    /*
    **	Create scenario filename and read the file.
    */

    sprintf(fname, "%s.INI", root);
    CCINIClass ini;
    CCFileClass file(fname);

    int result = ini.Load(file, true);
    if (result == 0) {
        GlyphX_Debug_Print("Failed to load scenario file");
        GlyphX_Debug_Print(fname);
        return (false);
    } else {

        GlyphX_Debug_Print("Opened scenario file");
        GlyphX_Debug_Print(fname);
    }

    /*
    ** Init the Scenario CRC value
    */
    ScenarioCRC = ini.Get_Unique_ID();

    /*
    **	Fetch the appropriate movie names from the INI file.
    */
    ini.Get_String("Basic", "Intro", "x", IntroMovie, sizeof(IntroMovie));
    ini.Get_String("Basic", "Brief", "x", BriefMovie, sizeof(BriefMovie));
    ini.Get_String("Basic", "Win", "x", WinMovie, sizeof(WinMovie));
    ini.Get_String("Basic", "Win2", "x", WinMovie2, sizeof(WinMovie2));
    ini.Get_String("Basic", "Win3", "x", WinMovie3, sizeof(WinMovie3));
    ini.Get_String("Basic", "Win4", "x", WinMovie4, sizeof(WinMovie4));
    ini.Get_String("Basic", "Lose", "x", LoseMovie, sizeof(LoseMovie));
    ini.Get_String("Basic", "Action", "x", ActionMovie, sizeof(ActionMovie));

    /*
    **	For single-player scenarios, 'BuildLevel' is the scenario number.
    **	This must be set before any buildings are created (if a factory is created,
    **	it needs to know the BuildLevel for the sidebar.)
    */
    if (GameToPlay == GAME_NORMAL) {
#ifdef NEWMENU
        if (Scen.Scenario <= 15) {
            BuildLevel = Scen.Scenario;
        } else if (_stricmp(Scen.ScenarioName, "scg30ea") == 0 || _stricmp(Scen.ScenarioName, "scg90ea") == 0
                   || _stricmp(Scen.ScenarioName, "scb22ea") == 0) {
            // N64 missions require build level 15
            BuildLevel = 15;
        } else {
            BuildLevel = ini.Get_Int("Basic", "BuildLevel", Scen.Scenario);
        }
#else
        BuildLevel = Scen.Scenario;
#endif
    }

    /*
    **	Jurassic scenarios are allowed to build the full multiplayer set
    **	of objects.
    */
    if (Special.IsJurassic) {
        BuildLevel = 98;
    }

    /*
    **	Fetch the transition theme for this scenario.
    */
    Scen.TransitTheme = ini.Get_ThemeType("Basic", "Theme", THEME_NONE);

    /*
    **	Read in the team-type data. The team types must be created before any
    **	triggers can be created.
    */
    TeamTypeClass::Read_INI(ini);
    Call_Back();

    /*
    **	Read in the specific information for each of the house types.  This creates
    **	the houses of different types.
    */
    HouseClass::Read_INI(ini);
    Call_Back();

    /*
    **	Read in the trigger data. The triggers must be created before any other
    **	objects can be initialized.
    */
    TriggerClass::Read_INI(ini);
    Call_Back();

    /*
    **	Read in the map control values. This includes dimensions
    **	as well as theater information.
    */
    Map.Read_INI(ini);
    Call_Back();

    /*
    **	Assign PlayerPtr by reading the player's house from the INI;
    **	Must be done before any TechnoClass objects are created.
    */
    //	if (GameToPlay == GAME_NORMAL && (ScenPlayer == SCEN_PLAYER_GDI || ScenPlayer == SCEN_PLAYER_NOD)) {
    if (GameToPlay == GAME_NORMAL) {
        Scen.CarryOverPercent = ini.Get_Int("Basic", "CarryOverMoney", 100);
        Scen.CarryOverPercent = Cardinal_To_Fixed(100, Scen.CarryOverPercent);
        Scen.CarryOverCap = ini.Get_Int("Basic", "CarryOverCap", -1);

        PlayerPtr = HouseClass::As_Pointer(ini.Get_HousesType("Basic", "Player", HOUSE_GOOD));
        PlayerPtr->IsHuman = true;

        int carryover;
        if (Scen.CarryOverCap != -1) {
            carryover = MIN((int)Fixed_To_Cardinal(Scen.CarryOverMoney, Scen.CarryOverPercent), Scen.CarryOverCap);
        } else {
            carryover = Fixed_To_Cardinal(Scen.CarryOverMoney, Scen.CarryOverPercent);
        }
        PlayerPtr->Credits += carryover;
        PlayerPtr->InitialCredits += carryover;

        if (Special.IsJurassic) {
            PlayerPtr->ActLike = Whom;
        }

    } else if (GameToPlay == GAME_CLIENT) {
        UnitClass::Set_New_Allowed(false);
        InfantryClass::Set_New_Allowed(false);
        BuildingClass::Set_New_Allowed(false);
        AircraftClass::Set_New_Allowed(false);
        UnitClass::Set_Delete_Allowed(false);
        InfantryClass::Set_Delete_Allowed(false);
        BuildingClass::Set_Delete_Allowed(false);
        AircraftClass::Set_Delete_Allowed(false);
        PlayerPtr = HouseClass::As_Pointer((HousesType)MPlayerLocalID);
        PlayerPtr->IsHuman = true;
        PlayerPtr->ActLike = MPlayerHouse;

        if (MPlayerHouse == HOUSE_BLUE_TEAM || MPlayerHouse == HOUSE_ORANGE_TEAM || MPlayerHouse == HOUSE_GREEN_TEAM
            || MPlayerHouse == HOUSE_GREY_TEAM) {
            PlayerPtr->Int4 = MPlayerHouse - HOUSE_SPECTATOR;
        } else {
            PlayerPtr->Int4 = -1;
        }
        if (MPlayerHouse == HOUSE_GOOD || MPlayerHouse == HOUSE_BAD) {
            if (PlayerPtr->IsHuman) {
                PlayerPtr->Init_Data(REMAP_GOLD, MPlayerHouse, 0);
            } else {
                PlayerPtr->Init_Data(REMAP_RED, MPlayerHouse, 0);
            }
        }

        HouseClass::As_Pointer(HOUSE_GOOD)->Init_Data(REMAP_RED, HOUSE_GOOD, 10000);
        HouseClass::As_Pointer(HOUSE_BAD)->Init_Data(REMAP_RED, HOUSE_BAD, 10000);

    } else if (GameToPlay == GAME_HOST) {
        PlayerPtr = HouseClass::As_Pointer(HOUSE_ADMIN);
        PlayerPtr->IsHuman = true;
        strcpy(PlayerPtr->Name, "Server");
        HouseClass::As_Pointer(HOUSE_GOOD)->Init_Data(REMAP_RED, HOUSE_GOOD, 10000);
        HouseClass::As_Pointer(HOUSE_BAD)->Init_Data(REMAP_RED, HOUSE_BAD, 10000);
    } else {

#ifdef OBSOLETE
        if (GameToPlay == GAME_NORMAL && ScenPlayer == SCEN_PLAYER_JP) {
            PlayerPtr = HouseClass::As_Pointer(HOUSE_MULTI4);
            PlayerPtr->IsHuman = true;
            PlayerPtr->Credits += CarryOverMoney;
            PlayerPtr->InitialCredits += CarryOverMoney;
            PlayerPtr->ActLike = Whom;
        } else {
            Assign_Houses();
        }
#endif
#ifdef REMASTER_BUILD
        // Call new Assign_Houses function. ST - 6/25/2019 11:07AM
        // Assign_Houses();
        GlyphX_Assign_Houses();
#else
        //Assign_Houses();
#endif
    }

#ifndef REMASTER_BUILD
    // This is needed to fix the object selection issues. OmniBlade - 09/07/2020
    CurrentObject.Set_Active_Context(PlayerPtr->Class->House);
#endif

    /*
    **	Attempt to read the map's binary image file; if fails, read the
    **	template data from the INI, for backward compatibility
    */
    if (fresh) {
        if (!Map.Read_Binary(root, &ScenarioCRC)) {
            TemplateClass::Read_INI(ini);
        }
    }
    Call_Back();

    /*
    **	Read in and place the 3D terrain objects.
    */
    TerrainClass::Read_INI(ini);
    Call_Back();

    /*
    **	Read in and place the units (all sides).
    */
    UnitClass::Read_INI(ini);
    Call_Back();

    AircraftClass::Read_INI(ini);
    Call_Back();

    /*
    **	Read in and place the infantry units (all sides).
    */
    InfantryClass::Read_INI(ini);
    Call_Back();

    /*
    **	Read in and place all the buildings on the map.
    */
    BuildingClass::Read_INI(ini);
    Call_Back();

    /*
    **	Read in the AI's base information.
    */
    Base.Read_INI(ini);
    Call_Back();

    /*
    **	Read in any normal overlay objects.
    */
    OverlayClass::Read_INI(ini);
    Call_Back();

    /*
    **	Read in any smudge overlays.
    */
    SmudgeClass::Read_INI(ini);
    Call_Back();

    WDTCrateShares[WDT_CRATE_STRENGTH] = ini.Get_Int("Crates", "AddStrength", 100);
    WDTCrateShares[WDT_CRATE_WEAPON] = ini.Get_Int("Crates", "AddWeapon", 100);
    WDTCrateShares[WDT_CRATE_SPEED] = ini.Get_Int("Crates", "AddSpeed", 150);
    WDTCrateShares[WDT_CRATE_RELOAD] = ini.Get_Int("Crates", "RapidReload", 100);
    WDTCrateShares[WDT_CRATE_RANGE] = ini.Get_Int("Crates", "AddRange", 100);
    WDTCrateShares[WDT_CRATE_HEAL] = ini.Get_Int("Crates", "Heal", 200);
    WDTCrateShares[WDT_CRATE_BOMB] = ini.Get_Int("Crates", "Bomb", 200);
    WDTCrateShares[WDT_CRATE_STEALTH] = ini.Get_Int("Crates", "Stealth", 50);
    WDTCrateShares[WDT_CRATE_TELEPORT] = ini.Get_Int("Crates", "Teleport", 50);
    WDTCrateShares[WDT_CRATE_KILL] = ini.Get_Int("Crates", "Kill", 0);
    WDTCrateShares[WDT_CRATE_UNCLOAK_ALL] = ini.Get_Int("Crates", "UncloakAll", 10);
    WDTCrateShares[WDT_CRATE_RESHOUD] = ini.Get_Int("Crates", "Reshroud", 5);
    WDTCrateShares[WDT_CRATE_UNSHROUD] = ini.Get_Int("Crates", "Unshroud", 5);
    WDTCrateShares[WDT_CRATE_RADAR] = ini.Get_Int("Crates", "Radar", 20);
    WDTCrateShares[WDT_CRATE_ARMAGEDDON] = ini.Get_Int("Crates", "Armageddon", 1);
    WDTCrateShares[WDT_CRATE_SUPER] = ini.Get_Int("Crates", "Super", 1);
    WDTCrateDensity = ini.Get_Int("Crates", "Density", 200);
    WDTCrateIonFactor = ini.Get_Int("Crates", "IonFactor", 400);
    WDTCrateTimerVal = ini.Get_Int("Crates", "CrateTimer", 300);

    /*
    **	Read in any briefing text.
    */
    ini.Get_TextBlock("Briefing", Scen.BriefingText, sizeof(Scen.BriefingText));

    /*
    **	If the briefing text could not be found in the INI file, then search
    **	the mission.ini file.
    */
    if (Scen.BriefingText[0] == '\0') {
        INIClass mini;
        CCFileClass missionIniFile("MISSION.INI");
        mini.Load(missionIniFile);
        mini.Get_TextBlock(root, Scen.BriefingText, sizeof(Scen.BriefingText));
    }

    /*
    **	Perform a final overpass of the map. This handles smoothing of certain
    **	types of terrain (tiberium).
    */
    Map.Overpass();
    Call_Back();

    if (GameToPlay == GAME_HOST) {
        MPlayerCredits = 500;
        MPlayerBases = 0;
        MPlayerTiberium = 0;
        MPlayerGoodies = 1;
        MPlayerGhosts = 0;
        BuildLevel = 7;
        MPlayerUnitCount = 1;
        Special.IsTGrowth = false;
        Special.IsTSpread = false;
        Special.IsJurassic = true;
    }

    /*
    **	Return with flag saying that the scenario file was read.
    */
    ScenarioInit--;
    return (true);
}

/***********************************************************************************************
 * Write_Scenario_Ini -- Write the scenario INI file.                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      root      root filename for the scenario                                               *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *   05/11/1995 JLB : Updates movie data.                                                      *
 *=============================================================================================*/
void Write_Scenario_Ini(char* root)
{
#ifndef CHEAT_KEYS
    root = root;
#else
    char fname[_MAX_FNAME + _MAX_EXT]; // full scenario name
    HousesType house;
    CCINIClass ini;

    /*
    **	Preload the old scenario if it is present because there may
    **	be some fields in the INI that are processed but not written
    **	out. Preloading the scenario will preserve these manually
    **	maintained entries.
    */
    sprintf(fname, "%s.INI", root);
    if (CCFileClass(fname).Is_Available()) {
        CCFileClass file(fname);
        ini.Load(file, true);
    }

    ini.Clear("Basic");

    ini.Put_String("Basic", "Intro", IntroMovie);
    ini.Put_String("Basic", "Brief", BriefMovie);
    ini.Put_String("Basic", "Win", WinMovie);
    ini.Put_String("Basic", "Win2", WinMovie);
    ini.Put_String("Basic", "Win3", WinMovie);
    ini.Put_String("Basic", "Win4", WinMovie);
    ini.Put_String("Basic", "Lose", LoseMovie);
    ini.Put_String("Basic", "Action", ActionMovie);
    ini.Put_String("Basic", "Player", PlayerPtr->Class->IniName);
    ini.Put_String("Basic", "Theme", Theme.Base_Name(Scen.TransitTheme));
    ini.Put_Int("Basic", "BuildLevel", BuildLevel);
    ini.Put_Int("Basic", "CarryOverMoney", Fixed_To_Cardinal(100, Scen.CarryOverPercent));
    ini.Put_Int("Basic", "CarryOverCap", Scen.CarryOverCap);

    TeamTypeClass::Write_INI(ini, true);
    TriggerClass::Write_INI(ini, true);
    Map.Write_INI(ini);
    Map.Write_Binary(root);
    HouseClass::Write_INI(ini);
    UnitClass::Write_INI(ini);
    InfantryClass::Write_INI(ini);
    BuildingClass::Write_INI(ini);
    TerrainClass::Write_INI(ini);
    OverlayClass::Write_INI(ini);
    SmudgeClass::Write_INI(ini);

    Base.Write_INI(ini);

    if (strlen(Scen.BriefingText)) {
        ini.Put_TextBlock("Briefing", Scen.BriefingText);
    }

    /*
    **	Write the scenario data out to a file.
    */
    RawFileClass rawfile(fname);
    ini.Save(rawfile, true);
#endif
}

bool Make_Player_Unit(int player_index)
{
    TechnoClass* tptr;
    const TechnoTypeClass* ttptr;
    AircraftClass* spawn;
    int x;
    int y;
    int index;
    int waypoint;
    CELL cell;

    ttptr = Fetch_Techno_Type(ActivePlayers[player_index]->RTTI, ActivePlayers[player_index]->Type);

    if (ttptr == NULL) {
        return false;
    }

    waypoint = ActivePlayers[player_index]->HousePtr->Int4 - 1;

    /*
	** Decide where to put the new unit.
	*/
    if (GameParams.IsCaptureTheFlag && waypoint >= 0 && waypoint <= 3) {
        cell = FlagHomes[waypoint];
    } else if (GameParams.Football && waypoint >= 0 && waypoint <= 3 && WDTGameTimer.Time() < 600) {
        int i = 0;
        do {
            int rx;
            int ry;
            ry = Random_Pick(-6, 7);
            rx = Random_Pick(-6, 6);
            cell = FlagHomes[waypoint] + ry + (rx << 7);
            i++;
        } while (i <= 100
                 && (!Map.In_Radar(cell) || Distance(cell, FlagHomes[waypoint]) < 5
                     || Distance(cell, FlagHomes[waypoint]) > 8));

        if (i > 100) {
            cell = FlagHomes[waypoint];
        }
    } else {
        x = Random_Pick(0, Map.MapCellWidth - 2) + Map.MapCellX;
        y = Random_Pick(0, Map.MapCellHeight - 1) + Map.MapCellY;
        cell = XY_Cell(x, y);
    }

    /*
	** Place the new unit.
	*/
    for (index = 0; index < ActivePlayers[player_index]->NumStartingUnits; index++) {
        tptr = (TechnoClass*)ttptr->Create_One_Of(ActivePlayers[player_index]->HousePtr);

        if (tptr != NULL) {
            if (!Try_Place_Object(tptr, cell, GameParams.IsCaptureTheFlag)) {
                //DELETE_OBJ(tptr, sizeof(AbstractClass));
                delete tptr;
                tptr = NULL;
            }

            ActivePlayers[player_index]->Technos.Add(tptr);
        }
    }

    /*
	** Handle special Aircraft logic.
	*/
    if (tptr != NULL && ActivePlayers[player_index]->RTTI == RTTI_BUILDINGTYPE
        && ActivePlayers[player_index]->Type == STRUCT_HELIPAD) {
        if (ActivePlayers[player_index]->HousePtr->ActLike == HOUSE_GOOD) {
            spawn = new AircraftClass(AIRCRAFT_ORCA, ActivePlayers[player_index]->HousePtr->Class->House);
        } else {
            spawn = new AircraftClass(AIRCRAFT_HELICOPTER, ActivePlayers[player_index]->HousePtr->Class->House);
        }

        if (spawn != NULL) {
            ScenarioInit++;
            spawn->Unlimbo(tptr->Docking_Coord(), (spawn)->Pose_Dir());
            spawn->Assign_Mission(MISSION_GUARD);
            ScenarioInit--;
            ActivePlayers[player_index]->Technos.Add(spawn);
            spawn->Select();
        }
    }

    return true;
}

bool Try_Place_Unknown(int, int, char)
{
    return true;
}

// Matching
bool Try_Place_Object(ObjectClass* obj, CELL cell, bool is_ctf)
{
    int fcounter;
    int new_y;
    int new_x;
    int dist;
    TechnoClass* techno;
    int cell_y;
    int cell_x;
    int tryval;
    CELL newcell;
    int offx[] = {1, 0, -1, 0};
    int offy[] = {0, 1, 0, -1};

    tryval = 0; // TODO unneeded.
    cell_x = Cell_X(cell);
    cell_y = Cell_Y(cell);

    /*------------------------------------------------------------------------
	First try to unlimbo the object in the given cell.
	------------------------------------------------------------------------*/
    if (!is_ctf) {
        if (Map.In_Radar(cell)) {
            techno = Map[cell].Cell_Techno();
            if (!techno || (techno->What_Am_I() == RTTI_INFANTRY && obj->What_Am_I() == RTTI_INFANTRY)) {
                if (obj->Unlimbo(Cell_Coord(cell), DIR_N)) {
                    return true;
                }
            }
        }
    }

    for (dist = 1; dist < 20; dist++) {
        new_x = cell_x - dist;
        new_y = cell_y - dist;

        for (tryval = 0; tryval <= 3; tryval++) {
            for (fcounter = 0; fcounter < 2 * dist; fcounter++) {
                newcell = XY_Cell(new_x, new_y);

                if (Map.In_Radar(newcell)) {
                    techno = Map[newcell].Cell_Techno();
                    if (!techno || (techno->What_Am_I() == RTTI_INFANTRY && obj->What_Am_I() == RTTI_INFANTRY)) {
                        if (obj->Unlimbo(Cell_Coord(newcell), DIR_N)) {
                            return true;
                        }
                    }
                }

                new_x += offx[tryval];
                new_y += offy[tryval];
            }
        }
    }

    return false;
}

// Matching
CELL Try_Place_Overlay(OverlayType overlay, CELL cell)
{
    int fcounter;
    int new_y;
    int new_x;
    int dist;
    CellClass* cellc;
    int cell_y;
    int cell_x;
    int tryval;
    CELL newcell;
    int offx[] = {1, 0, -1, 0};
    int offy[] = {0, 1, 0, -1};

    tryval = 0; // TODO unneeded.
    cell_x = Cell_X(cell);
    cell_y = Cell_Y(cell);

    if (Map.In_Radar(cell)) {
        cellc = &Map[cell];

        if (cellc->Is_Generally_Clear() && cellc->Overlay == OVERLAY_NONE) {
            cellc->Overlay = overlay;
            cellc->OverlayData = 0;
            cellc->Redraw_Objects();
            return cell;
        }
    }

    for (dist = 1; dist < 20; dist++) {
        new_x = cell_x - dist;
        new_y = cell_y - dist;

        for (tryval = 0; tryval <= 3; tryval++) {
            for (fcounter = 0; fcounter < 2 * dist; fcounter++) {
                newcell = XY_Cell(new_x, new_y);

                if (Map.In_Radar(newcell)) {
                    cellc = &Map[newcell];

                    if (cellc->Is_Generally_Clear() && cellc->Overlay == OVERLAY_NONE) {
                        cellc->Overlay = overlay;
                        cellc->OverlayData = 0;
                        cellc->Redraw_Objects();
                        return newcell;
                    }
                }

                new_x += offx[tryval];
                new_y += offy[tryval];
            }
        }
    }

    return -1;
}

/***********************************************************************************************
 * Scan_Place_Object -- places an object >near< the given cell                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      obj      ptr to object to Unlimbo                                                      *
 *      cell      center of search area                                                        *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      true = object was placed; false = it wasn't                                            *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/09/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
int Scan_Place_Object(ObjectClass* obj, CELL cell)
{
    int dist;            // for object placement
    FacingType rot;      // for object placement
    FacingType fcounter; // for object placement
    int tryval;
    CELL newcell;
    TechnoClass* techno;
    int skipit;

    /*------------------------------------------------------------------------
    First try to unlimbo the object in the given cell.
    ------------------------------------------------------------------------*/
    if (Map.In_Radar(cell)) {
        techno = Map[cell].Cell_Techno();
        if (!techno || (techno->What_Am_I() == RTTI_INFANTRY && obj->What_Am_I() == RTTI_INFANTRY)) {
            if (obj->Unlimbo(Cell_Coord(cell), DIR_N)) {
                return (true);
            }
        }
    }

    /*------------------------------------------------------------------------
    Loop through distances from the given center cell; skip the center cell.
    For each distance, try placing the object along each rotational direction;
    if none are available, try each direction with a random scatter value.
    If that fails, go to the next distance.
    This ensures that the closest coordinates are filled first.
    ------------------------------------------------------------------------*/
    for (dist = 1; dist < 32; dist++) {
        /*.....................................................................
        Pick a random starting direction
        .....................................................................*/
        rot = Random_Pick(FACING_N, FACING_NW);
        /*.....................................................................
        Try all directions twice
        .....................................................................*/
        for (tryval = 0; tryval < 2; tryval++) {
            /*..................................................................
            Loop through all directions, at this distance.
            ..................................................................*/
            for (fcounter = FACING_N; fcounter <= FACING_NW; fcounter++) {

                skipit = false;

                /*...............................................................
                Pick a coordinate along this directional axis
                ...............................................................*/
                newcell = Clip_Move(cell, rot, dist);

                /*...............................................................
                If this is our second try at this distance, add a random scatter
                to the desired cell, so our units aren't all aligned along spokes.
                ...............................................................*/
                if (tryval > 0)
                    newcell = Clip_Scatter(newcell, 1);

                /*...............................................................
                If, by randomly scattering, we've chosen the exact center, skip
                it & try another direction.
                ...............................................................*/
                if (newcell == cell)
                    skipit = true;

                if (!skipit) {
                    /*............................................................
                    Only attempt to Unlimbo the object if:
                    - there is no techno in the cell
                    - the techno in the cell & the object are both infantry
                    ............................................................*/
                    techno = Map[newcell].Cell_Techno();
                    if (!techno || (techno->What_Am_I() == RTTI_INFANTRY && obj->What_Am_I() == RTTI_INFANTRY)) {
                        if (obj->Unlimbo(Cell_Coord(newcell), DIR_N)) {
                            return (true);
                        }
                    }
                }

                rot++;
                if (rot > FACING_NW)
                    rot = FACING_N;
            }
        }
    }

    return (false);
}

/***********************************************************************************************
 * Sort_Cells -- sorts an array of cells by distance                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      cells         array to sort                                                            *
 *      numcells      # entries in 'cells'                                                     *
 *      outcells      array to store sorted values in                                          *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      none.                                                                                  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
static void Sort_Cells(CELL* cells, int numcells, CELL* outcells)
{
    int i, j, k;
    int num_sorted = 0;
    int num_unsorted = numcells;

    /*------------------------------------------------------------------------
    Pick the first cell at random
    ------------------------------------------------------------------------*/
    j = Random_Pick(0, numcells - 1);
    outcells[0] = cells[j];
    num_sorted++;

    for (k = j; k < num_unsorted - 1; k++) {
        cells[k] = cells[k + 1];
    }
    num_unsorted--;

    /*------------------------------------------------------------------------
    After the first cell, assign the other cells based on who's furthest away
    from the chosen ones.
    ------------------------------------------------------------------------*/
    for (i = 0; i < numcells; i++) {
        j = Furthest_Cell(outcells, num_sorted, cells, num_unsorted);
        outcells[num_sorted] = cells[j];
        num_sorted++;

        for (k = j; k < num_unsorted - 1; k++) {
            cells[k] = cells[k + 1];
        }
        num_unsorted--;
    }
}

/***********************************************************************************************
 * Furthest_Cell -- Finds cell furthest from a group of cells                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      cells            array of cells to find furthest-cell-away-from                        *
 *      numcells         # entries in 'cells'                                                  *
 *      tcells         array of cells to test; one of these will be selected as being          *
 *                     "furthest" from all the cells in 'cells'                                *
 *      numtcells      # entries in 'tcells'                                                   *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      index of 'tcell' that's furthest away from 'cells'                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
static int Furthest_Cell(CELL* cells, int numcells, CELL* tcells, int numtcells)
{
    int i;
    int j;
    int mindist;    // minimum distance a 'tcell' is from a 'cell'
    int maxmindist; // the highest mindist value of all tcells
    int maxmin_idx; // index of the tcell with largest mindist
    int dist;       // working distance measure

    /*------------------------------------------------------------------------
    Initialize
    ------------------------------------------------------------------------*/
    maxmindist = 0;
    maxmin_idx = 0;

    /*------------------------------------------------------------------------
    Loop through all test cells, finding the furthest one from all entries in
    the 'cells' array
    ------------------------------------------------------------------------*/
    for (i = 0; i < numtcells; i++) {

        /*.....................................................................
        Find the 'cell' closest to this 'tcell'
        .....................................................................*/
        mindist = 0xffff;
        for (j = 0; j < numcells; j++) {
            dist = Distance(tcells[i], cells[j]);
            if (dist <= mindist) {
                mindist = dist;
            }
        }

        /*.....................................................................
        If this tcell is further away than the others, save its distance &
        index value
        .....................................................................*/
        if (mindist >= maxmindist) {
            maxmindist = mindist;
            maxmin_idx = i;
        }
    }

    return (maxmin_idx);
}

/***********************************************************************************************
 * Clip_Scatter -- randomly scatters from given cell; won't fall off map                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      cell      cell to scatter from                                                         *
 *      maxdist   max distance to scatter                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      new cell number                                                                        *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
static CELL Clip_Scatter(CELL cell, int maxdist)
{
    int x, y;
    int xdist;
    int ydist;
    int xmin, xmax;
    int ymin, ymax;

    /*------------------------------------------------------------------------
    Get X & Y coords of given starting cell
    ------------------------------------------------------------------------*/
    x = Cell_X(cell);
    y = Cell_Y(cell);

    /*------------------------------------------------------------------------
    Compute our x & y limits
    ------------------------------------------------------------------------*/
    xmin = Map.MapCellX;
    xmax = xmin + Map.MapCellWidth - 1;
    ymin = Map.MapCellY;
    ymax = ymin + Map.MapCellHeight - 1;

    /*------------------------------------------------------------------------
    Adjust the x-coordinate
    ------------------------------------------------------------------------*/
    xdist = Random_Pick(0, maxdist);
    if (Random_Pick(0, 1) == 0) {
        x += xdist;
        if (x > xmax) {
            x = xmax;
        }
    } else {
        x -= xdist;
        if (x < xmin) {
            x = xmin;
        }
    }

    /*------------------------------------------------------------------------
    Adjust the y-coordinate
    ------------------------------------------------------------------------*/
    ydist = Random_Pick(0, maxdist);
    if (Random_Pick(0, 1) == 0) {
        y += ydist;
        if (y > ymax) {
            y = ymax;
        }
    } else {
        y -= ydist;
        if (y < ymin) {
            y = ymin;
        }
    }

    return (XY_Cell(x, y));
}

/***********************************************************************************************
 * Clip_Move -- moves in given direction from given cell; clips to map                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      cell      cell to start from                                                           *
 *      facing   direction to move                                                             *
 *      dist      distance to move                                                             *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      new cell number                                                                        *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1995 BRR : Created.                                                                 *
 *=============================================================================================*/
static CELL Clip_Move(CELL cell, FacingType facing, int dist)
{
    int x, y;
    int xmin, xmax;
    int ymin, ymax;

    /*------------------------------------------------------------------------
    Get X & Y coords of given starting cell
    ------------------------------------------------------------------------*/
    x = Cell_X(cell);
    y = Cell_Y(cell);

    /*------------------------------------------------------------------------
    Compute our x & y limits
    ------------------------------------------------------------------------*/
    xmin = Map.MapCellX;
    xmax = xmin + Map.MapCellWidth - 1;
    ymin = Map.MapCellY;
    ymax = ymin + Map.MapCellHeight - 1;

    /*------------------------------------------------------------------------
    Adjust the x-coordinate
    ------------------------------------------------------------------------*/
    switch (facing) {
    case FACING_N:
        y -= dist;
        break;

    case FACING_NE:
        x += dist;
        y -= dist;
        break;

    case FACING_E:
        x += dist;
        break;

    case FACING_SE:
        x += dist;
        y += dist;
        break;

    case FACING_S:
        y += dist;
        break;

    case FACING_SW:
        x -= dist;
        y += dist;
        break;

    case FACING_W:
        x -= dist;
        break;

    case FACING_NW:
        x -= dist;
        y -= dist;
        break;
    }

    /*------------------------------------------------------------------------
    Clip to the map
    ------------------------------------------------------------------------*/
    if (x > xmax)
        x = xmax;
    if (x < xmin)
        x = xmin;

    if (y > ymax)
        y = ymax;
    if (y < ymin)
        y = ymin;

    return (XY_Cell(x, y));
}

bool Init_Flag_Homes()
{
    int allocated[20];  // [esp+0h] [ebp-60h]
    int waypoint_check; // [esp+54h] [ebp-Ch]
    int try_count;      // [esp+58h] [ebp-8h]
    int index;          // [esp+5Ch] [ebp-4h]

    index = 0;
    for (waypoint_check = 0; waypoint_check < 20; waypoint_check++) {
        allocated[waypoint_check] = 0;
    }
    for (index = 0; index < 4; index++) {
        for (try_count = 0; try_count < 100; try_count++) {
            waypoint_check = WDT_Random_Pick(0, 9);
            if (Scen.Waypoint[waypoint_check + 4] != -1 && !allocated[waypoint_check]) {
                FlagHomes[index] = Scen.Waypoint[waypoint_check + 4];
                allocated[waypoint_check] = 1;
                break;
            }
        }
        if (try_count == 100) {
            for (waypoint_check = 0; waypoint_check < 10; waypoint_check++) {
                if (Scen.Waypoint[waypoint_check + 4] != -1 && !allocated[waypoint_check]) {
                    FlagHomes[index] = Scen.Waypoint[waypoint_check + 4];
                    allocated[waypoint_check] = 1;
                    break;
                }
            }
            if (waypoint_check == 10) {
                return 0;
            }
        }
    }
    return 1;
}

// Matching
void Setup_House_Flags(HousesType house)
{
    HouseClass* h;
    CELL rr3;
    CELL rr2;
    CELL rr1;
    int i;
    HouseClass* hptr;

    int team = house - 6;
    if (team >= 0 && team <= 3) {
        hptr = HouseClass::As_Pointer(house);
        if (hptr) {
            if (hptr->FlagHome == 0) {
                if (GameParams.IsCaptureTheFlag) {
                    hptr->Flag_Attach(FlagHomes[team], 0);
                    hptr->Make_CTF_Packet_Dropped(FlagHomes[team]);
                } else if (GameParams.Football) {
                    FootballCells[team] = FlagHomes[team == 0] + team;
                    hptr->Make_CTF_Packet_Dropped(FootballCells[team]);

                    if (GameParams.FootballNumFlags == 2) {
                        i = 0;
                        do {
                            rr1 = Random_Pick(-6, 7);
                            rr2 = Random_Pick(-6, 6);
                            rr3 = FlagHomes[team] + rr1 + (rr2 << 7);
                            i++;
                        } while (i <= 100
                                 && (!Map.In_Radar((CELL)rr3) || Distance((CELL)rr3, (CELL)FlagHomes[team]) < 5
                                     || Distance((CELL)rr3, (CELL)FlagHomes[team]) > 8));
                        if (i > 100) {
                            rr3 = FlagHomes[team];
                        }
                        hptr->Flag_Attach(rr3, 0);
                    } else if (team == 1) {
                        h = HouseClass::As_Pointer(HOUSE_GREEN_TEAM);
                        h->Flag_Attach(Cell_Mid(FlagHomes[0], FlagHomes[1]), 0);
                    }
                }
            }
        }
    }
}

// Matching
void Clear_Flags_Of_House(HousesType house)
{
    HouseClass* h;
    HouseClass* hptr;
    int team;

    team = house - 6;
    if (team >= 0 && team <= 3) {
        hptr = HouseClass::As_Pointer(house);
        if (hptr) {
            if (hptr->FlagHome != 0) {
                if (GameParams.Football && GameParams.FootballNumFlags == 1) {
                    if (team == 1) {
                        h = HouseClass::As_Pointer(HOUSE_GREEN_TEAM);
                        h->Flag_Remove(h->FlagLocation);
                    }
                } else {
                    hptr->Flag_Remove(hptr->FlagLocation);
                }
                if (GameParams.Football) {
                    hptr->Make_CTF_Packet_Picked_Up(FootballCells[team]);
                } else {
                    hptr->Make_CTF_Packet_Picked_Up(FlagHomes[team]);
                }
            }
        }
    }
}

// Matching
void Fortify_Flag_Home(HousesType house)
{
    int i;
    HouseClass* hptr;
    int team;
    BuildingClass* bptr;

    bptr = NULL;
    team = house - 6;

    if (team >= 0 && team <= 3) {
        hptr = HouseClass::As_Pointer(house);
        if (!hptr) {
            CCDebugString("ERROR: Fortify_FlagHome error 1\n");
        } else if (hptr->FlagHome == 0) {
            CCDebugString("ERROR: Fortify_FlagHome error 2\n");
        } else {
            for (i = 0; i < GameParams.NumCTFStructures; i++) {
                bptr = Create_Building(true, house, (HousesType)(GameParams.NumCTFStructures / 2 > i));

                if (bptr) {
                    if (!Try_Place_Object(bptr, hptr->FlagHome, 1)) {
                        CCDebugString("ERROR: Fortify_FlagHome error 3\n");
                        delete bptr;
                    }

                    bptr->Mod1 = sole_array[0][2] / 2;
                    bptr->Strength = bptr->Class_Of().MaxStrength + bptr->Mod1;
                    bptr->Mod5 = sole_array[4][2];
                } else {
                    CCDebugString("ERROR: Fortify_FlagHome error 4\n");
                }
            }

            hptr->IsHuman = true;
        }
    }
}

// Matching
void Clear_Buildings_Of_House(HousesType house)
{
    BuildingClass* bptr;

    for (int i = 0; i < Buildings.Count(); i++) {
        bptr = Buildings.Ptr(i);
        if (bptr->IsActive && bptr->Owner() == house) {
            delete bptr;
            i--;
        }
    }
}

void Setup_Flags_Of_House(HousesType house)
{
    int idx2;
    int idx1;
    int state[20];
    int index;
    int team;

    team = house - HOUSE_FIRST_TEAM;
    if (team >= 0 && team <= 3) {
        for (index = 0; index < 20; index++) {
            state[index] = 0;
        }

        Clear_Flags_Of_House(house);

        for (idx1 = 0; idx1 < 4; idx1++) {
            if (idx1 != team) {
                for (index = 0; index < 10; index++) {
                    if (FlagHomes[idx1] == Scen.Waypoint[index + 4]) {
                        state[index] = 1;
                        break;
                    }
                }
            }
        }
        for (idx2 = 0; idx2 < 100; idx2++) {
            index = WDT_Random_Pick(0, 9);
            if (Scen.Waypoint[index + 4] != -1 && !state[index]) {
                FlagHomes[team] = Scen.Waypoint[index + 4];
                break;
            }
        }
        if (idx2 == 100) {
            for (index = 0; index < 10; index++) {
                if (Scen.Waypoint[index + 4] != -1 && !state[index]) {
                    FlagHomes[team] = Scen.Waypoint[index + 4];
                    break;
                }
            }
        }
        Setup_House_Flags(house);
    }
}
