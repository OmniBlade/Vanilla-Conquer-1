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
#include <comm/comms.h>
#include "gprotocol.h"
#include "random.h"
#include "timer.h"
#include "wdt.h"
#include <common/internet.h>
#include <time.h>

//extern ListenerProtocolClass *Protocol;
//extern ListenerClass *Listener;
//extern long PlanetWestwoodPortNumber;
//extern int WDTRadarAdded; // TEMP.CPP
//extern int sole_array[SOLE_ARRAY_COUNT][3];
//extern int sole_array2[SOLE_ARRAY_COUNT];
//extern bool CratesDisabled; // TEMP.CPP
//extern TimerClass WDTGameTimer; // GLOBALS.CPP
//extern char WDTMapOverride[80];
//extern bool Overlay_Is_Crate(OverlayType type);
//extern void Client_Comm_Movement(FootClass *obj, CELL cell);
//extern bool DoFullRedraw;
//extern CELL FlagHomes[4];
//extern void Add_WDT_Radar(void);
//extern void Client_Handle_Sight(void);
//extern TimerClass ArmageddonDelayTimer;

// Looks like this replaces one of the pick random templates?
//int WDT_Random_Pick(int minval, int maxval);

static UnitType GDI_AI_Units[] =
    {UNIT_HTANK, UNIT_MTANK, UNIT_APC, UNIT_MSAM, UNIT_JEEP, UNIT_MSAM, UNIT_VICE, UNIT_HTANK};

static UnitType Nod_AI_Units[] =
    {UNIT_LTANK, UNIT_STANK, UNIT_FTANK, UNIT_BUGGY, UNIT_ARTY, UNIT_BIKE, UNIT_MSAM, UNIT_VICE};

StructType GDI_Defense_Buildings[] = {STRUCT_GTOWER, STRUCT_ATOWER};

StructType Nod_Defense_Buildings[] = {STRUCT_TURRET, STRUCT_OBELISK};

MissionType CommStats_Mission = MISSION_NONE;
MissionType CommStats_Suspended_Mission = MISSION_NONE;
MissionType CommStats_Mission_Queue = MISSION_NONE;
MissionType CommStats_Server_Mission = MISSION_NONE;
COORDINATE CommStats_Coord;
COORDINATE CommStats_Server_Coord;
TARGET CommStats_NavCom;
TARGET CommStats_TarCom;
int CommStats_Server_FPS;
int CommStats_Client_FPS;
int CommStats_Received_Bytes_Sec;
int CommStats_Sent_Bytes_Sec;
int CommStats_Sent_TCP;
int CommStats_Sent_UDP;
int CommStats_Received_TCP;
int CommStats_Received_UDP;
int UnknownGlobal7;
// Random3Class SyncRandom; // TODO, port Renegade Random3Class which Sole uses.
RandomClass SyncRandom;

int Host_Init_Listener()
{
    DestAddress v1;
    int v3;

    Protocol = new ListenerProtocolClass;
    Listener = new ListenerClass(Protocol);

    v1.Address.TCPIP.Host[0] = 0;
    v1.Address.TCPIP.Port = PlanetWestwoodPortNumber;
    v3 = Listener->Start_Listening(&v1, 32, 0);
    if (!v3) {
        delete Listener;
        Listener = 0;
        delete Protocol;
        Protocol = 0;
        return 0;
    }
    return 1;
}

void Host_Disconnect(void)
{
    int i;
    if (Listener) {
        Listener->Stop_Listening();
        delete Listener;
        Listener = NULL;
        delete Protocol;
        Protocol = NULL;
    }

    for (i = 0; i < ActivePlayers.Count(); i++) {
        if (ReliableComms[i]) {
            ReliableComms[i]->Disconnect();
            delete ActivePlayers[i];
            delete ReliableComms[i];
            delete ReliableProtocols[i];
        }
    }

    for (i = 0; i < RemoteAdminsComms.Count(); i++) {
        if (RemoteAdminsComms[i]) {
            RemoteAdminsComms[i]->Disconnect();
            delete RemoteAdminsComms[i];
            delete RemoteAdminsProtocols[i];
        }
    }

    ActivePlayers.Clear();
    ReliableComms.Clear();
    ReliableProtocols.Clear();
    RemoteAdminsComms.Clear();
    RemoteAdminsProtocols.Clear();
}

int Host_Pick_Random_Map()
{
    static int RandMapIndex = rand() + time(NULL);

    int old_index;
    int is_rand;

    //probably had a always true branch, stack won't match otherwise
    if (1) {
        char file_name[260];
        char destination[80];
        int i;

        if (strlen(WDTMapOverride) > 0) {
            for (i = 0; i < MPlayerScenarios.Count(); i++) {
                strcpy(destination, MPlayerScenarios[i]);
                strupr(destination);
                if (strstr(destination, WDTMapOverride)) {
                    return MPlayerFilenum[i];
                }
            }
        }

        CDFileClass fc("SERVER.INI");
        INIClass ini;
        ini.Load(fc);

        is_rand = ini.Get_Int("GameParms", "RandomMaps", NULL);
        old_index = RandMapIndex;

        if (is_rand) {
            RandMapIndex = WDT_Random_Pick(0, MPlayerFilenum.Count() - 1) + rand() + time(NULL);
        } else {
            RandMapIndex++;
        }

        RandMapIndex %= MPlayerFilenum.Count();

        if (RandMapIndex == old_index) {
            RandMapIndex++;
            RandMapIndex %= MPlayerFilenum.Count();
        }
    }

    return MPlayerFilenum[RandMapIndex];
}

bool Client_Connect_To_IP(const char* host)
{
    char destination[32];
    ReliableCommClass* reliable;
    ReliableProtocolClass* protocol;

    protocol = new ReliableProtocolClass;
    reliable = new ReliableCommClass(protocol, 422);

    ReliableProtocols.Add(protocol);
    ReliableComms.Add(reliable);
    strcpy(destination, host);

    *(short*)&destination[30] = PlanetWestwoodPortNumber;

    if (!ReliableComms[0]->Connect(destination, sizeof(destination), 0)) {
        ReliableComms[0]->Disconnect();
        delete ReliableComms[0];
        delete ReliableProtocols[0];
        ReliableComms.Clear();
        ReliableProtocols.Clear();
        return false;
    }
    return true;
}

void Client_Disconnect(void)
{
    if (ReliableComms.Count() && ReliableComms[0]) {
        ReliableComms[0]->Disconnect();
        delete ReliableComms[0];
        delete ReliableProtocols[0];
    }

    ReliableComms.Clear();
    ReliableProtocols.Clear();
}

void Clear_Packet_Data_Vectors(void)
{
    int i;

    for (i = 0; i < NewDeletePacketDatas.Count(); i++) {
        delete NewDeletePacketDatas[i];
    }
    NewDeletePacketDatas.Delete_All();

    for (i = 0; i < HealthPacketDatas.Count(); i++) {
        delete HealthPacketDatas[i];
    }
    HealthPacketDatas.Delete_All();

    for (i = 0; i < DamagePacketDatas.Count(); i++) {
        delete DamagePacketDatas[i];
    }
    DamagePacketDatas.Delete_All();

    for (i = 0; i < SquishPacketDatas.Count(); i++) {
        delete SquishPacketDatas[i];
    }
    SquishPacketDatas.Delete_All();

    for (i = 0; i < CapturePacketDatas.Count(); i++) {
        delete CapturePacketDatas[i];
    }
    CapturePacketDatas.Delete_All();

    for (i = 0; i < CargoPacketDatas.Count(); i++) {
        delete CargoPacketDatas[i];
    }
    CargoPacketDatas.Delete_All();

    for (i = 0; i < FlagPacketDatas.Count(); i++) {
        delete FlagPacketDatas[i];
    }
    FlagPacketDatas.Delete_All();

    for (i = 0; i < CTFPacketDatas.Count(); i++) {
        delete CTFPacketDatas[i];
    }
    CTFPacketDatas.Delete_All();

    for (i = 0; i < MovePacketDatas.Count(); i++) {
        delete MovePacketDatas[i];
    }
    MovePacketDatas.Delete_All();

    for (i = 0; i < TargetPacketDatas.Count(); i++) {
        delete TargetPacketDatas[i];
    }
    TargetPacketDatas.Delete_All();

    for (i = 0; i < FireAtPacketDatas.Count(); i++) {
        delete FireAtPacketDatas[i];
    }
    FireAtPacketDatas.Delete_All();

    for (i = 0; i < DoTurnPacketDatas.Count(); i++) {
        delete DoTurnPacketDatas[i];
    }
    DoTurnPacketDatas.Delete_All();

    for (i = 0; i < CratePacketDatas.Count(); i++) {
        delete CratePacketDatas[i];
    }
    CratePacketDatas.Delete_All();

    for (i = 0; i < PerCellPacketDatas.Count(); i++) {
        delete PerCellPacketDatas[i];
    }
    PerCellPacketDatas.Delete_All();

    for (i = 0; i < TechnoPacketDatas.Count(); i++) {
        delete TechnoPacketDatas[i];
    }
    TechnoPacketDatas.Delete_All();
}

