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

/* $Header: /CounterStrike/IOMAP.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : IOMAP.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Bill Randolph                                                *
 *                                                                                             *
 *                   Start Date : January 16, 1995                                             *
 *                                                                                             *
 *                  Last Update : March 12, 1996 [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * All map-related loading/saving routines should go in this module, so it can be overlayed.   *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CellClass::Code_Pointers -- codes class's pointers for load/save                          *
 *   CellClass::Decode_Pointers -- decodes pointers for load/save                              *
 *   CellClass::Load -- Reads from a save game file.                                           *
 *   CellClass::Save -- Write to a save game file.                                             *
 *   CellClass::Should_Save -- Should the cell be written to disk?                             *
 *   DisplayClass::Code_Pointers -- codes class's pointers for load/save                       *
 *   DisplayClass::Decode_Pointers -- decodes pointers for load/save                           *
 *   MapClass::Code_Pointers -- codes class's pointers for load/save                           *
 *   MapClass::Decode_Pointers -- decodes pointers for load/save                               *
 *   MouseClass::Load -- Loads from a save game file.                                          *
 *   MouseClass::Save -- Saves to a save game file.                                            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"

/***********************************************************************************************
 * CellClass::Should_Save -- Should the cell be written to disk?                               *
 *                                                                                             *
 *    This function will determine if the cell needs to be written to disk. Any cell that      *
 *    contains special data should be written to disk.                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Should this cell's data be written to disk?                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CellClass::Should_Save(void) const
{
    static CellClass const _identity_cell;

    return (memcmp(&_identity_cell, this, sizeof(*this)) != 0);
}

/***********************************************************************************************
 * CellClass::Load -- Loads from a save game file.                                             *
 *                                                                                             *
 * INPUT:   file  -- The file to read the cell's data from.                                    *
 *                                                                                             *
 * OUTPUT:  true = success, false = failure                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CellClass::Load(Straw& file)
{
    bool tmp;
    int32_t tmpi32;
    int8_t tmpi8;
    uint16_t tmpu16;

    file.Get(ID);
    
    file.Get(tmp);
    IsPlot = tmp;
    file.Get(tmp);
    IsCursorHere = tmp;
    file.Get(tmp);
    IsMapped = tmp;
    file.Get(tmp);
    IsVisible = tmp;
    file.Get(tmp);
    IsWaypoint = tmp;
    file.Get(tmp);
    IsRadarCursor = tmp;
    file.Get(tmp);
    IsFlagged = tmp;
    file.Get(tmp);
    IsToShroud = tmp;

    for (int i = 0; i < MZONE_COUNT; ++i) {
        file.Get(Zones[i]);
    }

    file.Get(Jammed);
    
    file.Get(tmpi32);
    Trigger.Set_Raw(tmpi32);

    file.Get(tmpu16);
    TType = static_cast<TemplateType>(tmpu16);
    file.Get(TIcon);

    file.Get(tmpi8);
    Overlay = static_cast<OverlayType>(tmpi8);
    file.Get(OverlayData);

    file.Get(tmpi8);
    Smudge = static_cast<SmudgeType>(tmpi8);
    file.Get(SmudgeData);

    file.Get(tmpi8);
    Owner = static_cast<HousesType>(tmpi8);

    file.Get(tmpi8);
    InfType = static_cast<HousesType>(tmpi8);

    file.Get(tmpi32);
    OccupierPtr = reinterpret_cast<ObjectClass*>(static_cast<intptr_t>(tmpi32));

    static_assert(ARRAY_SIZE(Overlapper) == 6, "Overlapper array is not expected size for save format.");
    for (int i = 0; i < ARRAY_SIZE(Overlapper); ++i) {
        file.Get(tmpi32);
        Overlapper[i] = reinterpret_cast<ObjectClass*>(static_cast<intptr_t>(tmpi32));
    }

    file.Get(IsMappedByPlayerMask);
    file.Get(IsVisibleByPlayerMask);

    file.Get(tmp);
    Flag.Occupy.Center = tmp;
    file.Get(tmp);
    Flag.Occupy.NW = tmp;
    file.Get(tmp);
    Flag.Occupy.NE = tmp;
    file.Get(tmp);
    Flag.Occupy.SW = tmp;
    file.Get(tmp);
    Flag.Occupy.SE = tmp;
    file.Get(tmp);
    Flag.Occupy.Vehicle = tmp;
    file.Get(tmp);
    Flag.Occupy.Monolith = tmp;
    file.Get(tmp);
    Flag.Occupy.Building = tmp;

    file.Get(tmpi8);
    Land = static_cast<LandType>(tmpi8);
    file.Get(tmpi8);
    OverrideLand = static_cast<LandType>(tmpi8);

    return (true);
}

/***********************************************************************************************
 * CellClass::Save -- Write to a save game file.                                               *
 *                                                                                             *
 * INPUT:   file  -- The file to write the cell's data to.                                     *
 *                                                                                             *
 * OUTPUT:  true = success, false = failure                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool CellClass::Save(Pipe& file) const
{
    bool tmp;
    int32_t tmpi32;
    int8_t tmpi8;
    uint16_t tmpu16;

    file.Put(ID);

    tmp = IsPlot;
    file.Put(tmp);
    tmp = IsCursorHere;
    file.Put(tmp);
    tmp = IsMapped;
    file.Put(tmp);
    tmp = IsVisible;
    file.Put(tmp);
    tmp = IsWaypoint;
    file.Put(tmp);
    tmp = IsRadarCursor;
    file.Put(tmp);
    tmp = IsFlagged;
    file.Put(tmp);
    tmp = IsToShroud;
    file.Put(tmp);

    for (int i = 0; i < MZONE_COUNT; ++i) {
        file.Put(Zones[i]);
    }

    file.Put(Jammed);

    tmpi32 = Trigger.Raw();
    file.Put(tmpi32);

    tmpu16 = TType;
    file.Put(tmpu16);
    file.Put(TIcon);

    tmpi8 = Overlay;
    file.Put(tmpi8);
    file.Put(OverlayData);

    tmpi8 = Smudge;
    file.Put(tmpi8);
    file.Put(SmudgeData);

    tmpi8 = Owner;
    file.Put(tmpi8);

    tmpi8 = InfType;
    file.Put(tmpi8);

    tmpi32 = static_cast<int32_t>(reinterpret_cast<intptr_t>(OccupierPtr));
    file.Put(tmpi32);

    for (int i = 0; i < ARRAY_SIZE(Overlapper); ++i) {
        tmpi32 = static_cast<int32_t>(reinterpret_cast<intptr_t>(Overlapper[i]));
        file.Put(tmpi32);
    }

    file.Put(IsMappedByPlayerMask);
    file.Put(IsVisibleByPlayerMask);

    tmp = Flag.Occupy.Center;
    file.Put(tmp);
    tmp = Flag.Occupy.NW;
    file.Put(tmp);
    tmp = Flag.Occupy.NE;
    file.Put(tmp);
    tmp = Flag.Occupy.SW;
    file.Put(tmp);
    tmp = Flag.Occupy.SE;
    file.Put(tmp);
    tmp = Flag.Occupy.Vehicle;
    file.Put(tmp);
    tmp = Flag.Occupy.Monolith;
    file.Put(tmp);
    tmp = Flag.Occupy.Building;
    file.Put(tmp);

    tmpi8 = Land;
    file.Put(tmpi8);

    tmpi8 = OverrideLand;
    file.Put(tmpi8);

    // value of CTFFlag is wiped out in Code/Decode pointers function.

    return (true);
}

/***********************************************************************************************
 * CellClass::Code_Pointers -- codes class's pointers for load/save                            *
 *                                                                                             *
 * This routine "codes" the pointers in the class by converting them to a number               *
 * that still represents the object pointed to, but isn't actually a pointer.  This            *
 * allows a saved game to properly load without relying on the games data still                *
 * being in the exact same location.                                                           *
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
 *   01/02/1995 BR : Created.                                                                  *
 *=============================================================================================*/
