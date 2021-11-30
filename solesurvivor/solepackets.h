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
#ifndef SOLEPACKETS_H
#define SOLEPACKETS_H

#include "soleparams.h"
#include "function.h"
#include <stdint.h>

enum PacketType
{
    PACKET_0,
    PACKET_CONNECTION,
    PACKET_SIDEBAR,
    PACKET_EVENT,
    PACKET_GAME_OPTIONS,
    PACKET_PLAYER_ACTIONS,
    PACKET_FRAMERATE,
    PACKET_OBJECT,
    PACKET_WAIT1,
    PACKET_WAIT2,
    PACKET_HOUSE_TEAM,
    PACKET_NEW_DELETE_OBJ,
    PACKET_HEALTH,
    PACKET_DAMAGE,
    PACKET_CRUSH,
    PACKET_CAPTURE,
    PACKET_CARGO,
    PACKET_FLAG,
    PACKET_CTF,
    PACKET_MOVEMENT,
    PACKET_TARGET,
    PACKET_FIRE_AT,
    PACKET_DO_TURN,
    PACKET_CRATE,
    PACKET_PCP,
    PACKET_TECHNO,
    PACKET_UI_EVENT,
    PACKET_GAME_RESULTS,
    PACKET_SCENARIO_CHANGE,
    PACKET_MESSAGE,
    PACKET_OBFUSCATED_MESSAGE,
    PACKET_SERVER_PASSWORD,
    PACKET_COUNT,
};

#pragma pack(push, 1)
struct PacketHeader
{
    uint16_t PacketLength;
    uint8_t PacketType;
};

struct ConnectionPacket
{
    PacketHeader Header;
    char PlayerName[12];
    uint8_t Side;
    uint8_t ChosenRTTI;
    int32_t ChosenType;
    int32_t field_15;
    int32_t field_19;
    int32_t VersionNumber;
};

struct GameOptionsPacket
{
    PacketHeader Header;
    uint8_t PlayerID;
    uint8_t PlayerHouse;
    int32_t PlayerColorIdx;
    uint8_t Scenario;
    int32_t Credits;
    uint32_t Bitfield1;
    uint8_t BuildLevel;
    uint8_t MPlayerUnitCount;
    uint32_t Special;
    GameParamStruct GameParams;
    int32_t TimerVal;
    uint32_t Bitfield2;
    int32_t TeamScores[4];
    uint8_t Damages[25];
    uint8_t RateOfFires[25];
    int32_t Ranges[25];
};

struct NewDeleteData
{
    bool ToDelete;
    TARGET Target;
    COORDINATE Coord;
    uint8_t Owner;
    uint8_t Mission;
    uint8_t Type;
    uint8_t Strength;
    uint8_t Speed;
    uint8_t Damage;
    uint8_t RateOfFire;
    uint8_t Range;
};

struct NewDeletePacket
{
    PacketHeader Header;
    uint8_t DataCount;
    NewDeleteData Data[11];
};

struct HealthData
{
    TARGET Target;
    int16_t Strength;
};

struct HealthPacket
{
    PacketHeader Header;
    uint8_t DataCount;
    HealthData Data[33];
};

struct DamageData
{
    TARGET Target;
    TARGET Source;
    WarheadType Warhead;
};

struct DamagePacket
{
    PacketHeader Header;
    uint8_t DataCount;
    DamageData Data[22];
};

struct CrushData
{
    TARGET Target;
    TARGET Source;
};

struct CrushPacket
{
    PacketHeader Header;
    uint8_t DataCount;
    CrushData Data[25];
};

struct CaptureData
{
    TARGET Target;
    HousesType House;
};

struct CapturePacket
{
    PacketHeader Header;
    uint8_t DataCount;
    CaptureData Data[40];
};

struct CargoData
{
    TARGET Cargo;
    TARGET Transport;
    bool Embarking;
};

struct CargoPacket
{
    PacketHeader Header;
    uint8_t DataCount;
    CargoData Data[22];
};

struct FlagData
{
    uint8_t House;
    TARGET Location;
    bool Attaching;
};

struct FlagPacket
{
    PacketHeader Header;
    uint8_t DataCount;
    FlagData Data[33];
};

struct CTFData
{
    uint8_t Type;
    CELL Cell;
    uint8_t Unk;
};

struct CTFPacket
{
    PacketHeader Header;
    uint8_t DataCount;
    CTFData Data[50];
};

struct MovementData
{
    TARGET Target;
    CELL Destination;
    FacingType Facing;
};

struct MovementPacket
{
    PacketHeader Header;
    uint8_t DataCount;
    MovementData Data[28];
};

struct TargetData
{
    TARGET Object;
    TARGET TarCom;
};

struct TargetPacket
{
    PacketHeader Header;
    uint8_t DataCount;
    TargetData Data[25];
};

struct FireAtData
{
    TARGET Whom;
    TARGET Target;
    uint8_t Which;
};

struct FireAtPacket
{
    PacketHeader Header;
    uint8_t DataCount;
    FireAtData Data[22];
};

struct DoTurnData
{
    TARGET Whom;
    DirType Dir;
};

struct DoTurnPacket
{
    PacketHeader Header;
    uint8_t DataCount;
    DoTurnData Data[40];
};

struct CrateData
{
    CELL Cell;
    OverlayType Overlay;
    uint8_t Frame;
};

struct CratePacket
{
    PacketHeader Header;
    uint8_t DataCount;
    CrateData Data[50];
};

struct PerCellData
{
    TARGET Target;
    HousesType Owner;
    CELL Cell;
    int8_t Number;
    uint32_t IntNumber;
};

struct PerCellPacket
{
    PacketHeader Header;
    uint8_t DataCount;
    PerCellData Data[16];
};

struct TechnoData
{
    TARGET Whom;
    uint8_t Type;
    uint8_t Value;
};

struct TechnoPacket
{
    PacketHeader Header;
    uint8_t DataCount;
    TechnoData Data[33];
};

#pragma pack(pop)

extern int PacketLength[PACKET_COUNT];

#endif
