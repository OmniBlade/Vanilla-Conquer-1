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
#include "defines.h"
#include "externs.h"

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
                                          REMAP_GOLD,           // Remap color ID number.
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
                                    REMAP_BLUE,           // Remap color ID number.
                                    RemapBlue,            // Default remap table.
                                    'J'                   // VOICE:		Voice prefix character.
);
static HouseTypeClass const HouseAdmin(HOUSE_ADMIN,
                                       "Admin",              //	NAME:			House name.
                                       TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                       "ADM",                // SUFFIX:		House file suffix.
                                       0,                    // LEMON:		Lemon vehicle frequency.
                                       COLOR_NEUTRAL,        // COLOR:		Dark Radar map color.
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
                                           COLOR_NEUTRAL,        // COLOR:		Dark Radar map color.
                                           COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                           REMAP_LTBLUE,           // Remap color ID number.
                                           RemapLtBlue,       // Default remap table.
                                           'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseTeam1(HOUSE_BLUE_TEAM,
                                       "Team 1",             //	NAME:			House name.
                                       TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                       "GDI",                // SUFFIX:		House file suffix.
                                       0,                    // LEMON:		Lemon vehicle frequency.
                                       COLOR_NEUTRAL,        // COLOR:		Dark Radar map color.
                                       COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                       REMAP_LTBLUE,           // Remap color ID number.
                                       RemapLtBlue,       // Default remap table.
                                       'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseTeam2(HOUSE_ORANGE_TEAM,
                                       "Team 2",             //	NAME:			House name.
                                       TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                       "GDI",                // SUFFIX:		House file suffix.
                                       0,                    // LEMON:		Lemon vehicle frequency.
                                       COLOR_NEUTRAL,        // COLOR:		Dark Radar map color.
                                       COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                       REMAP_ORANGE,         // Remap color ID number.
                                       RemapOrange,          // Default remap table.
                                       'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseTeam3(HOUSE_GREEN_TEAM,
                                       "Team 3",             //	NAME:			House name.
                                       TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                       "GDI",                // SUFFIX:		House file suffix.
                                       0,                    // LEMON:		Lemon vehicle frequency.
                                       COLOR_NEUTRAL,        // COLOR:		Dark Radar map color.
                                       COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                       REMAP_GREEN,          // Remap color ID number.
                                       RemapGreen,           // Default remap table.
                                       'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseTeam4(HOUSE_GREY_TEAM,
                                       "Team 4",             //	NAME:			House name.
                                       TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                       "GDI",                // SUFFIX:		House file suffix.
                                       0,                    // LEMON:		Lemon vehicle frequency.
                                       COLOR_NEUTRAL,        // COLOR:		Dark Radar map color.
                                       COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                       REMAP_BLUE,           // Remap color ID number.
                                       RemapBlue,            // Default remap table.
                                       'M'                   // VOICE:		Voice prefix character.
);

static HouseTypeClass const HouseMulti1(HOUSE_MULTI1,
                                        "Multi1",             //	NAME:			House name.
                                        TXT_CIVILIAN,         // FULLNAME:	Translated house name.
                                        "MP1",                // SUFFIX:		House file suffix.
                                        0,                    // LEMON:		Lemon vehicle frequency.
                                        COLOR_NEUTRAL,        // COLOR:		Radar map color.
                                        COLOR_BRIGHT_NEUTRAL, // COLOR:		Bright Radar map color.
                                        REMAP_LTBLUE,         // Remap color ID number.
                                        RemapLtBlue,          // Default remap table.
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
                                        REMAP_BLUE,           // Remap color ID number.
                                        RemapBlue,            // Default remap table.
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

#define DEFINE_MP_HOUSE(num)                                                                                           \
    static HouseTypeClass const HouseMulti##num(HOUSE_MULTI##num,                                                      \
                                                "Multi" #num,                                                          \
                                                TXT_CIVILIAN,                                                          \
                                                "MP" #num,                                                             \
                                                0,                                                                     \
                                                COLOR_NEUTRAL,                                                         \
                                                COLOR_BRIGHT_NEUTRAL,                                                  \
                                                REMAP_RED,                                                             \
                                                RemapRed,                                                              \
                                                'M');