void CellClass::Code_Pointers(void)
{
    if (Cell_Occupier() != NULL) {
        OccupierPtr = (ObjectClass*)(intptr_t)OccupierPtr->As_Target();
    }

    for (int index = 0; index < ARRAY_SIZE(Overlapper); index++) {
        if (Overlapper[index] != NULL && Overlapper[index]->IsActive) {
            Overlapper[index] = (ObjectClass*)(intptr_t)Overlapper[index]->As_Target();
        } else {
            Overlapper[index] = NULL;
        }
    }

    assert(CTFFlag == NULL);
}

/***********************************************************************************************
 * CellClass::Decode_Pointers -- decodes pointers for load/save                                *
 *                                                                                             *
 * This routine "decodes" the pointers coded in Code_Pointers by converting the                *
 * code values back into object pointers.                                                      *
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
 *   01/02/1995 BR : Created.                                                                  *
 *   03/12/1996 JLB : Simplified.                                                              *
 *=============================================================================================*/
void CellClass::Decode_Pointers(void)
{
    if (OccupierPtr != NULL) {
        OccupierPtr = As_Object(TARGET_SAFE_CAST(OccupierPtr), false);
        assert(OccupierPtr != NULL);
    }

    for (int index = 0; index < ARRAY_SIZE(Overlapper); index++) {
        if (Overlapper[index] != NULL) {
            Overlapper[index] = As_Object(TARGET_SAFE_CAST(Overlapper[index]), false);
            assert(Overlapper[index] != NULL);
        }
    }

    CTFFlag = NULL;
}

