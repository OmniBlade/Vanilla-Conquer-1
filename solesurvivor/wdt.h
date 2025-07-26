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

#ifndef WDT_H
#define WDT_H

#include "defines.h"
typedef struct PlayerNameTag
{
    DynamicVectorClass<TechnoClass*> Technos;
    unsigned char Address[32]; //struct?
    char Name[MPLAYER_NAME_MAX];
    HousesType House;
    HouseClass* HousePtr;
    RTTIType RTTI;
    int Type;
    int NumStartingUnits;
    unsigned UnknownBool : 1;
    int SomeTiming;
    float SomeScore;
} PlayerNodeType;

int Calculate_Points(HousesType player);
void Remove_All_Crates(void);
bool Overlay_Is_Crate(OverlayType);
void Decode_Credits_Entry(int entry, char* str);
void Secret_Credits_Dialog(void);
void Client_Handle_Sight();
bool Client_Process_Initial_Game_State_Packet(int size);
void CommStats_Set_Frame_Rate(int serverfps, int clientfps);
void Add_WDT_Radar();
void Remove_WDT_Radar(void);
void Client_Disconnect(void);
void Host_Disconnect(void);
void Update_CTF_Tracking(void);
void Update_Tracking(void);
void Enable_Crates(void);
void Disable_Crates(void);
const char *Get_Game_Duration_As_String(void);
int Sum_Object_Stats(ObjectClass *object);
void Show_Key_Commands(void);
void Show_Game_Parms(void);
void Show_Keyboard_Commands(void);
void Thanks_to_Testers(void);
float Armageddon_Probability(int* highest_team, int* lowest_team);
int Host_Init_Listener();
int Host_Pick_Random_Map();
bool Client_Connect_To_IP(const char* host);
void Clear_Packet_Data_Vectors(void);
TechnoClass* Create_AI_Unit(void);
BuildingClass* Create_Building(bool dont_rand = false, HousesType house = HOUSE_NONE, HousesType gdi_or_nod = HOUSE_NONE);
void CommStats_Set_Transmission_Stats(int rec_bytes_sec,
                                      int sent_bytes_sec,
                                      int sent_tcp,
                                      int sent_udp,
                                      int recieved_tcp,
                                      int recieved_udp);
                                      
 // From WDTDLG.CPP
void Victory_Dialog(HousesType team);
void Select_Voice_Dialog(void);
bool Unit_Choice_Dialog(void);
bool Client_Wait_For_WDT_Connection(void);
void Draw_Choice_Entry(RTTIType rtti, int type, int xpos, int ypos, HousesType house, int unused, bool draw_name);

// From solehelp.cpp
void Help_Menu(void);

#endif /* WDT_H */