DEFINE_MP_HOUSE(7);
DEFINE_MP_HOUSE(8);
DEFINE_MP_HOUSE(9);
DEFINE_MP_HOUSE(10);
DEFINE_MP_HOUSE(11);
DEFINE_MP_HOUSE(12);
DEFINE_MP_HOUSE(13);
DEFINE_MP_HOUSE(14);
DEFINE_MP_HOUSE(15);
DEFINE_MP_HOUSE(16);
DEFINE_MP_HOUSE(17);
DEFINE_MP_HOUSE(18);
DEFINE_MP_HOUSE(19);
DEFINE_MP_HOUSE(20);
DEFINE_MP_HOUSE(21);
DEFINE_MP_HOUSE(22);
DEFINE_MP_HOUSE(23);
DEFINE_MP_HOUSE(24);
DEFINE_MP_HOUSE(25);
DEFINE_MP_HOUSE(26);
DEFINE_MP_HOUSE(27);
DEFINE_MP_HOUSE(28);
DEFINE_MP_HOUSE(29);
DEFINE_MP_HOUSE(30);
DEFINE_MP_HOUSE(31);
DEFINE_MP_HOUSE(32);
DEFINE_MP_HOUSE(33);
DEFINE_MP_HOUSE(34);
DEFINE_MP_HOUSE(35);
DEFINE_MP_HOUSE(36);
DEFINE_MP_HOUSE(37);
DEFINE_MP_HOUSE(38);
DEFINE_MP_HOUSE(39);
DEFINE_MP_HOUSE(40);
DEFINE_MP_HOUSE(41);
DEFINE_MP_HOUSE(42);
DEFINE_MP_HOUSE(43);
DEFINE_MP_HOUSE(44);
DEFINE_MP_HOUSE(45);
DEFINE_MP_HOUSE(46);
DEFINE_MP_HOUSE(47);
DEFINE_MP_HOUSE(48);
DEFINE_MP_HOUSE(49);
DEFINE_MP_HOUSE(50);
DEFINE_MP_HOUSE(51);
DEFINE_MP_HOUSE(52);
DEFINE_MP_HOUSE(53);
DEFINE_MP_HOUSE(54);
DEFINE_MP_HOUSE(55);
DEFINE_MP_HOUSE(56);
DEFINE_MP_HOUSE(57);
DEFINE_MP_HOUSE(58);
DEFINE_MP_HOUSE(59);
DEFINE_MP_HOUSE(60);
DEFINE_MP_HOUSE(61);
DEFINE_MP_HOUSE(62);
DEFINE_MP_HOUSE(63);
DEFINE_MP_HOUSE(64);
DEFINE_MP_HOUSE(65);
DEFINE_MP_HOUSE(66);
DEFINE_MP_HOUSE(67);
DEFINE_MP_HOUSE(68);
DEFINE_MP_HOUSE(69);
DEFINE_MP_HOUSE(70);
DEFINE_MP_HOUSE(71);
DEFINE_MP_HOUSE(72);
DEFINE_MP_HOUSE(73);
DEFINE_MP_HOUSE(74);
DEFINE_MP_HOUSE(75);
DEFINE_MP_HOUSE(76);
DEFINE_MP_HOUSE(77);
DEFINE_MP_HOUSE(78);
DEFINE_MP_HOUSE(79);
DEFINE_MP_HOUSE(80);
DEFINE_MP_HOUSE(81);
DEFINE_MP_HOUSE(82);
DEFINE_MP_HOUSE(83);
DEFINE_MP_HOUSE(84);
DEFINE_MP_HOUSE(85);
DEFINE_MP_HOUSE(86);
DEFINE_MP_HOUSE(87);
DEFINE_MP_HOUSE(88);
DEFINE_MP_HOUSE(89);
DEFINE_MP_HOUSE(90);
DEFINE_MP_HOUSE(91);
DEFINE_MP_HOUSE(92);
DEFINE_MP_HOUSE(93);
DEFINE_MP_HOUSE(94);
DEFINE_MP_HOUSE(95);
DEFINE_MP_HOUSE(96);
DEFINE_MP_HOUSE(97);
DEFINE_MP_HOUSE(98);
DEFINE_MP_HOUSE(99);
DEFINE_MP_HOUSE(100);

HouseTypeClass const* const HouseTypeClass::Pointers[HOUSE_COUNT] = {
    &HouseGood,
    &HouseBad,
    &HouseCivilian,
    &HouseJP,
	&HouseAdmin,
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
    if (Special.IsJurassic && GameToPlay == GAME_NORMAL) {
    ((unsigned char&)HouseJP.Color) = (unsigned char)COLOR_BAD;
    ((unsigned char&)HouseJP.BrightColor) = (unsigned char)COLOR_BRIGHT_BAD;
    }
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