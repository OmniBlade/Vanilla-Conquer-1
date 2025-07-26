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

#ifndef QCLIENT_H
#define QCLIENT_H

void Client_Process_House_Packet(HouseUpdatePacketData* packet);            //Forward declare, move to qclient.h?
void Client_Process_Delete_Object_Packet(NewDeletePacketData* packet);      //Forward declare, move to qclient.h?
ObjectClass* Client_Process_New_Object_Packet(NewDeletePacketData* packet); //Forward declare, move to qclient.h?
void Client_Process_Health_Packet(HealthPacketData* packet);                //Forward declare, move to qclient.h?
void Client_Process_Damage_Packet(DamagePacketData* packet);                //Forward declare, move to qclient.h?
void Client_Process_Squish_Packet(SquishPacketData* packet);                //Forward declare, move to qclient.h?
void Client_Process_Capture_Packet(CapturePacketData* packet);              //Forward declare, move to qclient.h?
void Client_Process_Cargo_Packet(CargoPacketData* packet);                  //Forward declare, move to qclient.h?
void Client_Process_Flag_Packet(FlagPacketData* packet);                    //Forward declare, move to qclient.h?
void Client_Process_CTF_Packet(CTFPacketData* packet);                      //Forward declare, move to qclient.h?
void Client_Process_Movement_Packet(MovePacketData* packet);                //Forward declare, move to qclient.h?
void Client_Process_Target_Packet(TargetPacketData* packet);                //Forward declare, move to qclient.h?
void Client_Process_Fire_At_Packet(FireAtPacketData* packet);               //Forward declare, move to qclient.h?
void Client_Process_Do_Turn_Packet(DoTurnPacketData* packet);               //Forward declare, move to qclient.h?
void Client_Process_Techno_Packet(TechnoClass* obj,
                                  TechnoPacketDataType type,
                                  unsigned char value);  //Forward declare, move to qclient.h?
void Client_Process_Change_Scenario(int scenario_index); //Forward declare, move to qclient.h?
bool Client_Process_Packet(int connectiontype);
void Client_Process_Message(void);
void Client_Log_Player_Message(char*);
void Client_Comm_Movement(FootClass* obj, CELL cell);
void Client_Send_Event_Packet(void);
void Client_Send_Command(int, int);
void Show_Quote(void);

#endif // QCLIENT_H