void CommStats_Set_Current_Object(COORDINATE coord,
                                  COORDINATE server_coord,
                                  MissionType mission,
                                  MissionType suspended_mission,
                                  MissionType mission_queue,
                                  MissionType server_mission,
                                  TARGET navcom,
                                  TARGET tarcom)
{
    CommStats_Coord = coord;
    CommStats_Server_Coord = server_coord;
    CommStats_Mission = mission;
    CommStats_Suspended_Mission = suspended_mission;
    CommStats_Mission_Queue = mission_queue;
    CommStats_Server_Mission = server_mission;
    CommStats_NavCom = navcom;
    CommStats_TarCom = tarcom;
}

void CommStats_Set_Frame_Rate(int serverfps, int clientfps)
{
    CommStats_Server_FPS = serverfps;
    CommStats_Client_FPS = clientfps;
}

void CommStats_Set_Transmission_Stats(int rec_bytes_sec,
                                      int sent_bytes_sec,
                                      int sent_tcp,
                                      int sent_udp,
                                      int recieved_tcp,
                                      int recieved_udp)
{
    CommStats_Received_Bytes_Sec = rec_bytes_sec;
    CommStats_Sent_Bytes_Sec = sent_bytes_sec;
    CommStats_Sent_TCP = sent_tcp;
    CommStats_Sent_UDP = sent_udp;
    CommStats_Received_TCP = recieved_tcp;
    CommStats_Received_UDP = recieved_udp;
}

void CommStats_Mono_Debug_Print(bool fresh)
{
    if (fresh) {
        Mono_Clear_Screen();
        Mono_Set_Cursor(0, 0);
        Mono_Printf("                    ????????????? Current Object ????????????\n");
        Mono_Printf("                    ³            Coord: (  :   )(  :   )    ³\n");
        Mono_Printf("                    ³     Server Coord: (  :   )(  :   )    ³\n");
        Mono_Printf("                    ³                                       ³\n");
        Mono_Printf("                    ³          Mission:                     ³\n");
        Mono_Printf("                    ³ SuspendedMission:                     ³\n");
        Mono_Printf("                    ³     MissionQueue:                     ³\n");
        Mono_Printf("                    ³   Server Mission:                     ³\n");
        Mono_Printf("                    ³           NavCom:                     ³\n");
        Mono_Printf("                    ³           TarCom:                     ³\n");
        Mono_Printf("                    ?????????????????????????????????????????\n");
        Mono_Printf("                           ??????? Frame Rate ???????\n");
        Mono_Printf("                           ³      Server:           ³\n");
        Mono_Printf("                           ³      Client:           ³\n");
        Mono_Printf("                           ³  SpeedScale:           ³\n");
        Mono_Printf("                           ??????????????????????????\n");
        Mono_Printf("                         ????? Transmission Stats ?????\n");
        Mono_Printf("                         ³        Bytes/sec  TCP  UDP ³\n");
        Mono_Printf("                         ³     Sent: xxxx    xxx  xxx ³\n");
        Mono_Printf("                         ³ Received: xxxx    xxx  xxx ³\n");
        Mono_Printf("                         ??????????????????????????????\n");
    }

    if (!(Frame & 15)) {
        Mono_Set_Cursor(41, 1);
        Mono_Printf("%03x", (CommStats_Coord & 0x0000FF00) >> 8);
        Mono_Set_Cursor(44, 1);
        Mono_Printf("%03x", (CommStats_Coord & 0x000000FF));
        Mono_Set_Cursor(49, 1);
        Mono_Printf("%03x", (CommStats_Coord & 0xFF000000) >> 24);
        Mono_Set_Cursor(52, 1);
        Mono_Printf("%03x", (CommStats_Coord & 0x00FF0000) >> 16);
        Mono_Set_Cursor(41, 2);
        Mono_Printf("%03x", (CommStats_Server_Coord & 0x0000FF00) >> 8);
        Mono_Set_Cursor(44, 2);
        Mono_Printf("%03x", (CommStats_Server_Coord & 0x000000FF));
        Mono_Set_Cursor(49, 2);
        Mono_Printf("%03x", (CommStats_Server_Coord & 0xFF000000) >> 24);
        Mono_Set_Cursor(52, 2);
        Mono_Printf("%03x", (CommStats_Server_Coord & 0x00FF0000) >> 16);
        Mono_Set_Cursor(40, 4);
        Mono_Printf("%-20s", MissionClass::Mission_Name(CommStats_Mission));
        Mono_Set_Cursor(40, 5);
        Mono_Printf("%-20s", MissionClass::Mission_Name(CommStats_Suspended_Mission));
        Mono_Set_Cursor(40, 6);
        Mono_Printf("%-20s", MissionClass::Mission_Name(CommStats_Mission_Queue));
        Mono_Set_Cursor(40, 7);
        Mono_Printf("%-20s", MissionClass::Mission_Name(CommStats_Server_Mission));
        Mono_Set_Cursor(40, 8);
        Mono_Printf("%04x", CommStats_NavCom);
        Mono_Set_Cursor(40, 9);
        Mono_Printf("%04x", CommStats_TarCom);
        Mono_Set_Cursor(42, 12);
        Mono_Printf("%03d", CommStats_Server_FPS);
        Mono_Set_Cursor(42, 13);
        Mono_Printf("%03d", CommStats_Client_FPS);
        Mono_Set_Cursor(42, 14);
        Mono_Printf("%04x", SpeedScale);
        Mono_Set_Cursor(37, 18);
        Mono_Printf("%04d", CommStats_Sent_Bytes_Sec);
        Mono_Set_Cursor(37, 19);
        Mono_Printf("%04d", CommStats_Received_Bytes_Sec);
        Mono_Set_Cursor(45, 18);
        Mono_Printf("%03d", CommStats_Sent_TCP);
        Mono_Set_Cursor(45, 19);
        Mono_Printf("%03d", CommStats_Received_TCP);
        Mono_Set_Cursor(50, 18);
        Mono_Printf("%03d", CommStats_Sent_UDP);
        Mono_Set_Cursor(50, 19);
        Mono_Printf("%03d", CommStats_Received_UDP);
    }
}

int Get_Stat(SoleArrayType get_what, int initial_val, ObjectClass* obj)
{
    const InfantryTypeClass* iptr;
    const UnitTypeClass* utptr;
    int isunit;
    int unused;

    unused = 0;
    isunit = -1;
    utptr = NULL;
    iptr = NULL;

    if (obj->What_Am_I() == RTTI_UNIT) {
        isunit = true;
        utptr = &UnitTypeClass::As_Reference(((UnitClass*)obj)->Class->Type);
    } else if (obj->What_Am_I() == RTTI_INFANTRY) {
        isunit = false;
        iptr = &InfantryTypeClass::As_Reference(((InfantryClass*)obj)->Class->Type);
    } else {
        return initial_val;
    }

    double max;
    int value;

    switch (get_what) {
    case SOLE_ARRAY_STRENGTH:
        if (isunit) {
            value = utptr->MaxStrength;
            max = value;
        } else {
            value = iptr->MaxStrength;
            max = value;
        }
        break;

    case SOLE_ARRAY_DAMAGE:
        if (isunit) {
            value = Weapons[utptr->Primary].Attack;
            max = (short)value;
        } else {
            value = Weapons[iptr->Primary].Attack;
            max = (short)value;
        }
        break;

    case SOLE_ARRAY_SPEED:
        if (isunit) {
            value = utptr->MaxSpeed;
            max = (short)value;
        } else {
            value = iptr->MaxSpeed;
            max = (short)value;
        }
        break;

    case SOLE_ARRAY_ROF:
        return initial_val;

    case SOLE_ARRAY_RANGE:
        if (isunit) {
            max = Weapons[utptr->Primary].Range;
        } else {
            max = Weapons[iptr->Primary].Range;
        }
        break;

    default:
        return initial_val;
    }

    double f_initial_val = initial_val;
    max = f_initial_val * max / sole_array2[get_what];

    max += initial_val;
    max /= 2;

    return max;
}

