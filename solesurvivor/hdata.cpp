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

/* $Header:   F:\projects\c&c\vcs\code\hdata.cpv   2.17   16 Oct 1995 16:48:18   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : HDATA.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 22, 1994                                                 *
 *                                                                                             *
 *                  Last Update : January 23, 1995 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   HouseTypeClass::From_Name -- Fetch house pointer from its name.                           *
 *   HouseTypeClass::As_Reference -- Fetches a reference to the house specified.               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"

/*
**	These are the colors used to identify the various owners.
*/
const int COLOR_GOOD = 180;           // GOLD
const int COLOR_BRIGHT_GOOD = 176;    // GOLD
const int COLOR_BAD = 123;            // RED;
const int COLOR_BRIGHT_BAD = 127;     // RED;
const int COLOR_NEUTRAL = 205;        // WHITE;
const int COLOR_BRIGHT_NEUTRAL = 202; // WHITE;

static HouseTypeClass const HouseGood(HOUSE_GOOD,
                                      "GoodGuy",         //	NAME:			House name.
                                      TXT_GDI,           // FULLNAME:	Translated house name.
                                      "GDI",             // SUFFIX:		House file suffix.
                                      0,                 // LEMON:		Lemon vehicle frequency.
                                      COLOR_GOOD,        // COLOR:		Dark Radar map color.
                                      COLOR_BRIGHT_GOOD, // COLOR:		Bright Radar map color.
                                      REMAP_GOLD,        // Remap color ID number.
                                      RemapGold,         // Default remap table.
                                      'G'                // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseBad(HOUSE_BAD,
                                     "BadGuy",         //	NAME:			House name.
                                     TXT_NOD,          // FULLNAME:	Translated house name.
                                     "NOD",            // SUFFIX:		House file suffix.
                                     0,                // LEMON:		Lemon vehicle frequency.
                                     COLOR_BAD,        // COLOR:		Dark Radar map color.
                                     COLOR_BRIGHT_BAD, // COLOR:		Bright Radar map color.
                                     REMAP_LTBLUE,     // Remap color ID number.
                                     RemapLtBlue,      // Default remap table.
                                     'B'               // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseCivilian(HOUSE_NEUTRAL,
                                          "Neutral",            //	NAME:			House name.
                                          TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                          "CIV",                // SUFFIX:		House file suffix.
                                          0,                    // LEMON:		Lemon vehicle frequency.
                                          COLOR_NEUTRAL,        // COLOR:		Dark Radar map color.
                                          COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                          REMAP_NONE,           // Remap color ID number.
                                          RemapNone,            // Default remap table.
                                          'C'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseJP(HOUSE_JP,
                                    "Special",            //	NAME:			House name.
                                    TXT_JP,               // FULLNAME:	Translated house name.
                                    "JP",                 // SUFFIX:		House file suffix.
                                    0,                    // LEMON:		Lemon vehicle frequency.
                                    COLOR_NEUTRAL,        // COLOR:		Dark Radar map color.
                                    COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                    REMAP_LTBLUE,         // Remap color ID number.
                                    RemapLtBlue,          // Default remap table.
                                    'J'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMultiAdmin(HOUSE_ADMIN,
                                            "MultiAdmin",         //	NAME:			House name.
                                            TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                            "ADM",                // SUFFIX:		House file suffix.
                                            0,                    // LEMON:		Lemon vehicle frequency.
                                            COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                            COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                            REMAP_BLUE,           // Remap color ID number.
                                            RemapBlue,            // Default remap table.
                                            'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseSpectator(HOUSE_SPECTATOR,
                                           "Spectator",          //	NAME:			House name.
                                           TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                           "SPC",                // SUFFIX:		House file suffix.
                                           0,                    // LEMON:		Lemon vehicle frequency.
                                           COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                           COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                           REMAP_BLUE,           // Remap color ID number.
                                           RemapBlue,            // Default remap table.
                                           'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseTeam1(HOUSE_TEAM1,
                                       "Team 1",             //	NAME:			House name.
                                       TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                       "GDI",                // SUFFIX:		House file suffix.
                                       0,                    // LEMON:		Lemon vehicle frequency.
                                       COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                       COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                       REMAP_ORANGE,         // Remap color ID number.
                                       RemapOrange,          // Default remap table.
                                       'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseTeam2(HOUSE_TEAM2,
                                       "Team 2",             //	NAME:			House name.
                                       TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                       "GDI",                // SUFFIX:		House file suffix.
                                       0,                    // LEMON:		Lemon vehicle frequency.
                                       COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                       COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                       REMAP_GREEN,          // Remap color ID number.
                                       RemapGreen,           // Default remap table.
                                       'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseTeam3(HOUSE_TEAM3,
                                       "Team 3",             //	NAME:			House name.
                                       TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                       "GDI",                // SUFFIX:		House file suffix.
                                       0,                    // LEMON:		Lemon vehicle frequency.
                                       COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                       COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                       REMAP_LTBLUE,         // Remap color ID number.
                                       RemapLtBlue,          // Default remap table.
                                       'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseTeam4(HOUSE_TEAM4,
                                       "Team 4",             //	NAME:			House name.
                                       TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                       "GDI",                // SUFFIX:		House file suffix.
                                       0,                    // LEMON:		Lemon vehicle frequency.
                                       COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                       COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                       REMAP_LTBLUE,         // Remap color ID number.
                                       RemapLtBlue,          // Default remap table.
                                       'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti1(HOUSE_MULTI1,
                                        "Multi1",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP1",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_BLUE,           // Remap color ID number.
                                        RemapBlue,            // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti2(HOUSE_MULTI2,
                                        "Multi2",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP2",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_ORANGE,         // Remap color ID number.
                                        RemapOrange,          // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti3(HOUSE_MULTI3,
                                        "Multi3",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP3",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_GREEN,          // Remap color ID number.
                                        RemapGreen,           // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti4(HOUSE_MULTI4,
                                        "Multi4",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP4",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_LTBLUE,         // Remap color ID number.
                                        RemapLtBlue,          // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti5(HOUSE_MULTI5,
                                        "Multi5",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP5",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_GOLD,           // Remap color ID number.
                                        RemapGold,            // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti6(HOUSE_MULTI6,
                                        "Multi6",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP6",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti7(HOUSE_MULTI7,
                                        "Multi7",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP7",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti8(HOUSE_MULTI8,
                                        "Multi8",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP8",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti9(HOUSE_MULTI9,
                                        "Multi9",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP9",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti10(HOUSE_MULTI10,
                                        "Multi10",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP10",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti11(HOUSE_MULTI11,
                                        "Multi11",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP11",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti12(HOUSE_MULTI12,
                                        "Multi12",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP12",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti13(HOUSE_MULTI13,
                                        "Multi13",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP13",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti14(HOUSE_MULTI14,
                                        "Multi14",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP14",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti15(HOUSE_MULTI15,
                                        "Multi15",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP15",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti16(HOUSE_MULTI16,
                                        "Multi16",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP16",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti17(HOUSE_MULTI17,
                                        "Multi17",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP17",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti18(HOUSE_MULTI18,
                                        "Multi18",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP18",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti19(HOUSE_MULTI19,
                                        "Multi19",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP19",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti20(HOUSE_MULTI20,
                                        "Multi20",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP20",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti21(HOUSE_MULTI21,
                                        "Multi21",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP21",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti22(HOUSE_MULTI22,
                                        "Multi22",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP22",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti23(HOUSE_MULTI23,
                                        "Multi23",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP23",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti24(HOUSE_MULTI24,
                                        "Multi24",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP24",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti25(HOUSE_MULTI25,
                                        "Multi25",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP25",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti26(HOUSE_MULTI26,
                                        "Multi26",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP26",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti27(HOUSE_MULTI27,
                                        "Multi27",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP27",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti28(HOUSE_MULTI28,
                                        "Multi28",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP28",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti29(HOUSE_MULTI29,
                                        "Multi29",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP29",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti30(HOUSE_MULTI30,
                                        "Multi30",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP30",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti31(HOUSE_MULTI31,
                                        "Multi31",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP31",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti32(HOUSE_MULTI32,
                                        "Multi32",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP32",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti33(HOUSE_MULTI33,
                                        "Multi33",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP33",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti34(HOUSE_MULTI34,
                                        "Multi34",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP34",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti35(HOUSE_MULTI35,
                                        "Multi35",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP35",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti36(HOUSE_MULTI36,
                                        "Multi36",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP36",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti37(HOUSE_MULTI37,
                                        "Multi37",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP37",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti38(HOUSE_MULTI38,
                                        "Multi38",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP38",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti39(HOUSE_MULTI39,
                                        "Multi39",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP39",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti40(HOUSE_MULTI40,
                                        "Multi40",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP40",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti41(HOUSE_MULTI41,
                                        "Multi41",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP41",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti42(HOUSE_MULTI42,
                                        "Multi42",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP42",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti43(HOUSE_MULTI43,
                                        "Multi43",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP43",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti44(HOUSE_MULTI44,
                                        "Multi44",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP44",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti45(HOUSE_MULTI45,
                                        "Multi45",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP45",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti46(HOUSE_MULTI46,
                                        "Multi46",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP46",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti47(HOUSE_MULTI47,
                                        "Multi47",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP47",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti48(HOUSE_MULTI48,
                                        "Multi48",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP48",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti49(HOUSE_MULTI49,
                                        "Multi49",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP49",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti50(HOUSE_MULTI50,
                                        "Multi50",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP50",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti51(HOUSE_MULTI51,
                                        "Multi51",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP51",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti52(HOUSE_MULTI52,
                                        "Multi52",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP52",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti53(HOUSE_MULTI53,
                                        "Multi53",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP53",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti54(HOUSE_MULTI54,
                                        "Multi54",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP54",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti55(HOUSE_MULTI55,
                                        "Multi55",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP55",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti56(HOUSE_MULTI56,
                                        "Multi56",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP56",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti57(HOUSE_MULTI57,
                                        "Multi57",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP57",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti58(HOUSE_MULTI58,
                                        "Multi58",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP58",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti59(HOUSE_MULTI59,
                                        "Multi59",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP59",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti60(HOUSE_MULTI60,
                                        "Multi60",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP60",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti61(HOUSE_MULTI61,
                                        "Multi61",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP61",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti62(HOUSE_MULTI62,
                                        "Multi62",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP62",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti63(HOUSE_MULTI63,
                                        "Multi63",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP63",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti64(HOUSE_MULTI64,
                                        "Multi64",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP64",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti65(HOUSE_MULTI65,
                                        "Multi65",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP65",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti66(HOUSE_MULTI66,
                                        "Multi66",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP66",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti67(HOUSE_MULTI67,
                                        "Multi67",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP67",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti68(HOUSE_MULTI68,
                                        "Multi68",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP68",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti69(HOUSE_MULTI69,
                                        "Multi69",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP69",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti70(HOUSE_MULTI70,
                                        "Multi70",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP70",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti71(HOUSE_MULTI71,
                                        "Multi71",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP71",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti72(HOUSE_MULTI72,
                                        "Multi72",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP72",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti73(HOUSE_MULTI73,
                                        "Multi73",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP73",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti74(HOUSE_MULTI74,
                                        "Multi74",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP74",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti75(HOUSE_MULTI75,
                                        "Multi75",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP75",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti76(HOUSE_MULTI76,
                                        "Multi76",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP76",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti77(HOUSE_MULTI77,
                                        "Multi77",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP77",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti78(HOUSE_MULTI78,
                                        "Multi78",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP78",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti79(HOUSE_MULTI79,
                                        "Multi79",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP79",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti80(HOUSE_MULTI80,
                                        "Multi80",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP80",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti81(HOUSE_MULTI81,
                                        "Multi81",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP81",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti82(HOUSE_MULTI82,
                                        "Multi82",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP82",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti83(HOUSE_MULTI83,
                                        "Multi83",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP83",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti84(HOUSE_MULTI84,
                                        "Multi84",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP84",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti85(HOUSE_MULTI85,
                                        "Multi85",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP85",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti86(HOUSE_MULTI86,
                                        "Multi86",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP86",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti87(HOUSE_MULTI87,
                                        "Multi87",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP87",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti88(HOUSE_MULTI88,
                                        "Multi88",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP88",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti89(HOUSE_MULTI89,
                                        "Multi89",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP89",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti90(HOUSE_MULTI90,
                                        "Multi90",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP90",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti91(HOUSE_MULTI91,
                                        "Multi91",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP91",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti92(HOUSE_MULTI92,
                                        "Multi92",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP92",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti93(HOUSE_MULTI93,
                                        "Multi93",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP93",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti94(HOUSE_MULTI94,
                                        "Multi94",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP94",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti95(HOUSE_MULTI95,
                                        "Multi95",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP95",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti96(HOUSE_MULTI96,
                                        "Multi96",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP96",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti97(HOUSE_MULTI97,
                                        "Multi97",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP97",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti98(HOUSE_MULTI98,
                                        "Multi98",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP98",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti99(HOUSE_MULTI99,
                                        "Multi99",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP99",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti100(HOUSE_MULTI100,
                                        "Multi100",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP100",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_RED,            // Remap color ID number.
                                        RemapRed,             // Default remap table.
                                        'M'                   // VOICE:		Voice prefix character.
);

HouseTypeClass const* const HouseTypeClass::Pointers[HOUSE_COUNT] = {
    &HouseGood,
    &HouseBad,
    &HouseCivilian,
    &HouseJP,
    &HouseMultiAdmin,
    &HouseSpectator,
    &HouseTeam1,
    &HouseTeam2,
    &HouseTeam3,
    &HouseTeam4,
    &HouseMulti1,
    &HouseMulti2,
    &HouseMulti3,
    &HouseMulti4,
    &HouseMulti5,
    &HouseMulti6,
    &HouseMulti7,
    &HouseMulti8,
    &HouseMulti9,
    &HouseMulti10,
    &HouseMulti11,
    &HouseMulti12,
    &HouseMulti13,
    &HouseMulti14,
    &HouseMulti15,
    &HouseMulti16,
    &HouseMulti17,
    &HouseMulti18,
    &HouseMulti19,
    &HouseMulti20,
    &HouseMulti21,
    &HouseMulti22,
    &HouseMulti23,
    &HouseMulti24,
    &HouseMulti25,
    &HouseMulti26,
    &HouseMulti27,
    &HouseMulti28,
    &HouseMulti29,
    &HouseMulti30,
    &HouseMulti31,
    &HouseMulti32,
    &HouseMulti33,
    &HouseMulti34,
    &HouseMulti35,
    &HouseMulti36,
    &HouseMulti37,
    &HouseMulti38,
    &HouseMulti39,
    &HouseMulti40,
    &HouseMulti41,
    &HouseMulti42,
    &HouseMulti43,
    &HouseMulti44,
    &HouseMulti45,
    &HouseMulti46,
    &HouseMulti47,
    &HouseMulti48,
    &HouseMulti49,
    &HouseMulti50,
    &HouseMulti51,
    &HouseMulti52,
    &HouseMulti53,
    &HouseMulti54,
    &HouseMulti55,
    &HouseMulti56,
    &HouseMulti57,
    &HouseMulti58,
    &HouseMulti59,
    &HouseMulti60,
    &HouseMulti61,
    &HouseMulti62,
    &HouseMulti63,
    &HouseMulti64,
    &HouseMulti65,
    &HouseMulti66,
    &HouseMulti67,
    &HouseMulti68,
    &HouseMulti69,
    &HouseMulti70,
    &HouseMulti71,
    &HouseMulti72,
    &HouseMulti73,
    &HouseMulti74,
    &HouseMulti75,
    &HouseMulti76,
    &HouseMulti77,
    &HouseMulti78,
    &HouseMulti79,
    &HouseMulti80,
    &HouseMulti81,
    &HouseMulti82,
    &HouseMulti83,
    &HouseMulti84,
    &HouseMulti85,
    &HouseMulti86,
    &HouseMulti87,
    &HouseMulti88,
    &HouseMulti89,
    &HouseMulti90,
    &HouseMulti91,
    &HouseMulti92,
    &HouseMulti93,
    &HouseMulti94,
    &HouseMulti95,
    &HouseMulti96,
    &HouseMulti97,
    &HouseMulti98,
    &HouseMulti99,
    &HouseMulti100,
};

/***********************************************************************************************
 * HouseTypeClass::HouseTypeClass -- Constructor for house type objects.                       *
 *                                                                                             *
 *    This is the constructor for house type objects. This object holds the constant data      *
 *    for the house type.                                                                      *
 *                                                                                             *
 * INPUT:   house    -- The ID number for this house type.                                     *
 *          ini      -- The INI name of this house.                                            *
 *          fullname -- The text number representing the complete name of the house.           *
 *          ext      -- The filename extension used when loading data files.                   *
 *          lemon    -- The percentage for objects of this ownership to be lemon.              *
 *          remapc   -- The remap color number to use.                                         *
 *          color    -- The radar color to use for this "house".                               *
 *          prefix   -- A unique prefix letter used when building custom filenames.            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/21/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
HouseTypeClass::HouseTypeClass(HousesType house,
                               char const* ini,
                               int fullname,
                               char const* ext,
                               int lemon,
                               int color,
                               int bright_color,
                               PlayerColorType remapcolor,
                               unsigned char const* remap,
                               char prefix)
{
    RemapTable = remap;
    RemapColor = remapcolor;
    House = house;
    IniName = ini;
    FullName = fullname;
    strncpy(Suffix, ext, 3);
    Suffix[3] = '\0';
    Lemon = lemon;
    Color = color;
    BrightColor = bright_color;
    Prefix = prefix;
    FirepowerBias = 1;
    GroundspeedBias = 1;
    AirspeedBias = 1;
    ArmorBias = 1;
    ROFBias = 1;
    CostBias = 1;
    BuildSpeedBias = 1;
}

/***********************************************************************************************
 * HouseTypeClass::From_Name -- Fetch house pointer from its name.                             *
 *                                                                                             *
 *    This routine will convert the ASCII house name specified into a                          *
 *    real house number. Typically, this is used when processing a                             *
 *    scenario INI file.                                                                       *
 *                                                                                             *
 * INPUT:   name  -- ASCII name of house to process.                                           *
 *                                                                                             *
 * OUTPUT:  Returns with actual house number represented by the ASCII                          *
 *          name specified.                                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *   05/21/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
HousesType HouseTypeClass::From_Name(char const* name)
{
    if (name) {
        for (HousesType house = HOUSE_FIRST; house < HOUSE_COUNT; house++) {
            if (stricmp(Pointers[house]->IniName, name) == 0) {
                return (house);
            }
        }
    }
    return (HOUSE_NONE);
}

/***********************************************************************************************
 * HouseTypeClass::One_Time -- One-time initialization                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/21/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
void HouseTypeClass::One_Time(void)
{
    /*
    ** Change the radar color for special units; otherwise, they'll be the same
    ** color as the player!
    */
    // if (Special.IsJurassic && AreThingiesEnabled) {			// Assume funpark mode might be required. ST - 10/14/2019
    // 11:53AM
    ((unsigned char&)HouseJP.Color) = (unsigned char)COLOR_BAD;
    ((unsigned char&)HouseJP.BrightColor) = (unsigned char)COLOR_BRIGHT_BAD;
    //}
}

/***********************************************************************************************
 * HouseTypeClass::As_Reference -- Fetches a reference to the house specified.                 *
 *                                                                                             *
 *    Use this routine to fetch a reference to the house number specified.                     *
 *                                                                                             *
 * INPUT:   house -- The house number (HousesType) to look up.                                 *
 *                                                                                             *
 * OUTPUT:  Returns with a reference to the HouseTypeClass object that matches the house       *
 *          number specified.                                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
HouseTypeClass const& HouseTypeClass::As_Reference(HousesType house)
{
    return (*Pointers[house]);
}