// Each layer now saves and loads itself.
bool GScreenClass::Load(Straw &file)
{
    bool tmp;

    file.Get(tmp);
    IsToRedraw = tmp;
    file.Get(tmp);
    IsToUpdate = tmp;

    return true;
}

bool GScreenClass::Save(Pipe &file) const
{
    bool tmp;

    tmp = IsToRedraw;
    file.Put(tmp);
    tmp = IsToUpdate;
    file.Put(tmp);

    return true;
}

bool MapClass::Load(Straw &file)
{
    GScreenClass::Load(file);
    
    file.Get(MapCellX);
    file.Get(MapCellY);
    file.Get(MapCellWidth);
    file.Get(MapCellHeight);
    file.Get(TotalValue);

    // Don't worry about the map array, its saved/loaded at top level after rest of class.

    file.Get(XSize);
    file.Get(YSize);
    file.Get(Size);

    for (int crate = 0; crate > ARRAY_SIZE(Crates); ++crate)
    {
        Crates[crate].Load(file);
    }
    return true;
}

bool MapClass::Save(Pipe &file) const
{
    GScreenClass::Save(file);
    file.Put(MapCellX);
    file.Put(MapCellY);
    file.Put(MapCellWidth);
    file.Put(MapCellHeight);
    file.Put(TotalValue);

    // Don't worry about the map array, its saved/loaded at top level after rest of class.

    file.Put(XSize);
    file.Put(YSize);
    file.Put(Size);

    for (int crate = 0; crate > ARRAY_SIZE(Crates); ++crate)
    {
        Crates[crate].Save(file);
    }
    return true;
}

// Each layer now saves and loads itself.
bool DisplayClass::Load(Straw &file)
{
    uint32_t tmp32;
    uint8_t tmp8;
    bool tmp;

    MapClass::Load(file);

    file.Get(TacticalCoord);
    file.Get(TacLeptonWidth);
    file.Get(TacLeptonHeight);
    file.Get(ZoneCell);
    file.Get(ZoneOffset);

    // Don't save or load CursorSize, it gets set to point at CursorShapeSave array on decode pointers call after loading.

    for (unsigned i = 0; i < ARRAY_SIZE(CursorShapeSave); ++i) {
        file.Get(CursorShapeSave[i]);
    }

    file.Get(ProximityCheck);

    // Load coded PendingObjectPtr, it will be decoded as part of the post load fixups.
    file.Get(tmp32);
    PendingObjectPtr = reinterpret_cast<ObjectClass *>(static_cast<uintptr_t>(tmp32));

    // Don't save or load PendingObject, its restored as part of post load fixup.

    file.Get(tmp8);
    PendingHouse = static_cast<HousesType>(tmp8);

    file.Get(TacPixelX);
    file.Get(TacPixelY);
    file.Get(DesiredTacticalCoord);

    file.Get(tmp);
    IsToRedraw = tmp;
    file.Get(tmp);
    IsRepairMode = tmp;
    file.Get(tmp);
    IsSellMode = tmp;

    file.Get(tmp8);
    IsTargettingMode = static_cast<SpecialWeaponType>(tmp8);

    file.Get(tmp);
    IsRubberBand = tmp;
    file.Get(tmp);
    IsTentative = tmp;
    file.Get(tmp);
    IsShadowPresent = tmp;

    file.Get(BandX);
    file.Get(BandY);
    file.Get(NewX);
    file.Get(NewY);

    return true;
}

