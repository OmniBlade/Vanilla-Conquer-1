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
#ifndef SOLEWDT_H
#define SOLEWDT_H

#include "function.h"

enum GetWhatEnum : int8_t
{
    GET_STRENGTH,
    GET_DAMAGE,
    GET_SPEED,
    GET_ROF,
    GET_RANGE,
};

int Unit_Choice_Dialog(bool names = false);
void Add_WDT_Radar();
void Remove_WDT_Radar();
void Clear_Packet_Vectors();
void Host_Disconnect();
int Process_Crate_Pickup(int8_t crate_type, CELL cell, UnitClass* unit);
int Get_Stat(GetWhatEnum what, int initial_val, ObjectClass* obj);
int Calc_Object_Stats(ObjectClass* obj);

#endif
