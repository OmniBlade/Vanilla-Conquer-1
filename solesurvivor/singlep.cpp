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

// Matching
ObjectClass* Create_Offline_Choice(void)
{
    ObjectClass* retval = NULL;
    TechnoClass* techno;
    PlayerPtr = HouseClass::As_Pointer(HOUSE_MULTI1);
    PlayerPtr->ActLike = HOUSE_GOOD;

    if (Chosen_RTTI == RTTI_UNIT) {
        retval = new UnitClass((UnitType)Chosen_Type, HOUSE_MULTI1);
    } else {
        retval = new InfantryClass((InfantryType)Chosen_Type, HOUSE_MULTI1);
    }

    techno = (TechnoClass*)retval;
    HouseClass* hptr = HouseClass::As_Pointer(HOUSE_MULTI1);
    hptr->IsHuman = true;

    if (Frame == 1) {
        OfflinePoints = 0;
        OfflineDeathCount = 0;
    } else {
        for (int index = 0; index < 16384; index++) {
            CellClass* cell = &Map[index];
            if (cell->IsMapped || cell->IsVisible) {
                cell->Redraw_Objects();
                cell->IsMapped = false;
                cell->IsVisible = false;
            }
        }

        DoFullRedraw = true;
        Client_Handle_Sight();
    }

    hptr->Int2 = 0;
    hptr->Int3 = 0;
    hptr->Int4 = -1;

    sprintf(hptr->Name, MPlayerName);

    Map.Activate(true);

    Map.Color_List_Add_Player(hptr->Name, HOUSE_MULTI1);
    Map.Color_List_Update_Points(hptr->Name, hptr->Int2);

    techno->IsOwnedByPlayer = true;

    techno->Revealed(PlayerPtr);
    techno->Look(false);

    AllowVoice = false;
    techno->Select();
    AllowVoice = true;

    PlayerPtr->IsDefeated = false;

    ++ScenarioInit;
    int start_x = 0;
    int start_y = 0;
    Map.Compute_Start_Pos(start_x, start_y);
    for (int i = 0; i < ARRAY_SIZE(Scen.Views); ++i) {
        Scen.Views[i] = XY_Cell(start_x, start_y);
    }
    Scen.Waypoint[27] = XY_Cell(start_x, start_y);
    COORDINATE pos = Cell_Coord(XY_Cell(start_x, start_y));
    Map.Set_Tactical_Position(pos);
    --ScenarioInit;

    Map.Flag_To_Redraw(true);

    return retval;
}

// Matching
void Create_Offline_Unit(void)
{
    CELL cell;
    ObjectClass* object = 0;
    if (UnitClass::New_Allowed()) {
        object = Create_Offline_Choice();
    }

    if (object != NULL) {
        cell = XY_Cell(WDT_Random_Pick(0, Map.MapCellWidth - 2) + Map.MapCellX,
                       WDT_Random_Pick(0, Map.MapCellHeight - 1) + Map.MapCellY);
        if (Scan_Place_Object(object, cell) == 0) {
            delete object;
        } else {
            ++ScenarioInit;
            int start_x = 0;
            int start_y = 0;
            Map.Compute_Start_Pos(start_x, start_y);
            for (int i = 0; i < ARRAY_SIZE(Scen.Views); ++i) {
                Scen.Views[i] = XY_Cell(start_x, start_y);
            }
            Scen.Waypoint[27] = XY_Cell(start_x, start_y);
            COORDINATE pos = Cell_Coord(XY_Cell(start_x, start_y));
            Map.Set_Tactical_Position(pos);
            --ScenarioInit;

            Map.Flag_To_Redraw(true);
            TechnoClass* techno = (TechnoClass*)object;
            techno->Revealed(PlayerPtr);
            techno->Look(false);
        }
    }
}