bool DisplayClass::Save(Pipe &file) const
{
    uint32_t tmp32;
    uint8_t tmp8;
    bool tmp;

    MapClass::Save(file);
    
    file.Put(TacticalCoord);
    file.Put(TacLeptonWidth);
    file.Put(TacLeptonHeight);
    file.Put(ZoneCell);
    file.Put(ZoneOffset);

    // Don't save or load CursorSize, it gets set to point at CursorShapeSave array on decode pointers call after loading.

    for (unsigned i = 0; i < ARRAY_SIZE(CursorShapeSave); ++i) {
        file.Put(CursorShapeSave[i]);
    }

    file.Put(ProximityCheck);

    // Save coded PendingObjectPtr, it will be coded as part of the pre save fixups.
    tmp32 = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(PendingObjectPtr));
    file.Put(tmp32);

    // Don't save or load PendingObject, its restored as part of post load fixup.

    tmp8 = static_cast<uint8_t>(PendingHouse);
    file.Put(tmp8);

    file.Put(TacPixelX);
    file.Put(TacPixelY);
    file.Put(DesiredTacticalCoord);

    tmp = IsToRedraw;
    file.Put(tmp);
    tmp = IsRepairMode;
    file.Put(tmp);
    tmp = IsSellMode;
    file.Put(tmp);

    tmp8 = static_cast<uint8_t>(IsTargettingMode);
    file.Put(tmp8);

    tmp = IsRubberBand;
    file.Put(tmp);
    tmp = IsTentative;
    file.Put(tmp);
    tmp = IsShadowPresent;
    file.Put(tmp);

    file.Put(BandX);
    file.Put(BandY);
    file.Put(NewX);
    file.Put(NewY);

    return true;
}

bool RadarClass::Load(Straw &file)
{
    uint8_t tmp8;
    bool tmp;

    DisplayClass::Load(file);
    
    file.Get(RadX);
    file.Get(RadOffX);
    file.Get(RadY);
    file.Get(RadOffY);
    file.Get(RadWidth);
    file.Get(RadHeight);
    file.Get(RadIWidth);
    file.Get(RadIHeight);
    file.Get(RadPWidth);
    file.Get(RadPHeight);

    file.Get(tmp);
    IsToRedraw = tmp;
    file.Get(tmp);
    RadarCursorRedraw = tmp;
    file.Get(tmp);
    DoesRadarExist = tmp;
    file.Get(tmp);
    IsRadarActive = tmp;
    file.Get(tmp);
    IsRadarActivating = tmp;
    file.Get(tmp);
    IsRadarDeactivating = tmp;

    file.Get(IsRadarJammedByPlayerMask);

    file.Get(tmp);
    IsPulseActive = tmp;
    
    file.Get(RadarPulseFrame);
    file.Get(SpecialRadarFrame);
    file.Get(RadarAnimFrame);
    
    file.Get(RadarX);
    file.Get(RadarY);
    file.Get(RadarCellWidth);
    file.Get(RadarCellHeight);
    file.Get(RadarCell);
    file.Get(BaseX);
    file.Get(BaseY);
    file.Get(RadarWidth);
    file.Get(RadarHeight);

    file.Get(tmp);
    IsZoomed = tmp;
    file.Get(tmp);
    IsPlayerNames = tmp;
    file.Get(tmp);
    IsHouseSpy = tmp;

    file.Get(ZoomFactor);

    file.Get(tmp8);
    SpyingOn = static_cast<HousesType>(tmp8);

    file.Get(PixelPtr);

    for (int i = 0; i < PIXELSTACK; ++i) {
        file.Get(PixelStack[i]);
    }

    return true;
}