int Process_Crate_Pickup(WDTCrateType type, CELL cell, FootClass* object, HousesType house, int int_arg)
{
    FootClass* v127;
    char txt[80];

    char str[300];

    int a1;
    int l;
    int k;
    int v130;
    int index;
    AnimClass* anim;
    WeaponType v150;    // [esp+284h] [ebp-104h]
    WeaponType v164;    // [esp+2BCh] [ebp-CCh]
    int* v166;          // [esp+2C4h] [ebp-C4h]
    int j;              // [esp+2C8h] [ebp-C0h]
    SoleArrayType v198; // [esp+344h] [ebp-44h]
    int v199;           // [esp+348h] [ebp-40h]
    int v200;           // [esp+34Ch] [ebp-3Ch]
    //unsigned char v201; // [esp+350h] [ebp-38h]

    HouseClass* hptr;   // [esp+354h] [ebp-34h]
    HousesType actlike; // [esp+358h] [ebp-30h]
    UnitClass* optr;    // [esp+35Ch] [ebp-2Ch]
    bool steel;

    int retval = 0;
    int v208 = 1;
    CellClass* cptr = &Map[cell];

    if (cptr->Overlay == OVERLAY_STEEL_CRATE) {
        steel = true;
    } else {
        steel = false;
    }

    if (Overlay_Is_Crate(cptr->Overlay)) {
        cptr->Overlay = OVERLAY_NONE;
        cptr->OverlayData = 0;
        cptr->Redraw_Objects();
    }

    v208 = 1;
    if (GameParams.TeamCrates) {
        v208 = 0;
        for (int i = 0; i < Infantry.Count() + Units.Count(); i++) {
            if (Infantry.Count() > i) {
                optr = (UnitClass*)Infantry.Ptr(i);
                if (!(optr->IsActive)) {
                    continue;
                }
                actlike = optr->House->ActLike;
            } else {
                optr = Units.Ptr(i - Infantry.Count());
                if (!(optr->IsActive)) {
                    continue;
                }
                actlike = optr->House->ActLike;
            }

            hptr = HouseClass::As_Pointer(house);
            if (hptr->ActLike == actlike) {
                v208++;
            }
        }

        v208 /= 2;
        if (v208 < 3) {
            v208 = 3;
        }
        if (v208 > 10) {
            v208 = 10;
        }
    }
    v200 = 0;
    v199 = 0;

    switch (type) {
    case WDT_CRATE_STRENGTH:
    case WDT_CRATE_WEAPON:
    case WDT_CRATE_SPEED:
    case WDT_CRATE_RELOAD:
    case WDT_CRATE_RANGE: {

        if (type == WDT_CRATE_STRENGTH) {
            new AnimClass(ANIM_CRATE_ARMORD, Cell_Coord(cell));
        } else if (type == WDT_CRATE_WEAPON) {
            new AnimClass(ANIM_CRATE_MORTARD, Cell_Coord(cell));
        } else if (type == WDT_CRATE_SPEED) {
            new AnimClass(ANIM_CRATE_LITENNGD, Cell_Coord(cell));
        } else if (type == WDT_CRATE_RELOAD) {
            new AnimClass(ANIM_CRATE_RELOAD, Cell_Coord(cell));
        } else if (type == WDT_CRATE_RANGE) {
            new AnimClass(ANIM_CRATE_DEVIATOR, Cell_Coord(cell));
        }

        if (object == NULL || !object->IsActive) {
            break;
        }

        if (GameToPlay == GAME_HOST) {
            if (steel) {
                v200 = sole_array[type][1] / v208;
            } else {
                v200 = sole_array[type][0] / v208;
            }

            if (GameParams.TeamCrates) {
                v200++;
            }

            int_arg = v200;
            retval = int_arg;
            Map.Redraw_Tab();
        }

        if (GameToPlay != GAME_HOST || OfflineMode) {
            if (object->House == PlayerPtr) {
                if (type == WDT_CRATE_STRENGTH) {
                    Priority_Sound_Effect(VOC_CRATE_ARMOR);
                } else if (type == WDT_CRATE_WEAPON) {
                    Priority_Sound_Effect(VOC_CRATE_WEAPON);
                } else if (type == WDT_CRATE_SPEED) {
                    Priority_Sound_Effect(VOC_CRATE_SPEED);
                } else if (type == WDT_CRATE_RELOAD) {
                    Priority_Sound_Effect(VOC_CRATE_RELOAD);
                } else if (type == WDT_CRATE_RANGE) {
                    Priority_Sound_Effect(VOC_CRATE_RANGE);
                }

                if (steel) {
                    if (type == WDT_CRATE_STRENGTH) {
                        VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_MEGAARMOR);
                    } else if (type == WDT_CRATE_WEAPON) {
                        VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_MEGAWEAPON);
                    } else if (type == WDT_CRATE_SPEED) {
                        VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_MEGASPEED);
                    } else if (type == WDT_CRATE_RELOAD) {
                        VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_MEGARAPIDRELOAD);
                    } else if (type == WDT_CRATE_RANGE) {
                        VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_MEGARANGE);
                    }
                } else {
                    if (type == WDT_CRATE_STRENGTH) {
                        VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_ARMOR);
                    } else if (type == WDT_CRATE_WEAPON) {
                        VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_WEAPON);
                    } else if (type == WDT_CRATE_SPEED) {
                        VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_SPEED);
                    } else if (type == WDT_CRATE_RELOAD) {
                        VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_RAPIDRELOAD);
                    } else if (type == WDT_CRATE_RANGE) {
                        VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_RANGE);
                    }
                }
            }
        }

        if (GameParams.TeamCrates) {
            for (j = 0; j < Infantry.Count() + Units.Count(); j++) {
                if (Infantry.Count() > j) {
                    optr = (UnitClass*)Infantry.Ptr(j);
                    if (!optr->IsActive) {
                        continue;
                    }
                    actlike = optr->House->ActLike;
                } else {
                    optr = Units.Ptr(j - Infantry.Count());
                    if (!optr->IsActive) {
                        continue;
                    }
                    actlike = optr->House->ActLike;
                }

                hptr = HouseClass::As_Pointer(house);

                if (hptr->ActLike == actlike) {
                    v200 = Get_Stat((SoleArrayType)type, int_arg, optr);
                    v199 = Get_Stat((SoleArrayType)type, sole_array[type][2], optr);

                    if (type == WDT_CRATE_STRENGTH) {
                        v166 = &optr->Mod1;
                    } else if (type == WDT_CRATE_WEAPON) {
                        v166 = &optr->Mod3;
                    } else if (type == WDT_CRATE_SPEED) {
                        v166 = &optr->Mod2;
                    } else if (type == WDT_CRATE_RELOAD) {
                        v166 = &optr->Mod4;
                    } else if (type == WDT_CRATE_RANGE) {
                        v166 = &optr->Mod5;
                    }

                    *v166 += v200;

                    if (type == WDT_CRATE_RANGE) {
                        v164 = ((const TechnoTypeClass&)optr->Class_Of()).Primary;
                        if (v164 == WEAPON_FLAMETHROWER || v164 == WEAPON_FLAME_TONGUE || v164 == WEAPON_CHEMSPRAY
                            || v164 == WEAPON_STEG || v164 == WEAPON_TREX) {
                            optr->Mod5 = 0;
                        } else {
                            optr->IsPlanningToLook = true;
                        }
                    }

                    if (*v166 > v199) {
                        *v166 = v199;
                    }

                    if (type == WDT_CRATE_STRENGTH) {
                        optr->Strength += v200;
                    }
                }
            }
            Map.Redraw_Tab();
        }

        v200 = Get_Stat((SoleArrayType)type, int_arg, object);
        if (v200 <= 0) {
            v200 = 1;
        }

        optr = (UnitClass*)object;

        if (type == WDT_CRATE_STRENGTH) {
            v166 = &optr->Mod1;
        } else if (type == WDT_CRATE_WEAPON) {
            v166 = &optr->Mod3;
        } else if (type == WDT_CRATE_SPEED) {
            v166 = &optr->Mod2;
        } else if (type == WDT_CRATE_RELOAD) {
            v166 = &optr->Mod4;
        } else if (type == WDT_CRATE_RANGE) {
            v166 = &optr->Mod5;
            optr->IsPlanningToLook = true;
        }

        *v166 += v200;
        v199 = Get_Stat((SoleArrayType)type, sole_array[type][2], optr);
        if (*v166 > v199) {
            *v166 = v199;
        }

        if (type == WDT_CRATE_STRENGTH) {
            object->Strength += v200;
        }

        if (object->House == PlayerPtr) {
            Map.Redraw_Tab();
        }
        break;
    }
    case WDT_CRATE_HEAL:

        new AnimClass(ANIM_CRATE_HEALTHD, Cell_Coord(cell));

        if (object == NULL || !object->IsActive) {
            break;
        }

        if (GameToPlay == GAME_HOST) {
            object->Strength = object->Class_Of().MaxStrength + object->Mod1;
            retval = object->Strength;
            if (OfflineMode && object->House == PlayerPtr) {
                Priority_Sound_Effect(VOC_CRATE_HEAL);
                VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_HEAL);
            }
        } else {
            object->Strength = int_arg;
            if (object->House == PlayerPtr) {
                Priority_Sound_Effect(VOC_CRATE_HEAL);
                VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_HEAL);
            }
        }
        break;

    case WDT_CRATE_SUPER:

        new AnimClass(ANIM_CRATE_TIMEQK2, Cell_Coord(cell));

        if (object == NULL || !object->IsActive) {
            break;
        }

        v198 = SOLE_ARRAY_STRENGTH;
        object->Mod1 = Get_Stat(v198, sole_array[v198][2], object);
        object->Strength = object->Class_Of().MaxStrength + object->Mod1;
        v198 = SOLE_ARRAY_DAMAGE;
        object->Mod3 = Get_Stat(v198, sole_array[v198][2], object);
        v198 = SOLE_ARRAY_SPEED;
        object->Mod2 = Get_Stat(v198, sole_array[v198][2], object);
        v198 = SOLE_ARRAY_ROF;
        object->Mod4 = Get_Stat(v198, sole_array[v198][2], object);

        v150 = ((const TechnoTypeClass&)object->Class_Of()).Primary;
        if (v150 == WEAPON_FLAMETHROWER || v150 == WEAPON_FLAME_TONGUE || v150 == WEAPON_CHEMSPRAY
            || v150 == WEAPON_STEG || v150 == WEAPON_TREX) {
            object->Mod5 = 0;
        } else {
            v198 = SOLE_ARRAY_RANGE;
            object->Mod5 = Get_Stat(v198, sole_array[v198][2], object);
            object->IsPlanningToLook = true;
        }

        if (!GameParams.SuperInvuln) {
            object->Timer1.Set(0);
            object->TechnoUnk2 = false;
            object->Mark(MARK_CHANGE);
        }
        if (GameToPlay == GAME_HOST) {
            object->Make_Techno_Packet_Data(TECHNO_PACKET_DATA_ORANGE_CRATE, 1);
            object->Timer2.Set(60 * GameParams.SuperSeconds);
            object->TechnoUnk4 = true;
            Map.Redraw_Tab();
            retval = 0;
            if (OfflineMode && object->House == PlayerPtr) {
                Priority_Sound_Effect(VOC_MOTOR);
                Priority_Sound_Effect(VOC_RAMBO_ROCK);
            }
        } else {
            if (object->House == PlayerPtr) {
                Priority_Sound_Effect(VOC_MOTOR);
                Priority_Sound_Effect(VOC_RAMBO_ROCK);
                Map.Redraw_Tab();
            }
        }
        break;

    case WDT_CRATE_BOMB:

        anim = new AnimClass(ANIM_ATOM_BLAST, Cell_Coord(cell));

        if (object == NULL || !object->IsActive) {
            break;
        }

        if (object->House == PlayerPtr) {
            if (anim) {
                anim->OwnerHouse = PlayerPtr->Class->House;
            }
            VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_NUKE);
        }
        retval = 0;
        break;

    case WDT_CRATE_STEALTH:

        new AnimClass(ANIM_CRATE_STEALTH, Cell_Coord(cell));

        if (object == NULL || !object->IsActive) {
            break;
        }

        if (GameToPlay == GAME_HOST) {
            if (object->IsCloakable) {
                if (!(object->TechnoUnk3)) {
                    object->Do_Uncloak();
                    object->IsCloakable = false;
                }
            } else {
                object->IsCloakable = true;
            }
            retval = object->IsCloakable;
            if (object->House == PlayerPtr) {
                if (retval) {
                    VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_STEALTHON);
                } else {
                    VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_STEALTHOFF);
                }
            }
        } else {
            if (!int_arg) {
                object->Do_Uncloak();
            }
            object->IsCloakable = int_arg;

            if (object->House == PlayerPtr) {
                if (int_arg) {
                    VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_STEALTHON);
                } else {
                    VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_STEALTHOFF);
                }
            }
        }
        break;

    case WDT_CRATE_TELEPORT:

        if (object == NULL || !object->IsActive) {
            break;
        }

        if (int_arg == 0xFFFF) {
            CELL v131;
            for (index = 0; index < 1000; index++) {
                v131 = Map.Pick_Random_Cell();
                cptr = &Map[(CELL)v131];
                if (cptr->Is_Generally_Clear()) {
                    v130 = 0;
                    if (GameParams.IsCaptureTheFlag) {
                        for (k = 0; k < GameParams.NumTeams; k++) {
                            if (Distance(v131, FlagHomes[k]) < 25) {
                                v130 = 1;
                                break;
                            }
                        }
                    }

                    if (!v130) {
                        break;
                    }
                }
            }
            if (index == 1000) {
                v131 = Coord_Cell(object->Coord);
            }

            retval = (CELL)v131;

            for (l = 0; Logic.Count() > l; l++) {
                v127 = (FootClass*)Logic[l];
                if (v127 && !v127->IsInLimbo) {
                    a1 = v127->TarCom;
                    if (Target_Legal(a1) && As_Techno(a1) && object->As_Target() == a1) {
                        v127->Assign_Target(0);
                        v127->Assign_Destination(0);
                    }
                }
            }

            if (!Map.In_Radar(retval)) {
                sprintf(str, "*** Note: WDT.CPP line 1334: !Map.In_Radar(%d)\n", retval);
                CCDebugString(str);
            }

            break;
        }

        new AnimClass(ANIM_TELEFX, Cell_Coord(cell));

        Client_Comm_Movement(object, int_arg);
        if (object->House == PlayerPtr) {
            ScenarioInit++;
            int start_x = 0;
            int start_y = 0;
            Map.Compute_Start_Pos(start_x, start_y);
            for (int i = 0; i < ARRAY_SIZE(Scen.Views); ++i) {
                Scen.Views[i] = XY_Cell(start_x, start_y);
            }
            Scen.Waypoint[27] = XY_Cell(start_x, start_y);
            COORDINATE pos = Cell_Coord(XY_Cell(start_x, start_y));
            Map.Set_Tactical_Position(pos);
            ScenarioInit--;
            Map.Redraw_Objects();
            Priority_Sound_Effect(VOC_CRATE_TELEPORT);
            VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_TELEPORT);
        }
        break;

    case WDT_CRATE_KILL:

        if (object == NULL || !object->IsActive) {
            break;
        }

        if (house != HOUSE_ADMIN && GameParams.IonCannon == 2) {
            object->Strength = 2;
        }
        retval = 0;

        anim = new AnimClass(ANIM_ION_CANNON, object->Coord);

        if (anim) {
            anim->Attach_To(object);
            anim->IsRefCounted = true;
            object->AnimRefCount++;
        }

        if (object->House == PlayerPtr) {
            VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_ION);
        }

        if (object->House->IsHuman && GameParams.LifeLimit > 0 && GameParams.IonCannon == 2) {
            object->House->Int1--;
        }
        break;

    case WDT_CRATE_UNCLOAK_ALL:

        new AnimClass(ANIM_CRATE_UNCLOAK, Cell_Coord(cell));

        for (index = 0; index < Units.Count(); index++) {
            if (Units.Ptr(index)->IsCloakable && !Units.Ptr(index)->TechnoUnk3) {
                Units.Ptr(index)->Do_Uncloak();
                Units.Ptr(index)->IsCloakable = false;
            }
            if (Units.Ptr(index)->TechnoUnk5) {
                Units.Ptr(index)->Timer3.Set(240);
            }
        }
        for (index = 0; index < Infantry.Count(); index++) {
            if (Infantry.Ptr(index)->IsCloakable && !Infantry.Ptr(index)->TechnoUnk3) {
                Infantry.Ptr(index)->Do_Uncloak();
                Infantry.Ptr(index)->IsCloakable = false;
            }
        }

        hptr = HouseClass::As_Pointer(house);

        if (Options.IsVerbose && hptr) {
            sprintf(txt, Text_String(TXT_HAS_UNCLOAKED_ALL), hptr->Name);
            Messages.Add_Message(txt, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
            Map.Flag_To_Redraw(false);
        }

        if (object == NULL || !object->IsActive) {
            break;
        }
        if (object->House == PlayerPtr) {
            Priority_Sound_Effect(VOC_CRATE_UNCLOAK);
            VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_UNCLOAKALL);
        }
        break;

    case WDT_CRATE_RESHOUD:

        new AnimClass(ANIM_CRATE_EMPULSE, Cell_Coord(cell));

        if (!PlayerPtr->Is_Ally(house)) {
            VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_RESHROUD);
            DoFullRedraw = true;
            for (index = 0; index < 0x4000; index++) {
                cptr = &Map[index];
                if (cptr->IsMapped || cptr->IsVisible) {
                    cptr->Redraw_Objects();
                    cptr->IsMapped = false;
                    cptr->IsVisible = false;
                }
            }
            Client_Handle_Sight();
        }

        if (object == NULL || !object->IsActive) {
            break;
        }

        if (object->House == PlayerPtr) {
            Priority_Sound_Effect(VOC_CRATE_SHROUD);
        }
        if (Options.IsVerbose && (object->House == PlayerPtr || !PlayerPtr->Is_Ally(house))) {
            hptr = HouseClass::As_Pointer(house);
            if (hptr) {
                sprintf(txt, Text_String(TXT_HAS_RESHROUDED), hptr->Name);
                Messages.Add_Message(txt, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
            }
        }
        break;

    case WDT_CRATE_UNSHROUD:

        new AnimClass(ANIM_CRATE_EMPULSE, Cell_Coord(cell));

        if (PlayerPtr->Is_Ally(house) && DoFullRedraw) {
            DoFullRedraw = false;
            VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_UNSHROUD);
            for (index = 0; index < 0x4000; index++) {
                cptr = &Map[index];
                cptr->IsMapped = true;
                cptr->IsVisible = true;
                cptr->Redraw_Objects();
            }
            if (PlayerPtr->IsUnk2) {
                Map.Flag_To_Redraw(true);
                Map.IsToDrawUnknown = true;
            } else {
                Map.Redraw_Objects();
            }
        }
        if (object == NULL || !object->IsActive) {
            break;
        }
        if (object->House == PlayerPtr) {
            Priority_Sound_Effect(VOC_CRATE_UNSHROUD);
        }
        break;

    case WDT_CRATE_RADAR:

        new AnimClass(ANIM_CRATE_RADARD, Cell_Coord(cell));

        if (object == NULL || !object->IsActive) {
            break;
        }

        if (object->House == PlayerPtr) {
            VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_RADAR);
            if (!WDTRadarAdded) {
                Add_WDT_Radar();
            }
        }
        object->House->IsUnk2 = true;
        break;

    case WDT_CRATE_ARMAGEDDON:

        new AnimClass(ANIM_CRATE_RAPID, Cell_Coord(cell));

        Map.Shake_The_Screen(40);
        Priority_Sound_Effect(VOC_XPLOBIG7);
        hptr = HouseClass::As_Pointer(house);
        if (hptr) {
            sprintf(txt, Text_String(TXT_GOT_ARMAGEDDON), hptr->Name);
            Messages.Add_Message(txt, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
        }

        if (GameToPlay == GAME_HOST) {
            ArmageddonDelayTimer.Set(0, 1);
            for (HousesType hid = HOUSE_FIRST; hid < HOUSE_COUNT; hid++) {
                if (hid != HOUSE_ADMIN) {
                    hptr = HouseClass::As_Pointer(hid);
                    if (hptr) {
                        if (hptr->IsHuman && GameParams.LifeLimit > 0) {
                            hptr->Int1--;
                        }
                        hptr->Blowup_All(true);
                    }
                }
            }
        }
        VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_ARMAGEDDON);
        break;
    }

    return retval;
}

