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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : WINSTUB.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Steve Tall                                                   *
 *                                                                                             *
 *                   Start Date : 05/29/1996                                                   *
 *                                                                                             *
 *                  Last Update : May 29th 1996 [ST]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Overview:                                                                                   *
 *  Internet game statistics to collect and upload to the server                               *
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"

#define FIELD_PACKET_TYPE      "TYPE"
#define FIELD_GAME_ID          "IDNO"
#define FIELD_START_CREDITS    "CRED"
#define FIELD_BASES            "BASE"
#define FIELD_TIBERIUM         "TIBR"
#define FIELD_CRATES           "CRAT"
#define FIELD_AI_PLAYERS       "AIPL"
#define FIELD_CAPTURE_THE_FLAG "FLAG"
#define FIELD_START_UNIT_COUNT "UNIT"
#define FIELD_TECH_LEVEL       "TECH"
#define FIELD_SCENARIO         "SCEN"
#define FIELD_COMPLETION       "CMPL"
#define FIELD_START_TIME       "TIME"
#define FIELD_GAME_DURATION    "DURA"
#define FIELD_FRAME_RATE       "AFPS"
#define FIELD_SPEED_SETTING    "SPED"
#define FIELD_GAME_VERSION     "VERS"
#define FIELD_GAME_BUILD_DATE  "DATE"
#define FIELD_COVERT_PRESENT   "COVT"
#define FIELD_CPU_TYPE         "PROC"
#define FIELD_MEMORY           "MEMO"
#define FIELD_VIDEO_MEMORY     "VIDM"
#define FIELD_PLAYER1_HANDLE   "NAM1"
#define FIELD_PLAYER2_HANDLE   "NAM2"
#define FIELD_PLAYER1_TEAM     "SID1"
#define FIELD_PLAYER2_TEAM     "SID2"
#define FIELD_PLAYER1_COLOR    "COL1"
#define FIELD_PLAYER2_COLOR    "COL2"
#define FIELD_PLAYER1_CREDITS  "CRD1"
#define FIELD_PLAYER2_CREDITS  "CRD2"

#define FIELD_PLAYER1_UNITS_LEFT     "UNL1"
#define FIELD_PLAYER2_UNITS_LEFT     "UNL2"
#define FIELD_PLAYER1_INFANTRY_LEFT  "INL1"
#define FIELD_PLAYER2_INFANTRY_LEFT  "INL2"
#define FIELD_PLAYER1_PLANES_LEFT    "PLL1"
#define FIELD_PLAYER2_PLANES_LEFT    "PLL2"
#define FIELD_PLAYER1_BUILDINGS_LEFT "BLL1"
#define FIELD_PLAYER2_BUILDINGS_LEFT "BLL2"

#define FIELD_PLAYER1_UNITS_BOUGHT     "UNB1"
#define FIELD_PLAYER2_UNITS_BOUGHT     "UNB2"
#define FIELD_PLAYER1_INFANTRY_BOUGHT  "INB1"
#define FIELD_PLAYER2_INFANTRY_BOUGHT  "INB2"
#define FIELD_PLAYER1_PLANES_BOUGHT    "PLB1"
#define FIELD_PLAYER2_PLANES_BOUGHT    "PLB2"
#define FIELD_PLAYER1_BUILDINGS_BOUGHT "BLB1"
#define FIELD_PLAYER2_BUILDINGS_BOUGHT "BLB2"

#define FIELD_PLAYER1_UNITS_KILLED     "UNK1"
#define FIELD_PLAYER2_UNITS_KILLED     "UNK2"
#define FIELD_PLAYER1_INFANTRY_KILLED  "INK1"
#define FIELD_PLAYER2_INFANTRY_KILLED  "INK2"
#define FIELD_PLAYER1_PLANES_KILLED    "PLK1"
#define FIELD_PLAYER2_PLANES_KILLED    "PLK2"
#define FIELD_PLAYER1_BUILDINGS_KILLED "BLK1"
#define FIELD_PLAYER2_BUILDINGS_KILLED "BLK2"

#define FIELD_PLAYER1_BUILDINGS_CAPTURED "BLC1"
#define FIELD_PLAYER2_BUILDINGS_CAPTURED "BLC2"

#define FIELD_PLAYER1_CRATES_FOUND "CRA1"
#define FIELD_PLAYER2_CRATES_FOUND "CRA2"
#define FIELD_PLAYER1_HARVESTED    "HRV1"
#define FIELD_PLAYER2_HARVESTED    "HRV2"

#define PACKET_TYPE_HOST_GAME_INFO  (unsigned char)50
#define PACKET_TYPE_GUEST_GAME_INFO (unsigned char)51

enum
{
    COMPLETION_CONNECTION_LOST,
    COMPLETION_PLAYER_1_WON,
    COMPLETION_PLAYER_1_WON_BY_RESIGNATION,
    COMPLETION_PLAYER_1_WON_BY_DISCONNECTION,
    COMPLETION_PLAYER_2_WON,
    COMPLETION_PLAYER_2_WON_BY_RESIGNATION,
    COMPLETION_PLAYER_2_WON_BY_DISCONNECTION
};

extern unsigned int PlanetWestwoodGameID;
#ifdef _WIN32
extern HINSTANCE ProgramInstance;
#endif
extern unsigned int PlanetWestwoodStartTime;

char CPUType;

bool GameTimerInUse = false;
TimerClass GameTimer;