bool RadarClass::Save(Pipe &file) const
{
    uint8_t tmp8;
    bool tmp;

    DisplayClass::Save(file);
    
    file.Put(RadX);
    file.Put(RadOffX);
    file.Put(RadY);
    file.Put(RadOffY);
    file.Put(RadWidth);
    file.Put(RadHeight);
    file.Put(RadIWidth);
    file.Put(RadIHeight);
    file.Put(RadPWidth);
    file.Put(RadPHeight);

    tmp = IsToRedraw;
    file.Put(tmp);
    tmp = RadarCursorRedraw;
    file.Put(tmp);
    tmp = DoesRadarExist;
    file.Put(tmp);
    tmp = IsRadarActive;
    file.Put(tmp);
    tmp = IsRadarActivating;
    file.Put(tmp);
    tmp = IsRadarDeactivating;
    file.Put(tmp);

    file.Put(IsRadarJammedByPlayerMask);

    tmp = IsPulseActive;
    file.Put(tmp);
    
    file.Put(RadarPulseFrame);
    file.Put(SpecialRadarFrame);
    file.Put(RadarAnimFrame);
    
    file.Put(RadarX);
    file.Put(RadarY);
    file.Put(RadarCellWidth);
    file.Put(RadarCellHeight);
    file.Put(RadarCell);
    file.Put(BaseX);
    file.Put(BaseY);
    file.Put(RadarWidth);
    file.Put(RadarHeight);

    tmp = IsZoomed;
    file.Put(tmp);
    tmp = IsPlayerNames;
    file.Put(tmp);
    tmp = IsHouseSpy;
    file.Put(tmp);

    file.Put(ZoomFactor);

    tmp8 = static_cast<uint8_t>(SpyingOn);
    file.Put(tmp8);

    file.Put(PixelPtr);

    for (int i = 0; i < PIXELSTACK; ++i) {
        file.Put(PixelStack[i]);
    }

    return true;
}

// Each layer now saves and loads itself.
bool PowerClass::Load(Straw &file)
{
    bool tmp;

    RadarClass::Load(file);

    file.Get(tmp);
    IsToRedraw = tmp;
    file.Get(tmp);
    IsActive = tmp;

    FlashTimer.Load(file);

    file.Get(RecordedDrain);
    file.Get(RecordedPower);
    file.Get(DesiredDrainHeight);
    file.Get(DesiredPowerHeight);
    file.Get(DrainHeight);
    file.Get(PowerHeight);
    file.Get(DrainBounce);
    file.Get(PowerBounce);
    file.Get(PowerDir);
    file.Get(DrainDir);

    return true;
}

bool PowerClass::Save(Pipe &file) const
{
    bool tmp;

    RadarClass::Save(file);

    tmp = IsToRedraw;
    file.Put(tmp);
    tmp = IsActive;
    file.Put(tmp);

    FlashTimer.Save(file);

    file.Put(RecordedDrain);
    file.Put(RecordedPower);
    file.Put(DesiredDrainHeight);
    file.Put(DesiredPowerHeight);
    file.Put(DrainHeight);
    file.Put(PowerHeight);
    file.Put(DrainBounce);
    file.Put(PowerBounce);
    file.Put(PowerDir);
    file.Put(DrainDir);

    return true;
}

bool SidebarClass::StripClass::Save(Pipe &file) const
{
    bool tmp;

    file.Put(X);
    file.Put(Y);
    file.Put(ID);

    tmp = IsToRedraw;
    file.Put(tmp);
    tmp = IsBuilding;
    file.Put(tmp);
    tmp = IsScrollingDown;
    file.Put(tmp);
    tmp = IsScrolling;
    file.Put(tmp);

    file.Put(Flasher);
    file.Put(TopIndex);
    file.Put(Scroller);
    file.Put(Slid);
    file.Put(LastSlid);
    file.Put(BuildableCount);

    for (int i = 0; i < MAX_BUILDABLES; ++i) {
        file.Put(Buildables[i].BuildableID);
        static_assert(sizeof(Buildables[i].BuildableType) == sizeof(uint8_t), "BuildableType size not as expected.");
        uint8_t tmp = Buildables[i].BuildableType;
        file.Put(tmp);
        file.Put(Buildables[i].Factory);
        file.Put(Buildables[i].BuildableViaCapture);
    }

    return true;
}

bool SidebarClass::StripClass::Load(Straw &file)
{
    bool tmp;

    file.Get(X);
    file.Get(Y);
    file.Get(ID);

    file.Get(tmp);
    IsToRedraw = tmp;
    file.Get(tmp);
    IsBuilding = tmp;
    file.Get(tmp);
    IsScrollingDown = tmp;
    file.Get(tmp);
    IsScrolling = tmp;
    
    file.Get(Flasher);
    file.Get(TopIndex);
    file.Get(Scroller);
    file.Get(Slid);
    file.Get(LastSlid);
    file.Get(BuildableCount);

    for (int i = 0; i < MAX_BUILDABLES; ++i) {
        file.Get(Buildables[i].BuildableID);
        uint8_t tmp;
        file.Get(tmp);
        Buildables[i].BuildableType = static_cast<RTTIType>(tmp);
        file.Get(Buildables[i].Factory);
        file.Get(Buildables[i].BuildableViaCapture);
    }

    return true;
}