void Client_Handle_Sight(void)
{
    int i;
    TARGET flag_loc;
    ObjectClass* obj;
    CELL flag_home;
    HousesType actlike;

    actlike = PlayerPtr->ActLike;
    flag_home = HouseClass::As_Pointer(actlike)->FlagHome;
    if (flag_home) {
        Map.Sight_From(PlayerPtr, flag_home, 3, 0);
    }

    flag_loc = HouseClass::As_Pointer(actlike)->FlagLocation;
    if (Target_Legal(flag_loc)) {
        CELL flag_loc_cell = As_Cell(flag_loc);
        if (flag_loc_cell) {
            Map.Sight_From(PlayerPtr, flag_loc_cell, 3, 0);
        }
    }

    for (i = 0; i < DisplayClass::Layer[LAYER_GROUND].Count(); i++) {
        obj = DisplayClass::Layer[LAYER_GROUND][i];

        if (obj && obj->Is_Techno() && PlayerPtr->Is_Ally(((TechnoClass*)obj)->House)) {
            obj->Look(0);
        }
    }

    if (PlayerPtr->IsUnk2) {
        Map.Flag_To_Redraw(true);
        Map.IsToDrawUnknown = true;
    } else {
        Map.Redraw_Objects();
    }
}

int WDT_Random_Pick(int minval, int maxval)
{
    int val = SyncRandom(minval, maxval);
    return val;
}