bool SidebarClass::Load(Straw &file)
{
    bool tmp;
    
    PowerClass::Load(file);

    for (int i = 0; i < COLUMNS; ++i) {
        Column[i].Load(file);
    }

    file.Get(tmp);
    IsSidebarActive = tmp;
    file.Get(tmp);
    IsToRedraw = tmp;

    file.Get(tmp);
    IsRepairActive = tmp;
    file.Get(tmp);
    IsUpgradeActive = tmp;
    file.Get(tmp);
    IsDemolishActive = tmp;

    return true;
}

bool SidebarClass::Save(Pipe &file) const
{
    bool tmp;

    PowerClass::Save(file);

    for (int i = 0; i < COLUMNS; ++i) {
        Column[i].Save(file);
    }

    tmp = IsSidebarActive;
    file.Put(tmp);
    tmp = IsToRedraw;
    file.Put(tmp);

    tmp = IsRepairActive;
    file.Put(tmp);
    tmp = IsUpgradeActive;
    file.Put(tmp);
    tmp = IsDemolishActive;
    file.Put(tmp);

    return true;
}

bool TabClass::Load(Straw &file)
{
    bool tmp;
    
    SidebarClass::Load(file);

    Credits.Load(file);
    FlasherTimer.Load(file);

    file.Get(tmp);
    IsToRedraw = tmp;

    MoneyFlashTimer.Load(file);

    return true;
}

bool TabClass::Save(Pipe &file) const
{
    bool tmp;

    SidebarClass::Save(file);

    Credits.Save(file);
    FlasherTimer.Save(file);

    tmp = IsToRedraw;
    file.Put(tmp);

    MoneyFlashTimer.Save(file);

    return true;
}

bool HelpClass::Load(Straw &file)
{
    bool tmp;
    
    TabClass::Load(file);

    file.Get(HelpX);
    file.Get(HelpY);
    file.Get(HelpWidth);

    file.Get(tmp);
    IsRight = tmp;

    file.Get(Cost);
    file.Get(X);
    file.Get(Y);
    file.Get(DrawX);
    file.Get(DrawY);
    file.Get(Width);
    file.Get(Text);
    file.Get(Color);

    CountDownTimer.Load(file);

    return true;
}

bool HelpClass::Save(Pipe &file) const
{
    bool tmp;

    TabClass::Save(file);

    file.Put(HelpX);
    file.Put(HelpY);
    file.Put(HelpWidth);

    tmp = IsRight;
    file.Put(tmp);

    file.Put(Cost);
    file.Put(X);
    file.Put(Y);
    file.Put(DrawX);
    file.Put(DrawY);
    file.Put(Width);
    file.Put(Text);
    file.Put(Color);
    
    CountDownTimer.Save(file);

    return true;
}

bool ScrollClass::Load(Straw &file)
{
    bool tmp;
    
    HelpClass::Load(file);

    file.Get(tmp);
    IsAutoScroll = tmp;

    Counter.Load(file);

    file.Get(Inertia);

    return true;
}

bool ScrollClass::Save(Pipe &file) const
{
    bool tmp;

    HelpClass::Save(file);

    tmp = IsAutoScroll;
    file.Put(tmp);

    Counter.Save(file);

    file.Put(Inertia);

    return true;
}

/***********************************************************************************************
 * MouseClass::Load -- Loads from a save game file.                                            *
 *                                                                                             *
 * Loading the map is very complicated.  Here are the steps:                                   *
 * - Read the Theater for this save-game                                                       *
 * - call Init_Theater to perform theater-specific inits                                       *
 * - call Free_Cells to free the cell array, because loading the map object will overwrite     *
 *   the pointer to the cell array                                                             *
 * - read the map object from disk                                                             *
 * - call Alloc_Cells to re-allocate the cell array                                            *
 * - call Init_Cells to set the cells to a known state, because not every cell will be loaded  *
 * - read the cell objects into the cell array                                                 *
 * - After the map & all objects have been loaded & the pointers decoded, Init_IO() >MUST< be  *
 *   called to restore the map's button list to the proper state.                              *
 *                                                                                             *
 * INPUT:   file  -- The file to read the cell's data from.                                    *
 *                                                                                             *
 * OUTPUT:  true = success, false = failure                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1994 JLB : Created.                                                                 *
 *   03/12/1996 JLB : Simplified.                                                              *
 *=============================================================================================*/
bool MouseClass::Load(Straw& file)
{
    /*
    **	Load Theater:  Even though this value is located in the DisplayClass,
    **	it must be loaded first so initialization can be done before any other
    **	map data is loaded.  If initialization isn't done first, data read from
    **	disk will be over-written when initialization occurs.  This code must
    **	go in the most-derived Map class.
    */
    int8_t theater;
    if (!file.Get(theater)) {
        return (false);
    }

    LastTheater = THEATER_NONE;

    /*
    ** Remove any old theater specific uncompressed shapes
    */
    //	if (theater != LastTheater) {
    Reset_Theater_Shapes();
    //	}

    /*
    **	Init display mixfiles
    */
    Init_Theater(static_cast<TheaterType>(theater));
    TerrainTypeClass::Init(Scen.Theater);
    TemplateTypeClass::Init(Scen.Theater);
    OverlayTypeClass::Init(Scen.Theater);
    UnitTypeClass::Init(Scen.Theater);
    InfantryTypeClass::Init(Scen.Theater);
    BuildingTypeClass::Init(Scen.Theater);
    BulletTypeClass::Init(Scen.Theater);
    AnimTypeClass::Init(Scen.Theater);
    AircraftTypeClass::Init(Scen.Theater);
    VesselTypeClass::Init(Scen.Theater);
    SmudgeTypeClass::Init(Scen.Theater);

    // LastTheater = Scen.Theater;

    /*
    ** Free the cell array, because we're about to overwrite its pointers
    */
    Free_Cells();

    /*
    ** Read the entire map object in.  Only read in sizeof(MouseClass), so if we're
    ** in editor mode, none of the map editor object is read in.
    */
    ScrollClass::Load(file);

    bool tmp;
    file.Get(tmp);
    IsSmall = tmp;

    static_assert(sizeof(MouseType) == sizeof(uint8_t), "Size of MouseType does not match size of uint8_t.");
    uint8_t tmp8;
    file.Get(tmp8);
    CurrentMouseShape = static_cast<MouseType>(tmp8);
    file.Get(tmp8);
    NormalMouseShape = static_cast<MouseType>(tmp8);

    file.Get(Frame);

#ifdef SCENARIO_EDITOR
    new (this) MapEditClass(NoInitClass());
#else
    new (this) MouseClass(NoInitClass());
#endif

    /*
    ** Reallocate the cell array
    */
    Alloc_Cells();

    /*
    ** Init all cells to empty
    */
    Init_Cells();

    /*
    **	Read # cells saved
    */
    int count;
    if (!file.Get(count)) {
        return (false);
    }

    /*
    **	Read cells
    */
    for (int index = 0; index < count; index++) {
        CELL cell = 0;
        if (!file.Get(cell)) {
            return (false);
        }

        if (!(*this)[cell].Load(file)) {
            return (false);
        }
    }

    LastTheater = Scen.Theater;
    return (true);
}

/***********************************************************************************************
 * MouseClass::Save -- Save to a save game file.                                               *
 *                                                                                             *
 * INPUT:   file  -- The file to write the cell's data to.                                     *
 *                                                                                             *
 * OUTPUT:  true = success, false = failure                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/19/1994 JLB : Created.                                                                 *
 *   02/26/1996 JLB : Cleaned up.                                                              *
 *=============================================================================================*/
bool MouseClass::Save(Pipe& file) const
{
    /*
    **	Save Theater >first<
    */
    int8_t theater = Scen.Theater;
    file.Put(theater);

    ScrollClass::Save(file);

    bool tmp = IsSmall;
    file.Put(tmp);

    uint8_t tmp8 = CurrentMouseShape;
    file.Put(tmp8);
    tmp8 = NormalMouseShape;
    file.Put(tmp8);

    file.Put(Frame);

    /*
    **	Count how many cells will be saved.
    */
    int count = 0;
    CellClass const* cellptr = &(*this)[(CELL)0];
    for (CELL cell = 0; cell < MAP_CELL_TOTAL; cell++) {
        if (cellptr->Should_Save()) {
            count++;
        }
        cellptr++;
    }

    /*
    **	write out count of the cells.
    */
    file.Put(count);

    /*
    **	Save cells that need it
    */
    cellptr = &(*this)[(CELL)0];
    for (CELL cell = 0; cell < MAP_CELL_TOTAL; cell++) {
        if (cellptr->Should_Save()) {
            file.Put(&cell, sizeof(cell));
            cellptr->Save(file);
            count--;
        }
        cellptr++;
    }

    if (count != 0)
        return (false);

    return (true);
}