void Add_WDT_Radar(void)
{
    CCDebugString("Add_WDT_Radar\n");

    if (!WDTRadarAdded) {
        Map.Radar_Activate(1);

        if (Map.Is_Zoomed()) {
            Map.Zoom_Mode(Coord_Cell(Map.TacticalCoord));
        }

        Map.Activate(0);
        WDTRadarAdded = true;
        Map.Activate(1);
    }
}

void Remove_WDT_Radar(void)
{
    CCDebugString("Remove_WDT_Radar\n");

    if (WDTRadarAdded) {
        Map.Radar_Activate(0);

        if (Map.IsSidebarActive) {
            Map.Activate(0);
            WDTRadarAdded = false;
            Map.Activate(1);
        } else {
            WDTRadarAdded = false;
        }
    }
}

TechnoClass* Create_AI_Unit(void)
{
    TechnoClass* obj = NULL;

    if (WDT_Random_Pick(0, 10) == 0) {
        obj = new UnitClass((UnitType)WDT_Random_Pick(UNIT_TRIC, UNIT_STEG), HOUSE_JP);
    } else {
        if (WDT_Random_Pick(0, 1) == 0) {
            obj = new UnitClass(GDI_AI_Units[WDT_Random_Pick(0, 6)], HOUSE_GOOD);
        } else {
            obj = new UnitClass(Nod_AI_Units[WDT_Random_Pick(0, 7)], HOUSE_BAD);
        }
    }

    if (GameParams.IsCrates) {
        obj->Mod1 = WDT_Random_Pick(0, sole_array[SOLE_ARRAY_STRENGTH][2] / 3);
        obj->Mod2 = WDT_Random_Pick(0, sole_array[SOLE_ARRAY_SPEED][2] / 3);
        obj->Mod3 = WDT_Random_Pick(0, sole_array[SOLE_ARRAY_DAMAGE][2] / 3);
        obj->Mod4 = WDT_Random_Pick(0, sole_array[SOLE_ARRAY_ROF][2] / 3);
        obj->Mod5 = WDT_Random_Pick(0, sole_array[SOLE_ARRAY_RANGE][2] / 3);
        WeaponType weap = ((const TechnoTypeClass&)obj->Class_Of()).Primary;
        if (weap == WEAPON_FLAMETHROWER || weap == WEAPON_FLAME_TONGUE || weap == WEAPON_CHEMSPRAY
            || weap == WEAPON_STEG || weap == WEAPON_TREX) {
            obj->Mod5 = 0;
        }
        obj->Strength = obj->Class_Of().MaxStrength + obj->Mod1;
    } else {
        obj->Mod1 = Get_Stat(SOLE_ARRAY_STRENGTH, sole_array[SOLE_ARRAY_STRENGTH][2], obj);
        obj->Mod2 = Get_Stat(SOLE_ARRAY_SPEED, sole_array[SOLE_ARRAY_SPEED][2], obj);
        obj->Mod3 = Get_Stat(SOLE_ARRAY_DAMAGE, sole_array[SOLE_ARRAY_DAMAGE][2], obj);
        obj->Mod4 = Get_Stat(SOLE_ARRAY_ROF, sole_array[SOLE_ARRAY_ROF][2], obj);
        obj->Mod5 = Get_Stat(SOLE_ARRAY_RANGE, sole_array[SOLE_ARRAY_RANGE][2], obj);
    }

    return obj;
}

BuildingClass* Create_Building(bool dont_rand, HousesType house, HousesType gdi_or_nod)
{
    BuildingClass* obj = NULL;

    if (gdi_or_nod && gdi_or_nod != HOUSE_BAD) {
        gdi_or_nod = HOUSE_GOOD;

        if (WDT_Random_Pick(0, 1) == 0) {
            gdi_or_nod = HOUSE_BAD;
        }
    }

    if (gdi_or_nod == HOUSE_GOOD) {
        if (house == HOUSE_NONE) {
            house = HOUSE_GOOD;
        }

        if (dont_rand) {
            obj = new BuildingClass(STRUCT_ATOWER, house);
        } else {
            obj = new BuildingClass(GDI_Defense_Buildings[WDT_Random_Pick(0, 1)], house);
        }
    } else {
        if (house == HOUSE_NONE) {
            house = HOUSE_BAD;
        }

        if (dont_rand) {
            obj = new BuildingClass(STRUCT_OBELISK, house);
        } else {
            obj = new BuildingClass(Nod_Defense_Buildings[WDT_Random_Pick(0, 1)], house);
        }
    }

    obj->Mod2 = 0;

    switch ((unsigned int)WDT_Random_Pick(0, 3)) {
    case 0:
        obj->Mod1 = WDT_Random_Pick(sole_array[0][2] / 4, sole_array[0][2]);
        break;
    case 1:
        obj->Mod3 = WDT_Random_Pick(sole_array[1][2] / 4, sole_array[1][2]);
        break;
    case 2:
        obj->Mod4 = WDT_Random_Pick(sole_array[3][2] / 4, sole_array[2][2]);
        break;
    case 3:
        obj->Mod5 = WDT_Random_Pick(sole_array[4][2] / 4, sole_array[3][2]);
        break;
    }

    obj->BuildingUnk = true;
    obj->Strength = obj->Class_Of().MaxStrength + obj->Mod1;
    obj->Mod5 = sole_array[4][2] / 2;
    return obj;
}

void Disable_Crates(void)
{
    Remove_All_Crates();
    CratesDisabled = true;
}

void Enable_Crates(void)
{
    CratesDisabled = false;
}

void Remove_All_Crates(void)
{
    CratePacketData* pkt;
    CELL i;

    for (i = 0; i < MAP_CELL_TOTAL; i++) {
        CellClass& cell = Map[i];

        if (Overlay_Is_Crate(cell.Overlay)) {
            cell.Overlay = OVERLAY_NONE;
            cell.OverlayData = 0;
            cell.Redraw_Objects();
            pkt = new CratePacketData;
            pkt->Cell = i;
            pkt->Overlay = OVERLAY_NONE;
            pkt->OverlayFrame = 0;
            CratePacketDatas.Add(pkt);
        }
    }

    CrateCount = 0;
    WDTNumArmageddonCrates = 0;
}

void Destroy_Active_AI_Objects(void)
{
    int i;
    UnitClass* uptr;
    BuildingClass* bptr;
    HouseClass* hptr;

    for (i = 0; i < Units.Count(); i++) {
        uptr = (UnitClass*)Units.Active_Ptr(i);
        hptr = HouseClass::As_Pointer(uptr->Owner());
        if (!hptr->IsHuman) {
            delete uptr;
            i--;
        }
    }

    for (i = 0; i < Buildings.Count(); i++) {
        bptr = (BuildingClass*)Buildings.Active_Ptr(i);
        hptr = HouseClass::As_Pointer(bptr->Owner());
        if (!hptr->IsHuman) {
            delete bptr;
            i--;
        }
    }
}

const char* Get_Game_Duration_As_String(void)
{
    static char Game_Duration[40];
    unsigned int days;
    unsigned int hours;
    unsigned int minutes;
    unsigned int seconds;

    seconds = WDTGameTimer.Time() / TIMER_SECOND;
    minutes = seconds / 60;
    hours = minutes / 60;
    days = hours / 24;
    minutes -= hours * 60;
    hours -= days * 24;

    sprintf(Game_Duration, "%d:%d:%d", days, hours, minutes);
    //sprintf(Game_Duration, "%d:%d:%d", current_time / 60 / 60 / 24, current_time / 60 / 60 % 24, current_time / 60 % 60);
    return Game_Duration;
}