/***********************************************************************************************
 * DisplayClass::Code_Pointers -- codes class's pointers for load/save                         *
 *                                                                                             *
 * This routine "codes" the pointers in the class by converting them to a number               *
 * that still represents the object pointed to, but isn't actually a pointer.  This            *
 * allows a saved game to properly load without relying on the games data still                *
 * being in the exact same location.                                                           *
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
 *   01/02/1995 BR : Created.                                                                  *
 *=============================================================================================*/
void DisplayClass::Code_Pointers(void)
{
    /*
    **	Code PendingObjectPtr.
    */
    if (PendingObjectPtr) {
        PendingObjectPtr = (ObjectClass*)(intptr_t)PendingObjectPtr->As_Target();
    }

    /*
    ** Fix for saving game while in structure placement mode. ST - 4/15/2020 2:41PM
    */
    memset(CursorShapeSave, 0, sizeof(CursorShapeSave));
    if (CursorSize && CursorSize != CursorShapeSave) {

        int save_buffer_element_size = sizeof(CursorShapeSave) / sizeof(CursorShapeSave[0]);

        int index = 0;

        while (index < save_buffer_element_size - 2 && CursorSize[index] != REFRESH_EOL) {
            CursorShapeSave[index] = CursorSize[index];
            index++;
        }
        CursorShapeSave[index] = REFRESH_EOL;
    }

    /*
    **	Chain to parent.
    */
    MapClass::Code_Pointers();
}

/***********************************************************************************************
 * DisplayClass::Decode_Pointers -- decodes pointers for load/save                             *
 *                                                                                             *
 * This routine "decodes" the pointers coded in Code_Pointers by converting the                *
 * code values back into object pointers.                                                      *
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
 *   01/02/1995 BR : Created.                                                                  *
 *=============================================================================================*/
void DisplayClass::Decode_Pointers(void)
{
    /*
    **	Decode PendingObjectPtr.  We can't decode PendingObject here, because we'd
    **	have to reference PendingObjectPtr->Class_Of(), and the object that
    **	PendingObjectPtr is pointing to hasn't been decoded yet.  Since we can't
    **	decode PendingObjectPtr, we can't set the placement cursor shape here
    **	either.  These have to be done as last-minute fixups.
    */
    if (PendingObjectPtr) {
        PendingObjectPtr = As_Object(TARGET_SAFE_CAST(PendingObjectPtr), false);
        assert(PendingObjectPtr != NULL);
    }

    if (CursorSize) {
        CursorSize = CursorShapeSave;
    }

    /*
    **	Chain to parent.
    */
    MapClass::Decode_Pointers();
}

/***********************************************************************************************
 * MapClass::Code_Pointers -- codes class's pointers for load/save                             *
 *                                                                                             *
 * This routine "codes" the pointers in the class by converting them to a number               *
 * that still represents the object pointed to, but isn't actually a pointer.  This            *
 * allows a saved game to properly load without relying on the games data still                *
 * being in the exact same location.                                                           *
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
 *   01/02/1995 BR : Created.                                                                  *
 *=============================================================================================*/
void MapClass::Code_Pointers(void)
{
    CELL cell;

    for (cell = 0; cell < MAP_CELL_TOTAL; cell++) {
        (*this)[cell].Flag_Destroy();
    }

    CellClass* cellptr = &(*this)[(CELL)0];
    for (cell = 0; cell < MAP_CELL_TOTAL; cell++) {
        cellptr->Code_Pointers();
        cellptr++;
    }
}

/***********************************************************************************************
 * MapClass::Decode_Pointers -- decodes pointers for load/save                                 *
 *                                                                                             *
 * This routine "decodes" the pointers coded in Code_Pointers by converting the                *
 * code values back into object pointers.                                                      *
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
 *   01/02/1995 BR : Created.                                                                  *
 *=============================================================================================*/
void MapClass::Decode_Pointers(void)
{
    CellClass* cellptr = &(*this)[(CELL)0];
    for (CELL cell = 0; cell < MAP_CELL_TOTAL; cell++) {
        cellptr->Decode_Pointers();
        cellptr++;
    }
}