int Sum_Object_Stats(ObjectClass* object)
{
    SoleArrayType get_what;
    int str;
    int spd;
    int dmg;
    int rof;
    int rng;
    int sum;
    const TechnoTypeClass* class_of;
    TechnoClass* obj;

    obj = (TechnoClass*)object;

    get_what = SOLE_ARRAY_STRENGTH;
    class_of = (const TechnoTypeClass*)&object->Class_Of();
    str = class_of->MaxStrength + object->Mod1 - 170;
    str = str * 100 / Get_Stat(get_what, sole_array[get_what][2], obj);

    get_what = SOLE_ARRAY_SPEED;
    spd = class_of->MaxSpeed + object->Mod2 - 35;
    spd = spd * 100 / Get_Stat(get_what, sole_array[get_what][2], obj);

    get_what = SOLE_ARRAY_DAMAGE;
    dmg = Weapons[class_of->Primary].Attack + object->Mod3 - 60;
    dmg = dmg * 100 / Get_Stat(get_what, sole_array[get_what][2], obj);

    get_what = SOLE_ARRAY_ROF;
    rof = 60 - (Weapons[class_of->Primary].ROF - object->Mod4);
    rof = rof * 100 / Get_Stat(get_what, sole_array[get_what][2], obj);

    get_what = SOLE_ARRAY_RANGE;
    rng = Weapons[class_of->Primary].Range + object->Mod5 - 1152;
    rng = rng * 100 / Get_Stat(get_what, sole_array[get_what][2], obj);

    sum = str + spd + dmg + rof + rng;
    sum -= 20;

    return sum;
}

float Armageddon_Probability(int* highest_team, int* lowest_team)
{
    int min_index;
    float team_totals[4];
    int i;
    int max_index;
    float retval;
    HousesType actlike;

    *highest_team = *lowest_team = 0;

    for (i = 0; i < 4; i++) {
        team_totals[i] = 0.0;
    }

    if (Infantry.Count() + Units.Count() <= 2) {
        return 1.0;
    }

    for (i = 0; i < Infantry.Count() + Units.Count(); i++) {
        FootClass* fptr;

        // TODO, won't this always be true?
        if (Infantry.Count() > i) {
            fptr = Infantry.Ptr(i);

            if (!fptr->IsActive) {
                continue;
            }
            actlike = fptr->House->ActLike;
        } else {
            fptr = Units.Ptr(i - Infantry.Count());

            if (!fptr->IsActive) {
                continue;
            }
            actlike = fptr->House->ActLike;
        }

        if (actlike >= HOUSE_BLUE_TEAM && actlike <= HOUSE_GREY_TEAM) {
            int object_sum = Sum_Object_Stats(fptr);
            if (object_sum < 0) {
                object_sum = 0;
            }

            object_sum /= 3;
            object_sum += 15;
            team_totals[actlike - HOUSE_BLUE_TEAM] += object_sum;
        }
    }

    min_index = max_index = 0;

    for (i = 0; i < 4; i++) {
        if (team_totals[i] >= 1.0) {
            min_index = i;
        }
    }

    for (i = 0; i < 4; i++) {
        if (team_totals[i] < team_totals[min_index] && team_totals[i] >= 1.0) {
            min_index = i;
        }
    }

    for (i = 0; i < 4; i++) {
        if (team_totals[i] > team_totals[max_index]) {
            max_index = i;
        }
    }

    if (min_index == max_index) {
        return 1.0;
    }

    if (team_totals[min_index] < 1.0 || team_totals[max_index] < 1.0) {
        return 1.0;
    }

    *highest_team = max_index + 6;
    *lowest_team = min_index + 6;
    retval = team_totals[max_index] / team_totals[min_index];

    if (retval < 1.1) {
        *highest_team = *lowest_team = 0;
    }

    return retval;
}

void Update_Tracking(void)
{
    HouseClass* hptr;
    int x;
    int y;
    CELL cell = 0;
    HousesType team;

    if (GameParams.Football && GameParams.FootballNumFlags == 1) {
        team = HOUSE_GREEN_TEAM;
    } else {
        team = PlayerPtr->ActLike;
    }

    if (team >= HOUSE_BLUE_TEAM && team <= HOUSE_GREY_TEAM) {
        hptr = HouseClass::As_Pointer(team);
        if (Target_Legal(hptr->FlagLocation)) {
            cell = As_Cell(hptr->FlagLocation);
        }
        if (!cell) {
            UnitClass* uptr = As_Unit(hptr->FlagLocation);
            if (uptr) {
                cell = Coord_Cell(uptr->Coord);
            } else {
                InfantryClass* iptr = As_Infantry(hptr->FlagLocation);
                if (iptr) {
                    cell = Coord_Cell(iptr->Coord);
                }
            }
        }
    }

    if (cell) {
        x = Cell_X(cell);
        y = Cell_Y(cell);
        x -= Lepton_To_Cell(Map.TacLeptonWidth) / 2;
        y -= Lepton_To_Cell(Map.TacLeptonHeight) / 2;

        if (x < Map.MapCellX) {
            x = Map.MapCellX;
        }

        if (x + Lepton_To_Cell(Map.TacLeptonWidth) > Map.MapCellX + Map.MapCellWidth) {
            x = (Map.MapCellX + Map.MapCellWidth) - Lepton_To_Cell(Map.TacLeptonWidth);
        }

        if (y < Map.MapCellY) {
            y = Map.MapCellY;
        }

        if (y + Lepton_To_Cell(Map.TacLeptonHeight) > Map.MapCellY + Map.MapCellHeight) {
            y = (Map.MapCellY + Map.MapCellHeight) - Lepton_To_Cell(Map.TacLeptonHeight);
        }

        Map.Set_Tactical_Position(Cell_Coord(XY_Cell(x, y)));

        for (int i = 0; i < sizeof(Scen.Views) / sizeof(Scen.Views[0]); i++) {
            Scen.Views[i] = Coord_Cell(Map.TacticalCoord);
        }
    }
}

void Update_CTF_Tracking(void)
{
    HouseClass* hptr;
    int x;
    int y;
    CELL cell = 0;
    HousesType team = PlayerPtr->ActLike;

    if (team >= HOUSE_BLUE_TEAM && team <= HOUSE_GREY_TEAM) {
        hptr = HouseClass::As_Pointer(team);
        cell = hptr->FlagHome;
    }

    if (cell) {
        x = Cell_X(cell);
        y = Cell_Y(cell);
        x -= Lepton_To_Cell(Map.TacLeptonWidth) / 2;
        y -= Lepton_To_Cell(Map.TacLeptonHeight) / 2;

        if (x < Map.MapCellX) {
            x = Map.MapCellX;
        }

        if (x + Lepton_To_Cell(Map.TacLeptonWidth) > Map.MapCellX + Map.MapCellWidth) {
            x = (Map.MapCellX + Map.MapCellWidth) - Lepton_To_Cell(Map.TacLeptonWidth);
        }

        if (y < Map.MapCellY) {
            y = Map.MapCellY;
        }

        if (y + Lepton_To_Cell(Map.TacLeptonHeight) > Map.MapCellY + Map.MapCellHeight) {
            y = (Map.MapCellY + Map.MapCellHeight) - Lepton_To_Cell(Map.TacLeptonHeight);
        }

        Map.Set_Tactical_Position(Cell_Coord(XY_Cell(x, y)));

        for (int i = 0; i < sizeof(Scen.Views) / sizeof(Scen.Views[0]); i++) {
            Scen.Views[i] = Coord_Cell(Map.TacticalCoord);
        }
    }
}

//Added in Sole 1.01
int Calculate_Points(HousesType player)
{
    double enemies_allies_ratio;
    double enemies_allies_ratio_scaled_10;
    HouseClass* hptr;
    HousesType house;
    int num_enemies;
    int num_allies;

    num_allies = 0;
    num_enemies = 0;

    for (house = HOUSE_MULTI1; house < HOUSE_MULTI100; house++) {
        hptr = HouseClass::As_Pointer(house);
        //Check for Hunter name was added in 1.05
        if (hptr->IsHuman && !hptr->IsVisionary && stricmp(hptr->Name, Text_String(TXT_HUNTER)) != 0) {
            if (hptr->Is_Ally(player)) {
                num_allies++;
            } else {
                num_enemies++;
            }
        }
    }

    if (num_enemies == 0) {
        int i;

        for (i = 0; i < Units.Count(); i++) {
            UnitClass* uptr = (UnitClass*)Units.Active_Ptr(i);
            hptr = HouseClass::As_Pointer(uptr->Owner());

            if (!hptr->IsHuman && !hptr->Is_Ally(player)) {
                num_enemies++;
            }
        }

        for (i = 0; i < Infantry.Count(); i++) {
            InfantryClass* iptr = (InfantryClass*)Infantry.Active_Ptr(i);
            hptr = HouseClass::As_Pointer(iptr->Owner());

            if (!hptr->IsHuman && !hptr->Is_Ally(player)) {
                num_enemies++;
            }
        }

        for (i = 0; i < Buildings.Count(); i++) {
            BuildingClass* bptr = (BuildingClass*)Buildings.Active_Ptr(i);
            hptr = HouseClass::As_Pointer(bptr->Owner());
            //Check for Hunter name was added in 1.05
            if (!hptr->Is_Ally(player) && stricmp(hptr->Name, Text_String(TXT_HUNTER)) == 0) {
                num_enemies++;
            }
        }
    }

    enemies_allies_ratio_scaled_10 = 0.0;
    if (num_allies > 0) {
        enemies_allies_ratio = (double)num_enemies / (double)num_allies;
        enemies_allies_ratio_scaled_10 = enemies_allies_ratio * 10.0;

        if (GameParams.IsLamerCorrection && enemies_allies_ratio <= 0.5) {
            enemies_allies_ratio_scaled_10 = enemies_allies_ratio_scaled_10 * enemies_allies_ratio;
        }
    }

    return enemies_allies_ratio_scaled_10 + 0.5;
}

void Show_Key_Commands(void)
{
    int y = 30;
    Conquer_Clip_Text_Print(
        Text_String(TXT_KEY_SUMMARY), 10, y, 5u, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print("B", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        "Center view on your Base.", 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print("D", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_DROP_ANY_FLAG), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print("F", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_CENTER_ON_FLAG), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print("G", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_INTO_GUARD_MODE), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print(
        Text_String(TXT_H_OR_HOME), 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_CENTER_ON_UNIT), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print("L", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(Text_String(TXT_CLEAN_UP_PLAYER_LIST),
                            200,
                            y,
                            0xFu,
                            TBLACK,
                            TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 15;
    Conquer_Clip_Text_Print("N", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_TOGGLE_NAMES), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print("T", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_TOGGLE_TRACKING), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print(
        Text_String(TXT_SHIFT_S), 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_TAKE_SCREENSHOT), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print(
        Text_String(TXT_ENTER), 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_NORMAL_MESSAGING), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print(
        Text_String(TXT_SHIFT_ENTER), 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_TEAM_MESSAGING), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print(
        Text_String(TXT_ENTER_NAME_MESSAGE), 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(Text_String(TXT_SEND_PRIVATE_MESSAGE),
                            200,
                            y,
                            0xFu,
                            TBLACK,
                            TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 15;
    Conquer_Clip_Text_Print(
        Text_String(TXT_UP_ARROW), 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(Text_String(TXT_REPEAT_PRV_MSG_DEST),
                            200,
                            y,
                            0xFu,
                            TBLACK,
                            TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 15;
    Conquer_Clip_Text_Print(
        Text_String(TXT_ESCAPE), 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_OPTIONS_MENU_DOT), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print(
        Text_String(TXT_TAB), 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_TOGGLE_SIDEBAR), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print(
        Text_String(TXT_1_THROUGH_0), 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(Text_String(TXT_PREPARE_SEND_MSG_MACRO),
                            200,
                            y,
                            0xFu,
                            TBLACK,
                            TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 15;
    Conquer_Clip_Text_Print("F1", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_TOGGLE_HELP), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print("F2", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(Text_String(TXT_TOGGLE_PARAM_DISPLAY),
                            200,
                            y,
                            0xFu,
                            TBLACK,
                            TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 15;
    Conquer_Clip_Text_Print("F3", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(Text_String(TXT_TOGGLE_COMMAND_DISPLAY),
                            200,
                            y,
                            0xFu,
                            TBLACK,
                            TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 15;
    Conquer_Clip_Text_Print(
        Text_String(TXT_CTRL_F7_TO_F10), 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_REMEMBER_VIEW), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print(
        Text_String(TXT_F7_TO_F10), 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(Text_String(TXT_SHOW_REMEMBERED_VIEW),
                            200,
                            y,
                            0xFu,
                            TBLACK,
                            TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 15;
    Conquer_Clip_Text_Print(
        Text_String(TXT_CTRL_LEFT), 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_USE_C4), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print(
        Text_String(TXT_ALT_LEFT), 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_BUMP_TEAMMATE), 200, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15; // TODO, copy paste error in original?
    Map.Redraw_Objects();
}

void Show_Game_Parms(void)
{
    char buffer[500];
    int y;
    int wooden_share;

    y = 30;
    Conquer_Clip_Text_Print(
        Text_String(TXT_GAME_PARAMS_F2), 10, y, 5u, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;

    if (strcmp("Unknown", GameParams.ChannelName)) {
        sprintf(buffer, Text_String(TXT_YOU_ARE_IN_X_CHANNEL), GameParams.ChannelName);
        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    }

    wooden_share = 1000 - GameParams.Steel - GameParams.Green - GameParams.Orange;
    sprintf(buffer, Text_String(TXT_CRATE_RATIO), wooden_share, GameParams.Steel, GameParams.Green, GameParams.Orange);
    Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;

    if (GameParams.NumTeams > 0 && GameParams.TeamCrates && 60 * GameParams.TimeLimit > GameParams.ArmageddonTimer) {
        sprintf(buffer, Text_String(TXT_ARMAGEDDON_POSS));
    } else {
        sprintf(buffer, Text_String(TXT_ARMAGEDDON_IMPOSS));
    }

    Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;

    if (GameParams.Orange > 0) {
        sprintf(buffer, Text_String(TXT_ORANGE_LASTS), GameParams.SuperSeconds);
        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    }

    if (GameParams.SuperInvuln) {
        sprintf(buffer, Text_String(TXT_DONT_LOOSE_INVUL_ORANGE));
    } else {
        sprintf(buffer, Text_String(TXT_DO_LOOSE_INVUL_ORANGE));
    }

    Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;

    if (GameParams.IsLadderGame) {
        sprintf(buffer, Text_String(TXT_IS_LADDER_GAME));
    } else {
        sprintf(buffer, Text_String(TXT_ISNT_LADDER_GAME));
    }

    Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;

    if (GameParams.TimeLimit > 0) {
        sprintf(buffer, Text_String(TXT_IS_TIME_LIMIT), GameParams.TimeLimit);
    } else {
        sprintf(buffer, Text_String(TXT_NO_TIME_LIMIT));
    }

    Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;

    if (GameParams.ScoreLimit > 0) {
        sprintf(buffer, Text_String(TXT_IS_SCORE_LIMIT), GameParams.ScoreLimit);
    } else {
        sprintf(buffer, Text_String(TXT_NO_SCORE_LIMIT));
    }

    Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;

    if (GameParams.LifeLimit > 0) {
        sprintf(buffer, Text_String(TXT_IS_LIFE_LIMIT), GameParams.LifeLimit);
    } else {
        sprintf(buffer, Text_String(TXT_NO_LIFE_LIMIT));
    }

    Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;

    if (GameParams.IsCaptureTheFlag) {
        sprintf(buffer, Text_String(TXT_PLAYING_CTF));
        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;

        sprintf(buffer, Text_String(TXT_BASE_HAS_DEF_STRUCTS), GameParams.NumCTFStructures);
        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;

        if (GameParams.ResetTeamsInCTF) {
            sprintf(buffer, Text_String(TXT_TEAMS_RESET_ON_CTF));
        } else {
            sprintf(buffer, Text_String(TXT_TEAMS_DONT_RESET_ON_CTF));
        }

        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    } else if (GameParams.Football) {
        if (GameParams.FootballNumFlags == 1) {
            sprintf(buffer, Text_String(TXT_PLAYING_1F_FOOTBALL));
        } else {
            sprintf(buffer, Text_String(TXT_PLAYING_2F_FOOTBALL));
        }

        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    }

    if (GameParams.IsCrates) {
        if (GameParams.TeamCrates) {
            sprintf(buffer, Text_String(TXT_TEAMCRATES_ON));
        } else {
            sprintf(buffer, Text_String(TXT_TEAMCRATES_OFF));
        }

        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    }

    if (!OfflineMode && GameParams.IsAutoTeaming) {
        sprintf(buffer, Text_String(TXT_TEAM_SEL_AUTO));
        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    }

    if (!OfflineMode && GameParams.NumTeams) {
        sprintf(buffer, Text_String(TXT_THERE_ARE_X_TEAMS), GameParams.NumTeams);
        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    }

    if (!OfflineMode && GameParams.PlayersPerTeam != 999 && !GameParams.IsAutoTeaming) {
        sprintf(buffer, Text_String(TXT_MAX_OF_X_PER_TEAM), GameParams.PlayersPerTeam);
        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    }

    if (!OfflineMode && GameParams.AllowNoTeam != 999 && !GameParams.IsAutoTeaming) {
        if (GameParams.AllowNoTeam) {
            sprintf(buffer, Text_String(TXT_DONT_HAVE_TO_HAVE_TEAM));
        } else {
            sprintf(buffer, Text_String(TXT_MUST_HAVE_TEAM));
        }

        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    }

    if (!OfflineMode && GameParams.AllowPickTeam != 999 && !GameParams.IsAutoTeaming) {
        if (GameParams.AllowPickTeam) {
            sprintf(buffer, Text_String(TXT_MAY_CHOOSE_TEAM));
        } else {
            sprintf(buffer, Text_String(TXT_CANNOT_CHOOSE_TEAM));
        }

        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    }

    if (GameParams.IsCrates) {
        switch (GameParams.IonCannon) {
        case 0:
            sprintf(buffer, Text_String(TXT_NO_ION));
            break;
        case 1:
            sprintf(buffer, Text_String(TXT_ION_NOT_FATAL));
            break;
        case 2:
            sprintf(buffer, Text_String(TXT_ION_FATAL));
            break;
        }

        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    }

    if (GameParams.IsCrates) {
        if (GameParams.NoReshroud) {
            sprintf(buffer, Text_String(TXT_RESHROUD_OFF));
        } else {
            sprintf(buffer, Text_String(TXT_RESHROUD_ON));
        }

        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    }

    if (GameParams.IsCaptureTheFlag || GameParams.Football) {
        if (GameParams.AllowFlagSitting) {
            sprintf(buffer, Text_String(TXT_FLAG_SITTING_ALLOWED));
        } else {
            sprintf(buffer, Text_String(TXT_FLAG_SETTING_NOT_ALLOWED));
        }

        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;

        if (GameParams.LosePowerups) {
            sprintf(buffer, Text_String(TXT_FLAG_LOOSE_POWERUP));
        } else {
            sprintf(buffer, Text_String(TXT_FLAG_NOT_LOOSE_POWERUP));
        }

        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    }

    if (GameParams.FreeRadarForAll) {
        sprintf(buffer, Text_String(TXT_RADAR_IS_FREE));
    } else {
        sprintf(buffer, Text_String(TXT_RADAR_NOT_FREE));
    }

    Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;

    switch (GameParams.HealthBars) {
    case 0:
        sprintf(buffer, Text_String(TXT_HEALTH_SHOWN_FOR_YOU));
        break;
    case 1:
        sprintf(buffer, Text_String(TXT_HEALTH_SHOWN_FOR_TEAM));
        break;
    case 2:
        sprintf(buffer, Text_String(TXT_HEALTH_SHOWN_EVERYBODY));
        break;
    }

    Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;

    if (GameParams.IsMaxNumAIsScaled) {
        sprintf(buffer, Text_String(TXT_HUNTERS_MATCH_PLAYERS));
    } else {
        sprintf(buffer, Text_String(TXT_HUNTERS_NOT_MATCH_PLAYERS));
    }

    Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;

    if (!GameParams.IsMaxNumAIsScaled) {
        sprintf(buffer,
                Text_String(TXT_HUNTERS_FROM_NUMBERS),
                GameParams.AIUnitsPer10min,
                GameParams.MaxAIUnits,
                GameParams.AIBuildingsPer10min,
                GameParams.MaxAIBuildings);
        Conquer_Clip_Text_Print(buffer, 10, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
        y += 15;
    }

    Map.Redraw_Objects();
}

void Show_Keyboard_Commands(void)
{
    char str[300];
    int y = 30;
    Conquer_Clip_Text_Print(
        Text_String(TXT_ADVANCED_COMMANDS_F3), 10, y, 5u, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print("*ladder", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_VIEW_LADDER), 170, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print("*news", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(
        Text_String(TXT_DISPLAY_NEWS), 170, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15;
    Conquer_Clip_Text_Print("*verbose", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    Conquer_Clip_Text_Print(Text_String(TXT_TOGGLE_VERBOSE_MSG),
                            170,
                            y,
                            0xFu,
                            TBLACK,
                            TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 15;
    Conquer_Clip_Text_Print("*log", 10, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);

    if (MessageLogging) {
        sprintf(str, Text_String(TXT_TOGGLE_LOGGING_NOW_ON));
    } else {
        sprintf(str, Text_String(TXT_TOGGLE_LOGGING_OFF));
    }

    Conquer_Clip_Text_Print(str, 170, y, 0xFu, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 15; // TODO, copy paste error in original?
    Map.Redraw_Objects();
}

void Thanks_to_Testers(void)
{
    int y = 130;
    Conquer_Clip_Text_Print("Many thanks to our Sole Survivor Phase 1 & 2 Testers!",
                            240,
                            y,
                            YELLOW,
                            TBLACK,
                            TPF_CENTER | TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 25;
    Conquer_Clip_Text_Print("You are too numerous to list here,",
                            240,
                            y,
                            YELLOW,
                            TBLACK,
                            TPF_CENTER | TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 25;
    Conquer_Clip_Text_Print("but we couldn't have done it without you.",
                            240,
                            y,
                            YELLOW,
                            TBLACK,
                            TPF_CENTER | TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 25;
    Conquer_Clip_Text_Print("You know who you are...",
                            240,
                            y,
                            YELLOW,
                            TBLACK,
                            TPF_CENTER | TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 25;
    Conquer_Clip_Text_Print("We know who you are...",
                            240,
                            y,
                            YELLOW,
                            TBLACK,
                            TPF_CENTER | TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 25;
    Conquer_Clip_Text_Print("Let the rest find out the hard way!",
                            240,
                            y,
                            YELLOW,
                            TBLACK,
                            TPF_CENTER | TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    y += 30;
    Conquer_Clip_Text_Print(
        "Sole Survivor Team", 240, y, WHITE, TBLACK, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
    y += 25;

    Map.Redraw_Objects();
}

void Core_Team_Credits(void)
{
    Sound_Effect(VOC_XPLOBIG4);
    Map.Shake_The_Screen(50);
    Sound_Effect(VOC_BURN);

    VoiceThemes[2]->Play(VOX_THEME_SND_TEST);

    Messages.Add_Message("      --------------------- Core SOLE SURVIVOR Team -----------------------",
                         WHITE,
                         TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                         0);
    Messages.Add_Message("        Programmers: Tom Spencer-Smith, Neal Kettler, Dave Aldridge",
                         32,
                         TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                         0);
    Messages.Add_Message(
        "                           Producer: Ken Murphy", 32, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
    Messages.Add_Message(
        "                         VP/R&D: Steve Wetherill", 32, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
    Messages.Add_Message("                  -----------------------------------------------",
                         WHITE,
                         TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                         0);

    Map.Flag_To_Redraw(false);
}

#define CORE_CREDITS 0xC87AD5A4
#define STAR_ARES    0x51842BF3
#ifdef _WIN32
extern HWND MainWindow;
#endif

bool Handle_Typed_Message(char* message)
{
    char logline[300];
    char txt[200];
    char str[200];

    if (Obfuscate(message) == CORE_CREDITS) {
        Core_Team_Credits();
        return true;
    }

    if (stricmp(message, "Get me outta here!") == 0) {
        if (GameToPlay == GAME_HOST && OfflineMode) {
            Host_Send_Scenario_Change_Packet();
            return true;
        }
#ifdef _WIN32
    } else if (stricmp(message, "*ladder") == 0) {
        Hide_Mouse();
        Fade_Palette_To(BlackPalette, 0xFu, 0);
        VisiblePage.Clear();
        ShowWindow(MainWindow, 6);
        ShellExecuteA(0, 0, ButtonSixURL, 0, 0, 1);
        Show_Mouse();
        return true;
    } else if (stricmp(message, "*news") == 0) {
        Hide_Mouse();
        Fade_Palette_To(BlackPalette, 0xFu, 0);
        VisiblePage.Clear();
        ShowWindow(MainWindow, 6);
        ShellExecuteA(0, 0, "ssnews.txt", 0, 0, 1);
        Show_Mouse();
        return true;
#endif
    } else if (stricmp(message, "*verbose") == 0) {
        Options.IsVerbose = !Options.IsVerbose;

        if (Options.IsVerbose) {
            sprintf(str, Text_String(TXT_VERBOSE_ON));
        } else {
            sprintf(str, Text_String(TXT_VERBOSE_OFF));
        }

        Messages.Add_Message(str, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
        Map.Flag_To_Redraw(true);
        return true;
    } else if (stricmp(message, "*log") == 0) {
        if (!MessageLogging) {
            MessageLogging = true;
            sprintf(txt, Text_String(TXT_MSG_LOGGING_ON));
            sprintf(logline, "\r\n>>> Message logging turned ON at %s", Local_Time_As_String());
            Client_Log_Player_Message(logline);
        } else {
            sprintf(txt, Text_String(TXT_MSG_LOGGING_OFF));
            sprintf(logline, "\r\n>>> Message logging turned OFF at %s", Local_Time_As_String());
            Client_Log_Player_Message(logline);
            MessageLogging = false;
        }

        Messages.Add_Message(txt, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
        Map.Flag_To_Redraw(true);
        return true;
    } else if (Obfuscate(strtok(strdup(message), " \t")) == STAR_ARES) { // TODO Memeory leak!
        if (GameToPlay != GAME_CLIENT) {
            return true;
        }

        if (!GameParams.IsSquadChannel) {
            Messages.Add_Message(
                "This isn't a squad channel... save your breath!", 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
            Map.Flag_To_Redraw(true);
            Sound_Effect(VOC_SCOLD, VOL_FULL);
            return true;
        }

        if (60 * GameParams.TimeLimit - WDTGameTimer.Time() / 60 < 120) {
            Messages.Add_Message(
                "Please let this game finish first.", 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
            Map.Flag_To_Redraw(true);
            Sound_Effect(VOC_SCOLD, VOL_FULL);
            return true;
        }

        char* string = strtok(0, " \t");

        if (string) {
            int cmd = 0;
            int obfuscate = Obfuscate(string);
            Client_Send_Command(cmd, obfuscate);
        } else {
            Messages.Add_Message(
                "You did not give a valid password!", 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
            Map.Flag_To_Redraw(true);
            Sound_Effect(VOC_SCOLD, VOL_FULL);
        }

        return true;
    } else if (*message == '*') {
        Messages.Add_Message("No such command", 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
        Map.Flag_To_Redraw(true);
        return true;
    }

    return false;
}
