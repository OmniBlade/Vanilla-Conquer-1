#include "function.h"
#include <comms.h>
#include "gprotocol.h"
#include <limits.h>
#include <time.h>
#include <winnt.h>

int LastScoreCalcTime;
float CalculatedScore;
unsigned int IdleTime;
int PlayerCount;
int RecievesProcessed_5AADB0;
int RecievesProcessed_5AADB4;
int SquadPassword;
char AcceptedList[20][MULTI_HOUSE_COUNT];
bool ShowAcceptedList;
extern CountDownTimerClass FramerateUpdateTimer;
extern CountDownTimerClass TransmisionStatsTimer;
extern int PacketLength[PACKET_COUNT];
extern int RecievedBytesSec;
extern int SentBytesSec;
extern int SentTCP;
extern int SentUDP;
extern int RecievedTCP;
extern int RecievedUDP;
extern int ClientFPS;
extern int LastClientFrame;
extern TechnoClass *TechnoThatGotStealthCrate;
extern bool ShowServerDialog;
extern HWND ServerDlg;
extern void Host_Disconnect(void);
extern TimerClass WDTGameTimer;
extern HWND Create_Server_Window(void);
extern void Queue_AI_Normal();
extern void Host_Send_Health_Packet(int index);
extern bool Host_Send_Damage_Packet(int index);
extern void Host_Remove_Player(int index);
extern bool Host_Send_New_Delete_Packet(int index);
extern bool Host_Send_Cargo_Packet(int index);
extern bool Host_Send_Flag_Packet(int index);
extern bool Host_Send_CTF_Packet(int index);
extern void Host_Send_Target_Packet(int index);
extern void Host_Send_Move_Packet(int index);
extern void Host_Send_Fire_At_Packet(int index);
extern void Host_Send_Do_Turn_Packet(int index);
bool Host_Send_Crate_Packet(int index); // Forward declare from this file.
extern bool Host_Send_Per_Cell_Process_Packet(int index);
bool Host_Send_Per_Cell_Process_Packet_Offline(void); // Forward declare from this file.
extern bool Host_Send_Techno_Packet(int index);
extern void Host_Process_Remote_Admin_Packet(int index);
bool Host_Send_Player_Leave_Packet_To_All(HousesType house); // Forward declare from this file.
bool Host_Comm_Receive(int index);  // Forward declare from this file.
void Host_Fortify_Home_Of(HousesType house);  // Forward declare from this file.
void Host_Send_Spectator_Packet_To_All(int index); // Forward declare from this file.
bool Host_Make_Player_Unit(int player_index); // Forward declare from this file.
void Host_Send_Framerate_Packet(int index); // Forward declare from this file.
extern void Clear_Packet_Data_Vectors();
extern MainMenuSelection MenuSelection; // From INIT.CPP
extern void Disable_Crates(void); // From WDT.CPP
extern const char *Local_Time_As_String(void); // From WDT.CPP
void CommStats_Set_Transmission_Stats(int rec_bytes_sec, int sent_bytes_sec, int sent_tcp, int sent_udp, int recieved_tcp, int recieved_udp); // From WDT.CPP
extern void Create_Offline_Unit(void); // From SINGLEP.CPP

// Matching
void Host_Queue_AI()
{
	int index;
	int nd_index;
	HousesType house2;

	if (Frame == 0) {
		if (ShowServerDialog && !OfflineMode) {
			Set_Palette(BlackPalette);
			VisiblePage.Clear();
			ShowWindow(MainWindow, 6);
			ShowWindow(MainWindow, 0);

			if (!ServerDlg) {
				Create_Server_Window();
				ShowCursor(TRUE);
			}
		}

		WDTGameTimer.Set(0, true);
		TechnoThatGotStealthCrate = NULL;
	}
	
	for (index = 0; index < ActivePlayers.Count(); index++) {
		if (ReliableProtocols[index]->ConnectionState == 0) {
			house2 = ActivePlayers[index]->HousePtr->Class->House;
			Host_Remove_Player(index);
			Host_Send_Player_Leave_Packet_To_All(house2);
			index--;

			if (ActivePlayers.Count() <= 1) {
				Disable_Crates();
			}
		}
	}
	
	if (ActivePlayers.Count() == 0) {
		if (PlayerCount > 2 && time(0) - IdleTime > 900 || time(0) - IdleTime > 7200) {
			char buff[200]; // [esp+24h] [ebp-D4h]
			sprintf(buff, "Killing server after %d minutes idleness at time %s.\n", (time(0) - IdleTime) / 60, Local_Time_As_String());
			CCDebugString(buff);
			GameActive = 0;
			MenuSelection = SEL_EXIT;
		}
	} else {
		if (ActivePlayers.Count() > PlayerCount) {
			PlayerCount = ActivePlayers.Count();
		}

		IdleTime = time(0);
	}

	RecievesProcessed_5AADB0 = 0;

	for (index = 0; index < ActivePlayers.Count(); index++) {
		while (ReliableProtocols[index]->Queue->Num_Receive() > 0) {
			Host_Comm_Receive(index);
			++RecievesProcessed_5AADB0;
		}
	}

	if (RecievesProcessed_5AADB0 > 0) {
		++RecievesProcessed_5AADB4;
	}

	Queue_AI_Normal();
	
	if (HealthPacketDatas.Count()) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			Host_Send_Health_Packet(index);
		}
	}

	if (DamagePacketDatas.Count()) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			if (!Host_Send_Damage_Packet(index)) {
				Host_Remove_Player(index);
				index--;
			}
		}
	}

	if (NewDeletePacketDatas.Count()) {	
		for (index = 0; index < ActivePlayers.Count(); index++) {
			if (!ActivePlayers[index]->UnknownBool) {
				if(!Host_Send_New_Delete_Packet(index)) {
					Host_Remove_Player(index);
					index--;
				}
			}	
		}
	}

	if (CargoPacketDatas.Count()) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			if (!Host_Send_Cargo_Packet(index)) {
				Host_Remove_Player(index);
				index--;
			}	
		}
	}

	if (FlagPacketDatas.Count()) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			if (!Host_Send_Flag_Packet(index)) {
				Host_Remove_Player(index);
				index--;
			}			
		}
	}

	if (CTFPacketDatas.Count()) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			if (!Host_Send_CTF_Packet(index)) {
				Host_Remove_Player(index);
				index--;
			}			
		}
	}

	if (MovePacketDatas.Count()) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			Host_Send_Move_Packet(index);
		}
	}
	
	if (TargetPacketDatas.Count()) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			Host_Send_Target_Packet(index);
		}
	}
	
	if (FireAtPacketDatas.Count()) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			Host_Send_Fire_At_Packet(index);
		}
	}
	
	if (DoTurnPacketDatas.Count()) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			Host_Send_Do_Turn_Packet(index);
		}
	}
	
	if (CratePacketDatas.Count()) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			if (!Host_Send_Crate_Packet(index)) {
				Host_Remove_Player(index);
				index--;
			}			
		}
	}
	
	if (PerCellPacketDatas.Count()) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			if (!Host_Send_Per_Cell_Process_Packet(index)) {
				Host_Remove_Player(index);
				index--;
			}			
		}
	}
	
	if (OfflineMode) {
		if (PerCellPacketDatas.Count()) {
			Host_Send_Per_Cell_Process_Packet_Offline();
		}
	}

	if (TechnoPacketDatas.Count()) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			if (!Host_Send_Techno_Packet(index)) {
				Host_Remove_Player(index);
				index--;
			}			
		}
	}
	
	Clear_Packet_Data_Vectors();

	for (index = 0; index < ActivePlayers.Count(); index++) {
		ActivePlayers[index]->UnknownBool = false;			
	}
	
	if (GameParams.IsCaptureTheFlag && !GameParams.ResetTeamsInCTF) {
		for (HousesType house1 = HOUSE_BLUE_TEAM; house1 <= HOUSE_GREY_TEAM; house1++) {
			HouseClass *hc = HouseClass::As_Pointer(house1);
			if (hc->IsUnk1 && hc->Timer1.Expired()) {
				Host_Fortify_Home_Of(house1);
			}
		}
	}

	if (OfflineMode && PlayerPtr->IsDefeated && PlayerPtr->Timer1.Expired()) {
		PlayerPtr->IsDefeated = false;
		Create_Offline_Unit();
		++PlayerPtr->Int1;
	}

	for (index = 0; index < ActivePlayers.Count(); index++) {
		if (ActivePlayers[index]->HousePtr->IsDefeated &&
			ActivePlayers[index]->HousePtr->Timer1.Expired() &&
			ActivePlayers[index]->HousePtr->Class->House != HOUSE_SPECTATOR &&
			ActivePlayers[index]->HousePtr->Class->House != HOUSE_ADMIN &&
			!ActivePlayers[index]->HousePtr->IsVisionary) {
				if (GameParams.LifeLimit > 0 && ActivePlayers[index]->HousePtr->Int1 >= GameParams.LifeLimit ) {
					Host_Send_Spectator_Packet_To_All(index);
					ActivePlayers[index]->HousePtr->IsVisionary = true;
					ActivePlayers[index]->HousePtr->IsUnk2 = true;
				} else {
					if (GameParams.IsCaptureTheFlag && GameParams.ResetTeamsInCTF) {
						Host_Fortify_Home_Of(ActivePlayers[index]->HousePtr->ActLike);
					}

					ActivePlayers[index]->HousePtr->IsDefeated = false;
					Host_Make_Player_Unit(index);
					ActivePlayers[index]->HousePtr->Int1++;

					for (nd_index = 0; nd_index < NewDeletePacketDatas.Count(); nd_index++) {
						delete NewDeletePacketDatas[nd_index];
					}

					NewDeletePacketDatas.Delete_All();
				}
		}	
	}

	if (!FramerateUpdateTimer.Time()){
		ClientFPS = (Frame - LastClientFrame) / 2;
		LastClientFrame = Frame;

		for (index = 0; index < ActivePlayers.Count(); index++) {
			Host_Send_Framerate_Packet(index);
		}

		FramerateUpdateTimer.Set(120, true);
	}
	
	if (!TransmisionStatsTimer.Time()) {
		CommStats_Set_Transmission_Stats(RecievedBytesSec / 2, SentBytesSec / 2, SentTCP / 2, SentUDP / 2, RecievedTCP / 2, RecievedUDP / 2);
		RecievedBytesSec = 0;
		SentBytesSec = 0;
		SentTCP = 0;
		SentUDP = 0;
		RecievedTCP = 0;
		RecievedUDP = 0;
		TransmisionStatsTimer.Set(120, true);
	}
	
	for (index = 0; index < RemoteAdminsComms.Count(); index++) {
		while (RemoteAdminsProtocols[index]->Queue->Num_Receive() > 0) {
			Host_Process_Remote_Admin_Packet(index);
		}
	}
	
	for (index = 0; index < RemoteAdminsComms.Count(); index++) {
		if (!RemoteAdminsProtocols[index]->ConnectionState) {
			RemoteAdminsComms[index]->Disconnect();
			delete RemoteAdminsComms[index];
			delete RemoteAdminsProtocols[index];
			RemoteAdminsComms.Delete(index);
			RemoteAdminsProtocols.Delete(index);
			index--;
		}
	}
}

extern bool Host_Wait_Client(ReliableProtocolClass *protocol, char *name);

// Matching
bool Host_Send_Player_Objects_Packet(int player_index)
{
	int count;

	count = ActivePlayers[player_index]->Technos.Count();

	if (count == 0) {
		return true;
	}

	int index;
	WDTPacketStruct packet;
	int current;
	int max;
	int bytes;

	current = 0;
	max = 50; //sizeof(packet.PlayerJoin.Objects) / sizeof(packet.PlayerJoin.Objects[0]);
	packet.Header.Type = PACKET_PLAYER_UNITS;

	for (index = 0; index < count; index++) {
		packet.PlayerJoin.Objects[index] = ActivePlayers[player_index]->Technos[index]->As_Target();
		current++;

		/*
		** If we have the max our packet can handle, we send it.
		*/
		if (current >= max) {
			if (!Host_Wait_Client(ReliableProtocols[player_index], ActivePlayers[player_index]->HousePtr->Name)) {
				return false;
			}

			packet.PlayerJoin.Count = (unsigned char)current;
			bytes = PacketLength[PACKET_PLAYER_UNITS] - 200 + 4 * current;
			ReliableProtocols[player_index]->Queue->Queue_Send(&packet, bytes);
			ReliableComms[player_index]->Send();
			SentBytesSec += bytes;
			SentTCP++;
			current = 0;
		}
	}

	packet.PlayerJoin.Count = (unsigned char)current;

	/*
	** If we have remaining data, we send it.
	*/
	if (packet.PlayerJoin.Count != (unsigned char)0) {
		if (!Host_Wait_Client(ReliableProtocols[player_index], ActivePlayers[player_index]->HousePtr->Name)) {
			return false;
		}

		bytes = PacketLength[PACKET_PLAYER_UNITS] - 200 + 4 * current;
		ReliableProtocols[player_index]->Queue->Queue_Send(&packet, bytes);
		ReliableComms[player_index]->Send();
		SentBytesSec += bytes;
		SentTCP++;
	}

	return true;
}

bool Make_Player_Unit(int player_index);
bool Host_Make_Player_Unit(int player_index)
{
	int index;

	ActivePlayers[player_index]->Technos.Clear();
	Make_Player_Unit(player_index);
	
	if (NewDeletePacketDatas.Count()) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			if (!Host_Send_New_Delete_Packet(index)) {
				Host_Remove_Player(index);
				index--;
			}
		}
	}

	Host_Send_Player_Objects_Packet(player_index);
	return true;
}

void Setup_House_Flags(HousesType house);
void Setup_Flags_Of_House(HousesType house);
void Fortify_Flag_Home(HousesType house);

void Host_Fortify_Home_Of(HousesType house)
{
	HouseClass *hptr;
	int team = house - 6;
	if ( team >= 0 && team <= 3 )
	{
		hptr = HouseClass::As_Pointer(house);
		if ( hptr->IsUnk1)
		{
			Setup_Flags_Of_House(house);
			Fortify_Flag_Home(house);
			hptr->IsUnk1 = false;
		}
	}
}

extern GameResultClass ServerGameResults; // GLOBALS.CPP
extern void Clear_Flags_Of_House(HousesType house); // INI.CPP
extern void Clear_Buildings_Of_House(HousesType house); // INI.CPP
float Host_Calculate_Total_Score(void); // Forward declare
int Host_Comm_Calc_Score(int index); // Forward Declare

// Matching
void Host_Remove_Player(int player_idx)
{
	int index;
	TechnoClass *tptr;
	int is_ally;
	HouseClass *hptr;
	GameResultClass::GameResultEntry player_results;

	hptr = NULL;
	index = player_idx;
	memset(&player_results, 0, sizeof(player_results));
	hptr = ActivePlayers[player_idx]->HousePtr;

	/*
	** If we have a valid player house, save their score before they are removed.
	*/
	if (hptr != NULL &&
		hptr->Class != NULL &&
		hptr->Class->House != HOUSE_SPECTATOR &&
		hptr->Class->House != HOUSE_ADMIN) {
		strcpy(player_results.Name, ActivePlayers[index]->Name);
		player_results.ScoredPoints = ActivePlayers[index]->HousePtr->Int2;
		player_results.TotalDeaths = ActivePlayers[index]->HousePtr->Int3;
		player_results.PlayerCount = ActivePlayers[index]->SomeTiming;
		player_results.TimeIngame = time(0) - ActivePlayers[index]->SomeTiming;
		player_results.timing3_score = Host_Calculate_Total_Score();
		player_results.dword1C_score = Host_Comm_Calc_Score(index);
		
		if (ActivePlayers[index]->RTTI == RTTI_INFANTRY || ActivePlayers[index]->RTTI == RTTI_INFANTRYTYPE) {
			player_results.chosentype = (short)ActivePlayers[index]->Type + 100;
		} else {
			player_results.chosentype = ActivePlayers[index]->Type;
		}

		player_results.IsWinnerOrLoser = 0;
		ServerGameResults.Push_Result(&player_results);
	}

	/*
	** Handle if the player was the last member of a team in CTF/Football
	*/
	hptr = ActivePlayers[player_idx]->HousePtr;

	if (GameParams.IsCaptureTheFlag && hptr != NULL) {
		is_ally = false;

		for (index = 0; index < ActivePlayers.Count(); index++) {
			if (index != player_idx) {
				if (ActivePlayers[index]->HousePtr->Is_Ally(hptr)) {
					is_ally = true;
				}
			}
		}

		if (!is_ally) {
			Clear_Flags_Of_House(hptr->ActLike);
			Clear_Buildings_Of_House(hptr->ActLike);
		}
	}

	if (GameParams.Football && hptr != NULL) {
		is_ally = false;

		for (index = 0; index < ActivePlayers.Count(); index++) {
			if (index != player_idx) {
				if (ActivePlayers[index]->HousePtr->Is_Ally(hptr)) {
					is_ally = true;
				}
			}
		}

		if (!is_ally) {
			Clear_Flags_Of_House(hptr->ActLike);
		}
	}

	/*
	** Remove player objects, we are the server so no delete allowed checks.
	*/
	for (index = 0; index < ActivePlayers[player_idx]->Technos.Count(); index++) {
		tptr = ActivePlayers[player_idx]->Technos[index];
		tptr->Stun();
		DELETE_OBJ(tptr, sizeof(AbstractClass));
	}

	ActivePlayers[player_idx]->Technos.Clear();
	
	if (ActivePlayers[player_idx]->HousePtr != NULL) {
		ActivePlayers[player_idx]->HousePtr->IsHuman = false;
	}

	if (ActivePlayers[player_idx]->HousePtr != NULL) {
		Map.Color_List_Remove_Player(ActivePlayers[player_idx]->HousePtr->Name);
	}

	/*
	** Finally, delete the players comm objects.
	*/
	ReliableComms[player_idx]->Disconnect();
	
	delete ActivePlayers[player_idx];
	delete ReliableComms[player_idx];
	delete ReliableProtocols[player_idx];
	
	ActivePlayers.Delete(player_idx);
	ReliableComms.Delete(player_idx);
	ReliableProtocols.Delete(player_idx);
}

extern int RecievedBytesSec;
extern int RecievedTCP;
extern void Host_Extract_Compressed_Events_From_Packet(WDTPacketStruct *packet);
extern void Host_Process_Message_Packet(WDTPacketStruct *packet);
extern void Host_Process_Command_Packet(WDTPacketStruct *packet, int index);

bool Host_Comm_Receive(int index)
{
	WDTPacketStruct *packet;
	int unused;
	CommBufferClass *queue;

	//bug seemgly checking connection type......
	if (index != -1) {
		unused = index;
		queue = ReliableProtocols[index]->Queue;
		packet = (WDTPacketStruct *)queue->Get_Receive(0)->Buffer;
		RecievedBytesSec += queue->Get_Receive(0)->BufLen;
		RecievedTCP++;
	}

	switch(packet->Header.Type) {
		case PACKET_EVENT:
			Host_Extract_Compressed_Events_From_Packet(packet);
			break;
			
		case PACKET_MESSAGE:
			Host_Process_Message_Packet(packet);
			break;
			
		case PACKET_COMMAND_MESSAGE:
			Host_Process_Command_Packet(packet, index);
			break;
	}

	queue->UnQueue_Receive(0, 0, 0);
	
	return true;
}

static int Host_Extract_Compressed_Events(void *buf, int bufsize);
void Host_Extract_Compressed_Events_From_Packet(WDTPacketStruct *packet)
{
	int num = 0;
	num = Host_Extract_Compressed_Events(packet->Event.Data, packet->Event.Size);
}

/***************************************************************************
 * Extract_Compressed_Events -- extracts events from a packet   				*
 *                                                                         *
 * INPUT:                                                                  *
 *		buf			buffer containing events to extract								*
 *		bufsize		length of 'buf'														*
 *                                                                         *
 * OUTPUT:                                                                 *
 *		# events extracted																	*
 *                                                                         *
 * WARNINGS:                                                               *
 *		none.																						*
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 DRD : Created.                                             *
 *=========================================================================*/
static int Host_Extract_Compressed_Events(void *buf, int bufsize)
{
	int pos = 0;						// current buffer parsing position
	int leftover = bufsize;			// # bytes left to process
	EventClass *event;				// event ptr for parsing buffer
	int count = 0;						// # events processed
	int datasize = 0;					// size of data to copy
	EventClass eventdata;			// stores Frame, ID, etc
	unsigned char numunits = 0;	// # units stored in compressed MegaMissions
//int lasteventtype=0;
	//------------------------------------------------------------------------
	// Clear work event structure
	//------------------------------------------------------------------------
	memset (&eventdata, 0, sizeof(EventClass));

	//------------------------------------------------------------------------
	// Assume the first event is a FRAMEINFO event
	// Init 'datasize' to the amount of data to copy, minus the EventType value
	// For the 1st packet only, this will include all info before the Data
	// union, plus the size of the FrameInfo structure, minus the EventType size.
	//------------------------------------------------------------------------
	datasize = (offsetof(EventClass, Data)); //+
		//size_of(EventClass, Data.FrameInfo)) - sizeof(EventClass::EventType);
	event = (EventClass *)(((char *)buf) + pos);

	while ((unsigned)leftover >= (datasize + sizeof(EventClass::EventType)) ) {
		//.....................................................................
		// add event to the DoList, only if it's not a FRAMESYNC
		// (but FRAMEINFO's do get added.)
		//.....................................................................
		if (event->Type != EventClass::FRAMESYNC) {
			//..................................................................
			// initialize the common data from the FRAMEINFO event
			// keeping IsExecuted 0
			//..................................................................
			if (event->Type == EventClass::FRAMEINFO) {
				eventdata.ID = event->ID;

				//...............................................................
				// Adjust position past the common data
				//...............................................................
				pos += (offsetof(EventClass, Data) -
						 sizeof(EventClass::EventType));
				leftover -= (offsetof(EventClass, Data) -
								sizeof(EventClass::EventType));
			}
			//..................................................................
			// if MEGAMISSION event get the number of units (events to generate)
			//..................................................................
			else if (event->Type == EventClass::MEGAMISSION) {
				numunits = *(((unsigned char *)buf) + pos + sizeof(eventdata.Type));
				pos += sizeof(numunits);
				leftover -= sizeof(numunits);
			}

			//..................................................................
			// clear the union data portion of the event
			//..................................................................
			memset (&eventdata.Data, 0, sizeof(eventdata.Data));
			eventdata.Type = event->Type;
			datasize = EventClass::EventLength[ eventdata.Type ];

			switch (eventdata.Type) {
				case (EventClass::RESPONSE_TIME):
					break;

				case (EventClass::MEGAMISSION):
					memcpy ( &eventdata.Data.MegaMission,
						((char *)buf) + pos + sizeof(EventClass::EventType),
						datasize );

					if (numunits > 1) {
						pos += (datasize + sizeof(EventClass::EventType));
						leftover -= (datasize + sizeof(EventClass::EventType));
						datasize = sizeof(eventdata.Data.MegaMission.Whom);

						while (numunits) {
							if ( !DoList.Add( eventdata ) ) {
								return (-1);
							}

							//......................................................
							// Keep count of how many events we add to the queue
							//......................................................
							count++;
							numunits--;
							memcpy ( &eventdata.Data.MegaMission.Whom,
								((char *)buf) + pos, datasize );

							//......................................................
							// if one unit left fall thru to normal code
							//......................................................
							if (numunits == 1) {
								datasize -= sizeof(EventClass::EventType);
								break;
							}
							else {
								pos += datasize;
								leftover -= datasize;
							}
						}
					}
					break;

				default:
					memcpy ( &eventdata.Data,
						((char *)buf) + pos + sizeof(EventClass::EventType),
						datasize );
					break;
			}


			if ( !DoList.Add( eventdata ) ) {
				return (-1);
			}

			//..................................................................
			// Keep count of how many events we add to the queue
			//..................................................................
			count++;

			pos += (datasize + sizeof(EventClass::EventType));
			leftover -= (datasize + sizeof(EventClass::EventType));

			if (leftover) {
				event = (EventClass *)(((char *)buf) + pos);
				datasize = EventClass::EventLength[ event->Type ];
				if (event->Type == EventClass::MEGAMISSION) {
					datasize += sizeof(numunits);
				}
			}
		}
		//.....................................................................
		// FRAMESYNC event: This >should< be the only event in the buffer,
		// and it will be uncompressed.
		//.....................................................................
		else {
			pos += (datasize + sizeof(EventClass::EventType));
			leftover -= (datasize + sizeof(EventClass::EventType));
			event = (EventClass *)(((char *)buf) + pos);

			//..................................................................
			// size of FRAMESYNC event - EventType size
			//..................................................................
			datasize = (offsetof(EventClass, Data));// +
							//size_of(EventClass, Data.FrameInfo)) -
							//sizeof(EventClass::EventType);
		}
	}

	return (count);

}	// end of Extract_Compressed_Events

extern unsigned char Scale_Value_Down(int div, int mult);
extern char TempPacketBuffer[100000];

bool Host_Send_Game_State_Packet(int index)
{
	int i;
	WDTPacketStruct packet;
	GameStatePacketData *data;
	int num;
	int max;
	InfantryClass *iptr;
	BuildingClass *bptr;
	AircraftClass *aptr;
	UnitClass *uptr;
	int count;
	int size;

	data = (GameStatePacketData *)TempPacketBuffer;
	num = 0;

	for (i = 0; i < Infantry.Count(); i++) {
		iptr = Infantry.Ptr(i);

		data[num].Whom = iptr->As_Target();
		
		if (iptr->IsInLimbo) {
			data[num].Coord = -1;
		} else {
			data[num].Coord = iptr->Coord;
		}
		
		data[num].Health = iptr->Strength;
		data[num].Owner = iptr->Owner();
		data[num].Type = iptr->Class->Type;
		data[num]._bit_C = iptr->IsCloakable;
		data[num].Strength = Scale_Value_Down(sole_array[SOLE_ARRAY_STRENGTH][2], iptr->Mod1);
		data[num].Speed = Scale_Value_Down(sole_array[SOLE_ARRAY_SPEED][2], iptr->Mod2);
		data[num].Damage = Scale_Value_Down(sole_array[SOLE_ARRAY_DAMAGE][2], iptr->Mod3);
		data[num].ROF = Scale_Value_Down(sole_array[SOLE_ARRAY_ROF][2], iptr->Mod4);
		data[num].Range = Scale_Value_Down(sole_array[SOLE_ARRAY_RANGE][2], iptr->Mod5);
		num++;		
	}
	
	for (i = 0; i < Units.Count(); i++) {
		uptr = Units.Ptr(i);

		data[num].Whom = uptr->As_Target();
		
		if (uptr->IsInLimbo) {
			data[num].Coord = -1;
		} else {
			data[num].Coord = uptr->Coord;
		}

		data[num].Health = uptr->Strength;
		data[num].Owner = uptr->Owner();
		data[num].Type = uptr->Class->Type;
		data[num]._bit_C = uptr->IsCloakable;
		data[num].Strength = Scale_Value_Down(sole_array[SOLE_ARRAY_STRENGTH][2], uptr->Mod1);
		data[num].Speed = Scale_Value_Down(sole_array[SOLE_ARRAY_SPEED][2], uptr->Mod2);
		data[num].Damage = Scale_Value_Down(sole_array[SOLE_ARRAY_DAMAGE][2], uptr->Mod3);
		data[num].ROF = Scale_Value_Down(sole_array[SOLE_ARRAY_ROF][2], uptr->Mod4);
		data[num].Range = Scale_Value_Down(sole_array[SOLE_ARRAY_RANGE][2], uptr->Mod5);
		num++;
	}
	
	for (i = 0; i < Buildings.Count(); i++) {
		bptr = Buildings.Ptr(i);

		data[num].Whom = bptr->As_Target();
		
		if (bptr->IsInLimbo) {
			data[num].Coord = -1;
		} else {
			data[num].Coord = bptr->Coord;
		}
		
		data[num].Health = bptr->Strength;
		data[num].Owner = bptr->Owner();
		data[num].Type = bptr->Class->Type;
		data[num]._bit_C = bptr->IsCloakable;
		data[num].Strength = Scale_Value_Down(sole_array[SOLE_ARRAY_STRENGTH][2], bptr->Mod1);
		data[num].Speed = Scale_Value_Down(sole_array[SOLE_ARRAY_SPEED][2], bptr->Mod2);
		data[num].Damage = Scale_Value_Down(sole_array[SOLE_ARRAY_DAMAGE][2], bptr->Mod3);
		data[num].ROF = Scale_Value_Down(sole_array[SOLE_ARRAY_ROF][2], bptr->Mod4);
		data[num].Range = Scale_Value_Down(sole_array[SOLE_ARRAY_RANGE][2], bptr->Mod5);		
		num++;
	}
	
	for (i = 0; i < Aircraft.Count(); i++) {
		aptr = Aircraft.Ptr(i);

		data[num].Whom = aptr->As_Target();
		
		if (aptr->IsInLimbo) {
			data[num].Coord = -1;
		} else {
			data[num].Coord = aptr->Coord;
		}
		
		data[num].Health = aptr->Strength;
		data[num].Owner = aptr->Owner();
		data[num].Type = aptr->Class->Type;
		data[num]._bit_C = aptr->IsCloakable;
		data[num].Strength = Scale_Value_Down(sole_array[SOLE_ARRAY_STRENGTH][2], aptr->Mod1);
		data[num].Speed = Scale_Value_Down(sole_array[SOLE_ARRAY_SPEED][2], aptr->Mod2);
		data[num].Damage = Scale_Value_Down(sole_array[SOLE_ARRAY_DAMAGE][2], aptr->Mod3);
		data[num].ROF = Scale_Value_Down(sole_array[SOLE_ARRAY_ROF][2], aptr->Mod4);
		data[num].Range = Scale_Value_Down(sole_array[SOLE_ARRAY_RANGE][2], aptr->Mod5);
		num++;
	}

	if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
		return false;
	}
	packet.Header.Type = PACKET_GAME_STATE_START;
	packet.GameState.Size = sizeof(*data) * num;
	ReliableProtocols[index]->Queue->Queue_Send(&packet, PacketLength[PACKET_GAME_STATE_START]);
	ReliableComms[index]->Send();	
	SentBytesSec += PacketLength[PACKET_GAME_STATE_START];
	SentTCP++;
	
	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_GAME_STATE;
	
	for (i = 0; i < num; i++) {
		packet.GameState.Data[count] = data[i];
		count++;
		if (count >= max) {
			if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
				return false;
			}
			packet.GameState.Size = sizeof(*data) * count;
			//results in 0xC6 needs 0xC8...
			//size = (PacketLength[PACKET_GAME_STATE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			size = (PacketLength[PACKET_GAME_STATE] - 0xC8) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;
			count = 0;
		}
	}
	packet.GameState.Size = sizeof(*data) * count;
	if (packet.GameState.Size) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			return false;
		}
		//results in 0xC6 needs 0xC8...
		//size = (PacketLength[PACKET_GAME_STATE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
		size = (PacketLength[PACKET_GAME_STATE] - 0xC8) + sizeof(*data) * count;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}

	if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
		return false;
	}
	
	packet.Header.Type = PACKET_GAME_STATE_DONE;
	ReliableProtocols[index]->Queue->Queue_Send(&packet, PacketLength[PACKET_GAME_STATE_DONE]);
	ReliableComms[index]->Send();
	SentBytesSec += PacketLength[PACKET_GAME_STATE_DONE];
	SentTCP++;
	return true;
}

bool Host_Send_Houses_Update_Packet(int index)
{
	HouseUpdatePacketData *data;
	HouseClass *hptr;
	WDTPacketStruct packet;
	int count;
	int max;
	int size;
	HousesType i;

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_HOUSE_UPDATE;

	for (i = HOUSE_FIRST_TEAM; i < HOUSE_COUNT; i++) {
		hptr = HouseClass::As_Pointer(i);
		if (i > HOUSE_LAST_TEAM && !hptr->IsHuman) {
			continue;
		}
		
		data = &packet.HouseUpdate.Data[count];
		strcpy(data->PlayerName, hptr->Name);
		data->ActLike = hptr->ActLike;
		data->House = i;
		data->Credits = hptr->Credits;
		data->Color = hptr->RemapColor;
		data->ScoredPoints = hptr->Int2;
		data->TotalDeaths = hptr->Int3;
		data->IsVisionary = hptr->IsVisionary;
		data->FlagLocation = hptr->FlagLocation;
		data->FlagHome = hptr->FlagHome;			
		count++;
		if (count >= max) {
			if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
				return false;
			}
			packet.HouseUpdate.Count = count;
			size = (PacketLength[PACKET_HOUSE_UPDATE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;
			count = 0;
		}
	}

	packet.HouseUpdate.Count = count;
	if (packet.HouseUpdate.Count) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			return false;
		}
		size = (PacketLength[PACKET_HOUSE_UPDATE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}

	return true;
}


bool Host_Send_House_Update_Packet(HousesType house)
{
	int size;
	int i;
	HouseUpdatePacketData *data;
	WDTPacketStruct packet;
	HouseClass *hptr;

	hptr = HouseClass::As_Pointer(house);
	if (!hptr || !hptr->IsHuman) {
		return true;
	}

	packet.Header.Type = PACKET_HOUSE_UPDATE;
	
	data = packet.HouseUpdate.Data;
	packet.HouseUpdate.Count = 1;
	
	strcpy(data[0].PlayerName, hptr->Name);
	data[0].ActLike = hptr->ActLike;
	data[0].House = house;
	data[0].Credits = hptr->Credits;
	data[0].Color = hptr->RemapColor;
	data[0].ScoredPoints = hptr->Int2;
	data[0].TotalDeaths = hptr->Int3;
	data[0].IsVisionary = hptr->IsVisionary;
	data[0].FlagLocation = hptr->FlagLocation;
	data[0].FlagHome = hptr->FlagHome;
	
	for (i = 0; i < ActivePlayers.Count(); i++) {
		if (ActivePlayers[i]->HousePtr != hptr) {
			if (!Host_Wait_Client(ReliableProtocols[i], ActivePlayers[i]->HousePtr->Name)) {
				Host_Remove_Player(i);
				i--;
			} else {
				size = (PacketLength[PACKET_HOUSE_UPDATE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * 1;
				ReliableProtocols[i]->Queue->Queue_Send(&packet, size);
				ReliableComms[i]->Send();
				SentBytesSec += size;				
				SentTCP++;
			}
		}
	}
	return true;
}

extern bool Overlay_Is_Crate(OverlayType type);

bool Host_Send_Crates_State_Packet(int index)
{
	CratePacketData *data;
	CellClass *cptr;
	WDTPacketStruct packet;
	int count;
	int max;
	int size;
	short i;

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);;
	packet.Header.Type = PACKET_CRATE;

	for (i = 0; i < MAP_CELL_TOTAL; i++) {
		cptr = &Map[i];
		if (Map.In_Radar(i) && Overlay_Is_Crate(cptr->Overlay)) {
			data = &packet.Crate.Data[count];
			data->Cell = i;
			data->Overlay = cptr->Overlay;
			data->OverlayFrame = cptr->OverlayData;
			count++;
			if (count >= max) {
				if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
					return false;
				}
				packet.Crate.Count = count;
				size = (PacketLength[PACKET_CRATE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
				ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
				ReliableComms[index]->Send();
				SentBytesSec += size;
				SentTCP++;
				count = 0;
			}
		}
	}

	packet.Crate.Count = count;
	if (packet.Crate.Count) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			return false;
		}
		size = (PacketLength[PACKET_CRATE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}

	return true;
}

bool Host_Send_Player_Leave_Packet_To_All(HousesType house)
{
	int size;
	int i;
	WDTPacketStruct packet;

	packet.Header.Type = PACKET_PLAYER_LEAVE;
	packet.PlayerLeave.House = house;

	for (i = 0; i < ActivePlayers.Count(); i++) {
		if (!Host_Wait_Client(ReliableProtocols[i], ActivePlayers[i]->HousePtr->Name)) {
			Host_Remove_Player(i);
			i--;
		} else {
			size = PacketLength[PACKET_PLAYER_LEAVE];
			ReliableProtocols[i]->Queue->Queue_Send(&packet, size);
			ReliableComms[i]->Send();
			SentBytesSec += size;
			SentTCP++;
		}
	}

	return true;
}

void Host_Send_Health_Packet(int index)
{
	WDTPacketStruct packet;
	int i;
	int count;
	int max;
	HealthPacketData *data;
	int size;

	if (!HealthPacketDatas.Count()) {
		return;
	}
	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_HEALTH;
	if ( ReliableProtocols[index]->Queue->Num_Send() <= 2 * ReliableProtocols[index]->Queue->Max_Send() / 3) {
		for (i = 0; i < HealthPacketDatas.Count(); i++) {
			if (As_Object(HealthPacketDatas[i]->Whom) && As_Object(HealthPacketDatas[i]->Whom)->IsActive) {
				data = &packet.Health.Data[count];
				data->Whom = HealthPacketDatas[i]->Whom;
				data->Health = HealthPacketDatas[i]->Health;
				count++;
				if (count >= max) {
					packet.Health.Count = count;
					/*
					** Calculate actual size of packet to send.
					*/
					size = (PacketLength[PACKET_HEALTH] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
					ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
					ReliableComms[index]->Send();
					SentBytesSec += size;
					SentTCP++;
					count = 0;
				}
			}
		}
		packet.Health.Count = count;
		if (packet.Health.Count) {
			/*
			** Calculate actual size of packet to send.
			*/
			size = (PacketLength[PACKET_HEALTH] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;
		}
	}
}

bool Host_Send_Damage_Packet(int index)
{
	int i;
	WDTPacketStruct packet;
	int count;
	int max;
	DamagePacketData *data;
	int size;

	if (!DamagePacketDatas.Count()) {
		return true;
	}

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_DAMAGE;

	for (i = 0; i < DamagePacketDatas.Count(); i++) {
		data = &packet.Damage.Data[count];
		data->Whom = DamagePacketDatas[i]->Whom;
		data->Source = DamagePacketDatas[i]->Source;
		data->Warhead = DamagePacketDatas[i]->Warhead;
		count++;
		if (count >= max) {
			if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
				return false;
			}			
			packet.Damage.Count = count;
			/*
			** Calculate actual size of packet to send.
			*/
			size = (PacketLength[PACKET_DAMAGE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;
			count = 0;
		}
	}
	
	packet.Damage.Count = count;
	if (packet.Damage.Count) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			return false;
		}
		/*
		** Calculate actual size of packet to send.
		*/
		size = (PacketLength[PACKET_DAMAGE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}

	return true;
}

bool Host_Send_Squish_Packet(int index)
{
	int i;
	WDTPacketStruct packet;
	int count;
	int max;
	SquishPacketData *data;
	int size;

	if (!SquishPacketDatas.Count()) {
		return true;
	}

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_SQUISH;

	for (i = 0; i < SquishPacketDatas.Count(); i++) {
		if (As_Object(SquishPacketDatas[i]->field_0) && !As_Object(SquishPacketDatas[i]->field_0)->IsActive) {
			data = &packet.Crush.Data[count];
			data->field_0 = SquishPacketDatas[i]->field_0;
			data->field_4 = SquishPacketDatas[i]->field_4;
			count++;
			if (count >= max) {
				if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
					return false;
				}			
				packet.Crush.Count = count;
				/*
				** Calculate actual size of packet to send.
				*/
				size = (PacketLength[PACKET_SQUISH] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
				ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
				ReliableComms[index]->Send();
				SentBytesSec += size;
				SentTCP++;
				count = 0;
			}
		}
	}
	
	packet.Crush.Count = count;
	if (packet.Crush.Count) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			return false;
		}
		/*
		** Calculate actual size of packet to send.
		*/
		size = (PacketLength[PACKET_SQUISH] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}

	return true;
}

bool Host_Send_Capture_Packet(int index)
{
	int i;
	WDTPacketStruct packet;
	int count;
	int max;
	CapturePacketData *data;
	int size;

	if (!CapturePacketDatas.Count()) {
		return true;
	}

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_CAPTURE;

	for (i = 0; i < CapturePacketDatas.Count(); i++) {
		if (As_Object(CapturePacketDatas[i]->Whom) && As_Object(CapturePacketDatas[i]->Whom)->IsActive) {
			data = &packet.Capture.Data[count];
			data->Whom = CapturePacketDatas[i]->Whom;
			data->NewHouse = CapturePacketDatas[i]->NewHouse;
			count++;
			if (count >= max) {
				if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
					return false;
				}			
				packet.Capture.Count = count;
				/*
				** Calculate actual size of packet to send.
				*/
				size = (PacketLength[PACKET_CAPTURE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
				ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
				ReliableComms[index]->Send();
				SentBytesSec += size;
				SentTCP++;
				count = 0;
			}
		}
	}
	
	packet.Capture.Count = count;
	if (packet.Capture.Count) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			return false;
		}
		/*
		** Calculate actual size of packet to send.
		*/
		size = (PacketLength[PACKET_CAPTURE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}

	return true;
}

// Matching.
bool Host_Send_Cargo_Packet(int index)
{
	int i;
	WDTPacketStruct packet;
	int count;
	int max;
	CargoPacketData *data;
	int size;

	if (!CargoPacketDatas.Count()) {
		return true;
	}

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_CARGO;

	for (i = 0; i < CargoPacketDatas.Count(); i++) {
		if (As_Object(CargoPacketDatas[i]->Whom) &&
			As_Object(CargoPacketDatas[i]->Whom)->IsActive &&
			As_Object(CargoPacketDatas[i]->Cargo) &&
			As_Object(CargoPacketDatas[i]->Cargo)->IsActive) {
			data = &packet.Cargo.Data[count];
			data->Whom = CargoPacketDatas[i]->Whom;
			data->Cargo = CargoPacketDatas[i]->Cargo;
			data->State = CargoPacketDatas[i]->State;
			count++;
			if (count >= max) {
				if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
					return false;
				}			
				packet.Cargo.Count = count;
				/*
				** Calculate actual size of packet to send.
				*/
				size = (PacketLength[PACKET_CARGO] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
				ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
				ReliableComms[index]->Send();
				SentBytesSec += size;
				SentTCP++;
				count = 0;
			}
		}
	}
	
	packet.Cargo.Count = count;
	if (packet.Cargo.Count) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			return false;
		}
		/*
		** Calculate actual size of packet to send.
		*/
		size = (PacketLength[PACKET_CAPTURE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}

	return true;
}

// Matching.
bool Host_Send_Flag_Packet(int index)
{
	int i;
	WDTPacketStruct packet;
	int count;
	int max;
	FlagPacketData *data;
	int size;

	if (!FlagPacketDatas.Count()) {
		return true;
	}

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_FLAG;

	for (i = 0; i < FlagPacketDatas.Count(); i++) {
		if (Target_Kind(FlagPacketDatas[i]->Whom) != KIND_CELL &&
			(!As_Object(FlagPacketDatas[i]->Whom) || !As_Object(FlagPacketDatas[i]->Whom)->IsActive)) {
				continue;
		}
		data = &packet.Flag.Data[count];
		data->House = FlagPacketDatas[i]->House;
		data->Whom = FlagPacketDatas[i]->Whom;
		data->Attached = FlagPacketDatas[i]->Attached;
		count++;
		if (count >= max) {
			if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
				return false;
			}			
			packet.Flag.Count = count;
			/*
			** Calculate actual size of packet to send.
			*/
			size = (PacketLength[PACKET_FLAG] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;
			count = 0;
		}
	}
	
	packet.Flag.Count = count;
	if (packet.Flag.Count) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			return false;
		}
		/*
		** Calculate actual size of packet to send.
		*/
		size = (PacketLength[PACKET_CAPTURE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}

	return true;
}

bool Host_Send_CTF_Packet(int index)
{
	int i;
	WDTPacketStruct packet;
	int count;
	int max;
	CTFPacketData *data;
	int size;

	if (!CTFPacketDatas.Count()) {
		return true;
	}

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_CTF;

	for (i = 0; i < CTFPacketDatas.Count(); i++) {
		data = &packet.CTF.Data[count];
		data->House = CTFPacketDatas[i]->House;
		data->Cell = CTFPacketDatas[i]->Cell;
		data->State = CTFPacketDatas[i]->State;
		count++;
		if (count >= max) {
			if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
				return false;
			}			
			packet.CTF.Count = count;
			/*
			** Calculate actual size of packet to send.
			*/
			size = (PacketLength[PACKET_CTF] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;
			count = 0;
		}	
	}
	
	packet.CTF.Count = count;
	if (packet.CTF.Count) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			return false;
		}
		/*
		** Calculate actual size of packet to send.
		*/
		size = (PacketLength[PACKET_CTF] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}

	return true;
}

void Host_Send_Move_Packet(int index)
{
	WDTPacketStruct packet;
	int i;
	int count;
	int max;
	MovePacketData *data;
	int size;

	if (!MovePacketDatas.Count()) {
		return;
	}
	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_MOVE;
	if ( ReliableProtocols[index]->Queue->Num_Send() <= 2 * ReliableProtocols[index]->Queue->Max_Send() / 3) {
		for (i = 0; i < MovePacketDatas.Count(); i++) {
			if (As_Object(MovePacketDatas[i]->Whom) && As_Object(MovePacketDatas[i]->Whom)->IsActive) {
				data = &packet.Movement.Data[count];
				*data = *MovePacketDatas[i];
				count++;
				if (count >= max) {
					packet.Movement.Count = count;
					/*
					** Calculate actual size of packet to send.
					*/
					size = (PacketLength[PACKET_MOVE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
					ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
					ReliableComms[index]->Send();
					SentBytesSec += size;
					SentTCP++;
					count = 0;
				}
			}
		}
		packet.Movement.Count = count;
		if (packet.Movement.Count) {
			/*
			** Calculate actual size of packet to send.
			*/
			size = (PacketLength[PACKET_MOVE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;
		}
	}
}

// Matching.
void Host_Send_Target_Packet(int index)
{
	WDTPacketStruct packet;
	int i;
	int count;
	int max;
	TargetPacketData *data;
	int size;

	if (!TargetPacketDatas.Count()) {
		return;
	}

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_TARGET;
	if (ReliableProtocols[index]->Queue->Num_Send() <= 2 * ReliableProtocols[index]->Queue->Max_Send() / 3) {
		for (i = 0; i < TargetPacketDatas.Count(); i++) {
			if (!As_Object(TargetPacketDatas[i]->Whom) ||
				!As_Object(TargetPacketDatas[i]->Whom)->IsActive || 
				(As_Object(TargetPacketDatas[i]->Target) &&
				!As_Object(TargetPacketDatas[i]->Target)->IsActive)) {
				continue;
			}
			
			data = &packet.Target.Data[count];
			*data = *TargetPacketDatas[i];
			count++;
			if (count >= max) {
				packet.Target.Count = count;
				/*
				** Calculate actual size of packet to send.
				*/
				size = (PacketLength[PACKET_TARGET] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
				ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
				ReliableComms[index]->Send();
				SentBytesSec += size;
				SentTCP++;
				count = 0;
			}
		}
		packet.Target.Count = count;
		if (packet.Target.Count) {
			/*
			** Calculate actual size of packet to send.
			*/
			size = (PacketLength[PACKET_TARGET] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;
		}
	}
}

// Matching.
void Host_Send_Fire_At_Packet(int index)
{
	WDTPacketStruct packet;
	int i;
	int count;
	int max;
	FireAtPacketData *data;
	int size;

	if (!FireAtPacketDatas.Count()) {
		return;
	}

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_FIRE_AT;
	if (ReliableProtocols[index]->Queue->Num_Send() <= 2 * ReliableProtocols[index]->Queue->Max_Send() / 3) {
		for (i = 0; i < FireAtPacketDatas.Count(); i++) {
			if (!As_Object(FireAtPacketDatas[i]->Whom) ||
				!As_Object(FireAtPacketDatas[i]->Whom)->IsActive ||
				FireAtPacketDatas[i]->Target == 0 ||
				(As_Object(FireAtPacketDatas[i]->Target) &&
				!As_Object(FireAtPacketDatas[i]->Target)->IsActive)) {
				continue;
			}
			
			data = &packet.FireAt.Data[count];
			*data = *FireAtPacketDatas[i];
			count++;
			if (count >= max) {
				packet.FireAt.Count = count;
				/*
				** Calculate actual size of packet to send.
				*/
				size = (PacketLength[PACKET_FIRE_AT] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
				ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
				ReliableComms[index]->Send();
				SentBytesSec += size;
				SentTCP++;
				count = 0;
			}
		}
		packet.FireAt.Count = count;
		if (packet.FireAt.Count) {
			/*
			** Calculate actual size of packet to send.
			*/
			size = (PacketLength[PACKET_TARGET] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;
		}
	}
}

void Host_Send_Do_Turn_Packet(int index)
{
	WDTPacketStruct packet;
	int i;
	int count;
	int max;
	DoTurnPacketData *data;
	int size;

	if (!DoTurnPacketDatas.Count()) {
		return;
	}

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_DO_TURN;
	if ( ReliableProtocols[index]->Queue->Num_Send() <= 2 * ReliableProtocols[index]->Queue->Max_Send() / 3) {
		for (i = 0; i < DoTurnPacketDatas.Count(); i++) {
			if (As_Object(DoTurnPacketDatas[i]->Whom) && As_Object(DoTurnPacketDatas[i]->Whom)->IsActive) {
				data = &packet.DoTurn.Data[count];
				*data = *DoTurnPacketDatas[i];
				count++;
				if (count >= max) {
					packet.Movement.Count = count;
					/*
					** Calculate actual size of packet to send.
					*/
					size = (PacketLength[PACKET_DO_TURN] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
					ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
					ReliableComms[index]->Send();
					SentBytesSec += size;
					SentTCP++;
					count = 0;
				}
			}
		}
		packet.Movement.Count = count;
		if (packet.Movement.Count) {
			/*
			** Calculate actual size of packet to send.
			*/
			size = (PacketLength[PACKET_DO_TURN] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;
		}
	}
}

bool Host_Send_Crate_Packet(int index)
{
	int i;
	WDTPacketStruct packet;
	int count;
	int max;
	CratePacketData *data;
	int size;

	if (!CratePacketDatas.Count()) {
		return true;
	}

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_CRATE;

	for (i = 0; i < CratePacketDatas.Count(); i++) {
		data = &packet.Crate.Data[count];
		*data = *CratePacketDatas[i];
		count++;
		if (count >= max) {
			if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
				return false;
			}			
			packet.Crate.Count = count;
			/*
			** Calculate actual size of packet to send.
			*/
			size = (PacketLength[PACKET_CRATE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;
			count = 0;
		}
	}
	
	packet.Crate.Count = count;
	if (packet.Crate.Count) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			return false;
		}
		/*
		** Calculate actual size of packet to send.
		*/
		size = (PacketLength[PACKET_CRATE] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}

	return true;
}

bool Host_Send_Per_Cell_Process_Packet(int index)
{
	int i;
	WDTPacketStruct packet;
	int count;
	int max;
	PerCellPacketData *data;
	int size;

	if (!PerCellPacketDatas.Count()) {
		return true;
	}
	
	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_PCP;

	for (i = 0; i < PerCellPacketDatas.Count(); i++) {
		data = &packet.PerCell.Data[count];
		*data = *PerCellPacketDatas[i];

		if (data->Number == (char)WDT_CRATE_TELEPORT) {
			if (As_Object(data->Whom) && As_Object(data->Whom)->IsActive) {
				Process_Crate_Pickup((WDTCrateType)data->Number, data->Cell, (FootClass *)As_Object(data->Whom), data->Owner, data->_IntNumber);
			}
		}
		count++;
		if (count >= max) {
			if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
				return false;
			}			
			packet.PerCell.Count = count;
			size = (PacketLength[PACKET_PCP] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;			
			count = 0;
		}
	}
	
	packet.PerCell.Count = count;
	if (packet.PerCell.Count) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			return false;
		}
		/*
		** Calculate actual size of packet to send.
		*/
		size = (PacketLength[PACKET_PCP] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}

	return true;
}

bool Host_Send_Per_Cell_Process_Packet_Offline(void)
{
	WDTPacketStruct packet;
	int i;
	int count;
	int max;
	PerCellPacketData *data;

	if (!PerCellPacketDatas.Count()) {
		return true;
	}
	
	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_PCP;

	for (i = 0; i < PerCellPacketDatas.Count(); i++) {
		data = &packet.PerCell.Data[count];
		*data = *PerCellPacketDatas[i];

		if (data->Number == (char)WDT_CRATE_TELEPORT) {
			if (As_Object(data->Whom) && As_Object(data->Whom)->IsActive) {
				Process_Crate_Pickup((WDTCrateType)data->Number, data->Cell, (FootClass *)As_Object(data->Whom), data->Owner, data->_IntNumber);
			}
		}
		count++;
		if (count >= max) {
			packet.PerCell.Count = count;
			count = 0;
		}
	}
	
	packet.PerCell.Count = count;
	
	return true;
}

bool Host_Send_Techno_Packet(int index)
{
	int i;
	WDTPacketStruct packet;
	int count;
	int max;
	TechnoPacketData *data;
	int size;

	if (!TechnoPacketDatas.Count()) {
		return true;
	}

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_TECHNO;

	for (i = 0; i < TechnoPacketDatas.Count(); i++) {
		data = &packet.Techno.Data[count];
		*data = *TechnoPacketDatas[i];
		count++;
		if (count >= max) {
			if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
				return false;
			}			
			packet.Techno.Count = count;
			/*
			** Calculate actual size of packet to send.
			*/
			size = (PacketLength[PACKET_TECHNO] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;
			count = 0;
		}
	}
	
	packet.Techno.Count = count;
	if (packet.Techno.Count) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			return false;
		}
		/*
		** Calculate actual size of packet to send.
		*/
		size = (PacketLength[PACKET_TECHNO] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}

	return true;
}

// Auto variables in stack slightly off?
bool Host_Send_New_Delete_Packet(int index)
{
	int j;
	int i;
	WDTPacketStruct packet;
	int count;
	int max;
	NewDeletePacketData *data;
	ObjectClass *optr;
	int size;

	if (!NewDeletePacketDatas.Count()) {
		return true;
	}
	
	/*
	** Clean up any duplicate requests before processing the list for sending.
	*/
	for (i = 0; i < NewDeletePacketDatas.Count(); i++) {
		for (j = i + 1; j < NewDeletePacketDatas.Count(); j++) {
			if (NewDeletePacketDatas[i]->Whom == NewDeletePacketDatas[j]->Whom) {
				NewDeletePacketDatas.Delete(j);
				NewDeletePacketDatas.Delete(i);
				i--;
				break;
			}
		}
	}

	count = 0;
	max = MAX_PACKET_DATA_SIZE / sizeof(*data);
	packet.Header.Type = PACKET_NEW_DELETE_OBJ;

	for (i = 0; i < NewDeletePacketDatas.Count(); i++) {
		data = &packet.NewDelete.Data[count];

		if (!NewDeletePacketDatas[i]->IsDeletePacket) {
			optr = As_Object(NewDeletePacketDatas[i]->Whom);

			if (!optr || !optr->IsActive) {
				continue;
			}

			data->IsDeletePacket = false;
			data->Whom = NewDeletePacketDatas[i]->Whom;

			if (optr->IsInLimbo) {
				data->Coord = (COORDINATE)(-1);
			} else {
				data->Coord = optr->Coord;
			}

			data->Owner = optr->Owner();

			if (optr->Is_Techno()) {
				data->Mission = optr->Get_Mission();
			} else {
				data->Mission = MISSION_NONE;
			}

			switch (optr->What_Am_I()) {
				case RTTI_INFANTRY:
					data->Type = ((InfantryClass *)optr)->Class->Type;
					break;
				case RTTI_UNIT:
					data->Type = ((UnitClass *)optr)->Class->Type;
					break;
				case RTTI_BUILDING:
					data->Type = ((BuildingClass *)optr)->Class->Type;
					break;
				case RTTI_AIRCRAFT:
					data->Type = ((AircraftClass *)optr)->Class->Type;
					break;
				default:
					continue;
			}

			data->Strength = Scale_Value_Down(sole_array[0][2], optr->Mod1);;
			data->Speed = Scale_Value_Down(sole_array[2][2], optr->Mod2);
			data->Damage = Scale_Value_Down(sole_array[1][2], optr->Mod3);
			data->ROF = Scale_Value_Down(sole_array[3][2], optr->Mod4);
			data->Range = Scale_Value_Down(sole_array[4][2], optr->Mod5);
		} else {
			data->IsDeletePacket = true;
			data->Whom = NewDeletePacketDatas[i]->Whom;
		}

		count++;
		if (count >= max) {
			if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
				return false;
			}			
			packet.NewDelete.Count = count;
			/*
			** Calculate actual size of packet to send.
			*/
			size = (PacketLength[PACKET_NEW_DELETE_OBJ] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++;
			count = 0;
		}
	}
	
	packet.NewDelete.Count = count;
	if (packet.NewDelete.Count) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			return false;
		}
		/*
		** Calculate actual size of packet to send.
		*/
		size = (PacketLength[PACKET_NEW_DELETE_OBJ] - (sizeof(*data) * (MAX_PACKET_DATA_SIZE / sizeof(*data)))) + sizeof(*data) * count;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}

	return true;
}
void Host_Send_Spectator_Packet_To_All(int index)
{
	WDTPacketStruct packet;
	int size;
	int i;

	packet.Header.Type = PACKET_SPECTATOR;
	packet.Spectator.House = ActivePlayers[index]->HousePtr->Class->House;
	size = PacketLength[PACKET_SPECTATOR];
	
	for (i = 0; i < ActivePlayers.Count(); i++) {
		if (!Host_Wait_Client(ReliableProtocols[i], ActivePlayers[i]->HousePtr->Name)) {
			Host_Remove_Player(i);
			i--;
		} else {
			ReliableProtocols[i]->Queue->Queue_Send(&packet, size);
			ReliableComms[i]->Send();
			SentBytesSec += size;
			SentTCP++; 
		}
	}
	Map.Color_List_Toggle_Spectator(ActivePlayers[index]->Name, true);
}

extern void Victory_Dialog(HousesType team);
extern int Host_Game_Results(void);
void Host_Send_Game_Results_Packet_To_All(HousesType house)
{
	int res;
	int i;
	int size;
	WDTPacketStruct packet;

	if (OfflineMode) {
		Victory_Dialog(HOUSE_MULTI1);
		Sound_Effect(VOC_RADAR_OFF);
		return;
	}

	packet.Header.Type = PACKET_GAME_END;
	packet.GameEnd.House = house;
	size = PacketLength[PACKET_GAME_END];
	
	for (i = 0; i < ActivePlayers.Count(); i++) {
		if (!Host_Wait_Client(ReliableProtocols[i], ActivePlayers[i]->HousePtr->Name)) {
			Host_Remove_Player(i);
			i--;
		} else {
			ReliableProtocols[i]->Queue->Queue_Send(&packet, size);
			ReliableComms[i]->Send();
			SentBytesSec += size;
			SentTCP++;
		}
	}
	res = Host_Game_Results();
	if (res != 1) {
		printf("Error sending game results: %d\n", res);
	} else {
		printf("Game results sent ok\n");
	}
}

extern int SquadAcceptanceState; // CONQUER.CPP
extern bool DebugLogTeams;
extern CountDownTimerClass CrateKeepTimer; // GLOBALS.CPP
extern int CrateDensity;
extern int Host_Pick_Random_Map();
extern void Add_WDT_Radar();
extern void Clear_Team_Scores();
extern void Remove_WDT_Radar();
extern CountDownTimerClass CrateKeepTimer;
extern bool Init_Flag_Homes(); // INI.CPP

// Matching.
void Host_Send_Scenario_Change_Packet(void)
{
	WDTPacketStruct packet;
	int index;
	int size;
	int team;
	HousesType house;
	
	if (GameParams.IsSquadChannel && SquadAcceptanceState == 2) {
		SquadAcceptanceState = 0;
	}

	Scenario = Host_Pick_Random_Map();
	packet.Header.Type = PACKET_SCENARIO_CHANGE;
	GameOptionsBitfield = !GameOptionsBitfield;
	packet.Scenario._SomeBit = GameOptionsBitfield;
	packet.Scenario.Scenario = Scenario;
	size = PacketLength[PACKET_SCENARIO_CHANGE];

	for (index = 0; index < ActivePlayers.Count(); index++) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			Host_Remove_Player(index);
			index--;
		} else {
			ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
			ReliableComms[index]->Send();
			SentBytesSec += size;
			SentTCP++; 
		}
	}
	
	Set_Scenario_Name(ScenarioName, Scenario, ScenPlayer, ScenDir, SCEN_VAR_A);
	
	Hide_Mouse();
	Fade_Palette_To(BlackPalette, 15, Call_Back);
	HiddenPage.Clear();
	VisiblePage.Clear();
	
	if (!GameParams.FreeRadarForAll) {
		Remove_WDT_Radar();
	}
	
	Frame = 0;

	if (Start_Scenario(ScenarioName)) {
		Fade_Palette_To(BlackPalette, 15, Call_Back);
		Map.Set_Cursor_Shape(MOUSE_NORMAL);
		Map.Set_Default_Mouse(MOUSE_NORMAL, false);
		Map.PendingObjectPtr = NULL;
		Map.PendingObject = NULL;
		Map.PendingHouse = HOUSE_NONE;

		if (!OfflineMode) {
			Debug_Unshroud = true;
			Map.Activate(true);
			Add_WDT_Radar();
		} else {
			Map.Activate(false);
		}
		
		Set_Logic_Page(HidPage);
		Map.Flag_To_Redraw(true);
		Map.Render();
		Fade_Palette_To(GamePalette, 15, NULL);
		Show_Mouse();
		GameActive = true;
		DoList.Init();
		OutList.Init();
		Keyboard::Clear();
		Kbd.Clear();
		InMainLoop = true;

		/*
		** Prepare player houses for new scenario.
		*/
		for (index = 0; index < ActivePlayers.Count(); index++) {
			ActivePlayers[index]->HousePtr->IsDefeated = true;
			ActivePlayers[index]->HousePtr->Timer1.Set(0);
			ActivePlayers[index]->HousePtr->Int1 = 0;
			ActivePlayers[index]->HousePtr->Int2 = 0;
			ActivePlayers[index]->HousePtr->Int3 = 0;
			ActivePlayers[index]->HousePtr->IsHuman = true;
			strcpy(ActivePlayers[index]->HousePtr->Name, ActivePlayers[index]->Name);

			if (ActivePlayers[index]->House == HOUSE_GOOD) {
				ActivePlayers[index]->HousePtr->Init_Data(REMAP_YELLOW, HOUSE_GOOD, 0);
				ActivePlayers[index]->HousePtr->Int4 = -1;
			} else if (ActivePlayers[index]->House == HOUSE_BAD) {
				ActivePlayers[index]->HousePtr->Init_Data(REMAP_YELLOW, HOUSE_BAD, 0);
				ActivePlayers[index]->HousePtr->Int4 = -1;
			} else if (ActivePlayers[index]->House == HOUSE_BLUE_TEAM) {
				ActivePlayers[index]->HousePtr->Init_Data(REMAP_AQUA, HOUSE_BLUE_TEAM, 0);
				ActivePlayers[index]->HousePtr->Int4 = 1;
			} else if (ActivePlayers[index]->House == HOUSE_ORANGE_TEAM) {
				ActivePlayers[index]->HousePtr->Init_Data(REMAP_ORANGE, HOUSE_ORANGE_TEAM, 0);
				ActivePlayers[index]->HousePtr->Int4 = 2;
			} else if (ActivePlayers[index]->House == HOUSE_GREEN_TEAM) {
				ActivePlayers[index]->HousePtr->Init_Data(REMAP_GREEN, HOUSE_GREEN_TEAM, 0);
				ActivePlayers[index]->HousePtr->Int4 = 3;
			} else if (ActivePlayers[index]->House == HOUSE_GREY_TEAM) {
				ActivePlayers[index]->HousePtr->Init_Data(REMAP_BLUE, HOUSE_GREY_TEAM, 0);
				ActivePlayers[index]->HousePtr->Int4 = 4;
			}
		}

		for (index = 0; index < ActivePlayers.Count(); index++) {
			if (!Host_Send_Houses_Update_Packet(index)) {
				Host_Remove_Player(index);
				index--;
			} else {
				Map.Color_List_Toggle_Spectator(ActivePlayers[index]->Name, false);
				Map.Color_List_Update_Points(ActivePlayers[index]->Name, ActivePlayers[index]->HousePtr->Int2);
			}
		}

		Clear_Packet_Data_Vectors();
		ClientFPS = 30;
		LastClientFrame = 0;
		FramerateUpdateTimer.Set(120, 1);
		RecievedBytesSec = 0;
		SentBytesSec = 0;
		SentTCP = 0;
		SentUDP = 0;
		RecievedTCP = 0;
		RecievedUDP = 0;
		TransmisionStatsTimer.Set(120, 1);
		CrateMaker = 1;
		CrateKeepTimer.Set(36000, 1);
		CrateTimer.Set(0);

		if (GameParams.CrateDensityOverride > 0) {
			CrateDensity = Map.MapCellHeight * Map.MapCellWidth / GameParams.CrateDensityOverride;
		} else {
			if (WDTCrateDensity > 0) {
				CrateDensity = Map.MapCellHeight * Map.MapCellWidth / WDTCrateDensity;
			} else {
				CrateDensity = Map.MapCellHeight * Map.MapCellWidth;
			}
		}

		if (CrateDensity > 1000) {
			CrateDensity = 1000;
		}

		if (GameParams.IsCaptureTheFlag) {
			Init_Flag_Homes();

			for (index = 0; index < ActivePlayers.Count(); index++) {
				if (ActivePlayers[index]->HousePtr->Class->House != HOUSE_ADMIN &&
					ActivePlayers[index]->HousePtr->Class->House != HOUSE_SPECTATOR) {
					house = ActivePlayers[index]->HousePtr->ActLike;
					team = house - 6;
					
					if (team >= 0 && team <= 3) {
						if (HouseClass::As_Pointer(house)->FlagHome == 0) {
							Setup_House_Flags(house);
							Fortify_Flag_Home(house);
						}
					}
				}
			}
		}

		if (GameParams.Football) {
			Init_Flag_Homes();

			for (index = 0; index < ActivePlayers.Count(); index++) {
				if (ActivePlayers[index]->HousePtr->Class->House != HOUSE_ADMIN && ActivePlayers[index]->HousePtr->Class->House != HOUSE_SPECTATOR) {
					house = ActivePlayers[index]->HousePtr->ActLike;
					team = house - 6;
					
					if (team >= 0 && team <= 1) {
						if (HouseClass::As_Pointer(house)->FlagHome == 0) {
							Setup_House_Flags(house);
						}
					}
				}
			}
		}

		WDTGameTimer.Set(0, true);

		if (SpecialDialog){
			Set_Logic_Page(SeenBuff);
			AllSurfaces.SurfacesRestored = true;
		}
		
		if (DebugLogTeams) {
			CCDebugString("*ClearTeamScore D:\n");
		}

		Clear_Team_Scores();
	}
}

void Host_Send_Framerate_Packet(int index)
{
	WDTPacketStruct packet;

	if (ReliableProtocols[index]->Queue->Num_Send() <= 2 * ReliableProtocols[index]->Queue->Max_Send() / 3) {
		packet.Header.Type = PACKET_FRAMERATE;
		packet.FrameRate.FPS = ClientFPS;
		ReliableProtocols[index]->Queue->Queue_Send(&packet, PacketLength[PACKET_FRAMERATE]);
		ReliableComms[index]->Send();
		SentBytesSec += PacketLength[PACKET_FRAMERATE];
		SentTCP++;
	}
}

void Host_Process_Remote_Admin_Packet(int index)
{
	AdminResponsePacket response;
	PacketHeaderStruct *packet;
	CommBufferClass *queue;
	int respond;
	int i;
	int count;
	HousesType house;

	respond = false;
	queue = RemoteAdminsProtocols[index]->Queue;
	packet = (PacketHeaderStruct*)queue->Get_Receive(0)->Buffer;
	
	switch (packet->Type) {
		case 0:
			count = 0;
			for (i = 0; i < ActivePlayers.Count(); i++) {
				house = ActivePlayers[i]->HousePtr->Class->House;
				if (house != HOUSE_SPECTATOR && house != HOUSE_ADMIN) {
					count++;
				}
			}
			response.Header.Type = (PacketType)0;
			response.Value = count;
			response.Data[0] = 0;
			respond = true;
			break;
			
		case 1:
			count = 0;
			for (i = 0; i < ActivePlayers.Count(); i++) {
				house = ActivePlayers[i]->HousePtr->Class->House;
				if (house == HOUSE_SPECTATOR) {
					count++;
				}
			}
			response.Header.Type = (PacketType)1;
			response.Value = count;
			response.Data[0] = 0;
			respond = true;
			break;
			
		case 2:
			count = 0;
			for (i = 0; i < ActivePlayers.Count(); i++) {
				house = ActivePlayers[i]->HousePtr->Class->House;
				if ( house == HOUSE_ADMIN) {
					count++;
				}
			}
			response.Header.Type = (PacketType)2;
			response.Value = count;
			response.Data[0] = 0;
			respond = true;
			break;
			
		case 3:
			response.Header.Type = (PacketType)3;
			response.Value = RemoteAdminsComms.Count();
			response.Data[0] = 0;
			respond = true;
			break;
			
		case 4:
			response.Header.Type = (PacketType)4;
			response.Value = 0;
			for (i = 0; i < MPlayerFilenum.Count(); i++) {
				if (Scenario == MPlayerFilenum[i]) {
					strcpy(response.Data, MPlayerScenarios[i]);
				}
			}
			respond = true;
			break;
			
		case 5:
			Host_Disconnect();
			Free_Scenario_Descriptions();
			Prog_End();
			exit(0);
			break;
			
		case 6:
			Set_Palette(BlackPalette);
			VisiblePage.Clear();
			ShowWindow(MainWindow, 6);
			break;
	}

	if (respond) {
		RemoteAdminsProtocols[index]->Queue->Queue_Send(&response, sizeof(AdminResponsePacket));
		RemoteAdminsComms[index]->Send();
	}
	
	queue->UnQueue_Receive(0, 0, 0);
}

// Matching
void Host_Process_Message_Packet(WDTPacketStruct *packet)
{
	char buffer[100];
	int size;
	int i;
	char *msg;
	HouseClass *hptr;

	if (packet->Message.To == HOUSE_NONE) {
		for (i = 0; i < ActivePlayers.Count(); i++) {
			if (ActivePlayers[i]->HousePtr->Class->House != packet->Message.From) {
				if (packet->Message.Team != -1 &&
					packet->Message.Team != ActivePlayers[i]->HousePtr->Int4 &&
					ActivePlayers[i]->HousePtr->Class->House != HOUSE_ADMIN) {
					continue;
				}

				if (!Host_Wait_Client(ReliableProtocols[i], ActivePlayers[i]->HousePtr->Name)) {
					continue;
				}

				/*
				** Calculate actual size of packet to send.
				*/
				msg = packet->Message.MessageBuffer;
				size = (PacketLength[PACKET_MESSAGE] - (sizeof(packet->Message.MessageBuffer))) + strlen(msg) + 1;
				ReliableProtocols[i]->Queue->Queue_Send(packet, size);
				ReliableComms[i]->Send();
				SentBytesSec += size;
				SentTCP++;
			}
		}
	} else {
		for (i = 0; i < ActivePlayers.Count(); i++) {
			if (ActivePlayers[i]->HousePtr->Class->House == packet->Message.To) {
				break;
			}
		}

		if (ActivePlayers.Count() == i) {
			return;
		}

		if (Host_Wait_Client(ReliableProtocols[i], ActivePlayers[i]->HousePtr->Name)) {
			/*
			** Calculate actual size of packet to send.
			*/
			msg = packet->Message.MessageBuffer;
			size = (PacketLength[PACKET_MESSAGE] - (sizeof(packet->Message.MessageBuffer))) + strlen(msg) + 1;
			ReliableProtocols[i]->Queue->Queue_Send(packet, size);
			ReliableComms[i]->Send();
			SentBytesSec += size;
			SentTCP++;
		}
	}

	hptr = HouseClass::As_Pointer(packet->Message.From);

	if (hptr->Class->House == HOUSE_ADMIN && packet->Message.To == HOUSE_NONE) {
		sprintf(buffer, "%s: %s", hptr->Name, packet->Message.MessageBuffer);
		Messages.Add_Message(buffer, 5, TPF_6POINT | TPF_FULLSHADOW | TPF_BRIGHT_COLOR, 0);
		Map.Flag_To_Redraw(false);
	} else if (packet->Message.Team != -1) {
		sprintf(buffer, "(%s): %s", hptr->Name, packet->Message.MessageBuffer);
		Messages.Add_Message(buffer, MPlayerTColors[packet->Message.Team + 1], TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
		Map.Flag_To_Redraw(false);
	} else {
		if (packet->Message.To == HOUSE_NONE) {
			sprintf(buffer, "%s: %s", hptr->Name, packet->Message.MessageBuffer);
			Messages.Add_Message(buffer, 7, TPF_6PT_GRAD | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
			Map.Flag_To_Redraw(false);
		} else {
			sprintf(buffer, "%s (to %s): %s", hptr->Name, HouseClass::As_Pointer(packet->Message.To)->Name, packet->Message.MessageBuffer);
			Messages.Add_Message(buffer, 210, TPF_6PT_GRAD | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
			Map.Flag_To_Redraw(false);
		}
	}
}

void Host_Add_To_Accepted_List(const char *name)
{
	for (int i = 0; i < 100; i++) {
		if (stricmp(AcceptedList[i], name) == 0){
			break;
		}
	}
	
	if (i == 100) {
		i = 0;
		while (stricmp(AcceptedList[i], "") != 0) {
			i++;
		}
		
		strcpy(AcceptedList[i], name);
	}
}

extern void Get_Or_Set_Current_Directory(bool get);
extern void Host_Send_Password_Packet_To_All(void);
extern CountDownTimerClass SquadGameCountdownTimer;
extern void Host_Send_Command_Response_Packet(int index, int state);

// Matching
void Host_Process_Command_Packet(WDTPacketStruct *packet, int index)
{
	char buf[200];
	char string[100];
	char name[100];
	bool sysop;
	char filename[260];
	int num;

	if (packet->Command.State == 0) {
		Get_Or_Set_Current_Directory(0);
		getcwd(filename, sizeof(filename));
		strcat(filename, "\\SERVER.INI");
		sysop = false;
		num = 1;
		
		do {
			sprintf(name, "Sysop%d", num);
			GetPrivateProfileString("Multiplayer", name, "", string, sizeof(string), filename);
			if (!stricmp(string, ActivePlayers[index]->HousePtr->Name)) {
				sysop = true;
			}
			num++;
		} while (!sysop && strlen(string) > 0);
		
		if (sysop) {
			SquadPassword = packet->Command.Obfuscated;
			SquadAcceptanceState = 1;
			Host_Send_Password_Packet_To_All();
			SquadGameCountdownTimer.Set(60 * (GameParams.PasswordCountdownSeconds + 2));
			
			for (int i = 0; i < 100; i++) {
				strcpy(AcceptedList[i], "");
			}
			
			num = 1;
			do {
				sprintf(name, "Sysop%d", num);
				GetPrivateProfileString("Multiplayer", name, "", string, sizeof(string), filename);
				if (stricmp(string, "")) {
					Host_Add_To_Accepted_List(string);
				}
				num++;
			} while ( strlen(string) > 0);
		} else {
			sprintf(buf, "*** Warning: %s knows the password command\n.", ActivePlayers[index]->HousePtr->Name);
			CCDebugString(buf);
		}
	} else {
		int res = packet->Command.Obfuscated == SquadPassword;
		if (res == 1) {
			Host_Add_To_Accepted_List(ActivePlayers[index]->HousePtr->Name);
		}
		Host_Send_Command_Response_Packet(index, res);
	}
}

extern bool Handle_Typed_Message(char *message); // WDT.CPP

// Matching
void Host_Process_Message(char *message)
{
	WDTPacketStruct packet;
	char str[100];
	char privmsg[100];
	char *msg;
	int i;
	bool is_to;
	int size;
	int index;

	is_to = false;

	if (message == NULL) {
		msg = Messages.Get_Edit_Buf();
	} else {
		msg = message;
	}

	if (strlen(msg) == 0) {
		return;
	}

	if (Obfuscate(msg) == PARM_SECRET_CREDITS) {
		SpecialDialog = SDLG_SECRET_CREDITS;
		return;
	}

	if (*msg == '#') {
		is_to = true;
		msg++;
		
		i = 0;
		while (true) {
			if (strlen(msg) <= i || i >= sizeof(privmsg) || msg[i] == ' ') {
				break;
			}
			
			privmsg[i] = msg[i];
			i++;
		}
		
		privmsg[i] = 0;
		
		if (msg[i] != ' ') {
			sprintf(str, Text_String(TXT_FORMAT_FOR_PRIVATE_MESSAGE_IS));
			Messages.Add_Message(str, 15, TPF_6POINT|TPF_NOSHADOW|TPF_BRIGHT_COLOR, 0);
			Map.Flag_To_Redraw(false);
			return;
		}
		
		msg += strlen(privmsg) + 1;
		
		if (!strlen(msg)) {
			return;
		}

		index = 0; 
		while (true) {
			if(index >= Houses.Count() || stricmp(Houses.Ptr(index)->Name, privmsg) == 0) {		
				break;
			}

			index++;
		}

		if (Houses.Count() == index) {
			sprintf(str, Text_String(TXT_NO_PLAYER_NAMED), privmsg);
			Messages.Add_Message(str, 15, TPF_6POINT|TPF_NOSHADOW|TPF_BRIGHT_COLOR, 0);
			Map.Flag_To_Redraw(false);
			return;
		}
		
		Messages.Add_Private_Message(privmsg);
	} else {
		if (Handle_Typed_Message(msg)) {
			return;
		}
	}

	packet.Header.Type = PACKET_MESSAGE;
	packet.Message.From = PlayerPtr->Class->House;

	if (is_to) {
		packet.Message.To = Houses.Ptr(index)->Class->House;
	} else {
		packet.Message.To = HOUSE_NONE;
	}

	packet.Message.Team = -1;
	
	strcpy(packet.Message.MessageBuffer, msg);
	size = (PacketLength[PACKET_MESSAGE] - 80) + strlen(msg) + 1;

	if (is_to) {
		for (i = 0; i < ActivePlayers.Count(); i++) {
			if (ActivePlayers[i]->HousePtr->Class->House == packet.Message.To) {
				break;
			}
		}

		if (ActivePlayers.Count() == i) {
			return;
		}

		ReliableProtocols[i]->Queue->Queue_Send(&packet, size);
		ReliableComms[i]->Send();
		SentBytesSec += size;
		SentTCP++;
	} else {
		for (i = 0; i < ActivePlayers.Count(); i++) {
			ReliableProtocols[i]->Queue->Queue_Send(&packet, size);
			ReliableComms[i]->Send();
			SentBytesSec += size;
			SentTCP++;
		}
	}

	if (is_to) {
		sprintf(str, Text_String(TXT_S_TO_S_COLON_S), PlayerPtr->Name, privmsg, msg);
		Messages.Add_Message(str, 210, TPF_6PT_GRAD | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
	} else {
		sprintf(str, "%s: %s", PlayerPtr->Name, msg);
		Messages.Add_Message(str, 5, TPF_6POINT | TPF_FULLSHADOW | TPF_BRIGHT_COLOR, 0);
	}

	Map.Flag_To_Redraw(false);
}

// Matching
void Host_Recreate_Admin_Units(void)
{
	HouseClass *hptr;
	int i;
	TechnoClass *tptr;
	bool delete_existing;
	int index;
	HousesType house;

	hptr = HouseClass::As_Pointer(HOUSE_ADMIN);

	if (hptr->ActiveBScan == 0 && hptr->ActiveAScan == 0 && hptr->UScan == 0 && hptr->ActiveIScan == 0) {
		delete_existing = false;
	} else {
		delete_existing = true;
	}
	
	if (delete_existing) {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			house = ActivePlayers[index]->HousePtr->Class->House;
			if (house == HOUSE_ADMIN) {
				for (i = 0; i < ActivePlayers[index]->Technos.Count(); i++) {
					tptr = ActivePlayers[index]->Technos[i];

					if (tptr != NULL && tptr->IsActive) {
						tptr->Stun();
						DELETE_OBJ(tptr, sizeof(AbstractClass));
					}
				}

				ActivePlayers[index]->Technos.Clear();
			}
		}
	} else {
		for (index = 0; index < ActivePlayers.Count(); index++) {
			house = ActivePlayers[index]->HousePtr->Class->House;
			if (house == HOUSE_ADMIN) {
				Make_Player_Unit(index);
			}
		}
	}
}

extern const char *Local_Time_As_String();
extern bool Host_Swollow_Packets_While_Waiting(ReliableProtocolClass *protocol);

bool Host_Wait_Client(ReliableProtocolClass *protocol, char *name)
{
	char buf[1000];

	if (!protocol->ConnectionState) {
		return false;
	}

	if (protocol->Queue->Num_Send() == protocol->Queue->Max_Send()) { 
		if (!Host_Swollow_Packets_While_Waiting(protocol)) {
			sprintf(buf, "** Wait_Client failed for %s at time %s\n", name, Local_Time_As_String());
			CCDebugString(buf);
			
			return false;
		}
	}
	
	return true;
}

bool Host_Swollow_Packets_While_Waiting(ReliableProtocolClass *protocol)
{
	int index;
	PacketHeaderStruct *head;
	SendQueueType *send;

	for (index = 0; index < protocol->Queue->Num_Send(); index++) {
		send = protocol->Queue->Get_Send(index);
		if (send) {
			head = (PacketHeaderStruct*)send->Buffer;
			if (head->Type == PACKET_HEALTH
			  ||head->Type == PACKET_MOVE
			  ||head->Type == PACKET_TARGET
			  ||head->Type == PACKET_FIRE_AT
			  ||head->Type == PACKET_DO_TURN
			  ||head->Type == PACKET_FRAMERATE) {
				protocol->Queue->UnQueue_Send(0, 0, index, 0, 0);
				return true;
			}
		}
	}
	
	return false;
}

void Host_Ban_Player(char *player_to_ban)
{
	for (int i = 0; i < ActivePlayers.Count(); i++) {
		if (!strcmp(ActivePlayers[i]->HousePtr->Name, player_to_ban)) {
			HousesType house = ActivePlayers[i]->HousePtr->Class->House;
			Host_Remove_Player(i);
			Host_Send_Player_Leave_Packet_To_All(house);
			char buf[200];
			sprintf(buf, "Player %s banned.\n", player_to_ban);
			CCDebugString(buf);
			break;
		}
	}
	char *data = new char[12];
	strcpy(data, player_to_ban);
	BannedPlayers.Add(data);
}

extern HousesType Who_Won_Or_Lost(GAMEPARAMS *params); // UNIT.CPP

// Almost, hidden retval and size in wrong places
int Host_Game_Results(void)
{
	int index;
	int playercount;
	struct hostent *heptr;
	int buf_pos;
	int send_size;
	FILE *stream;
	SOCKET con_sock;
	unsigned long ip_addr;
	int size;
	char *dataptr;
	unsigned short packet_size;
	HousesType winner;
	GameResultClass::GameResultEntry game_result;
	GameResultClass::StateStruct state;
	struct sockaddr_in addr;
	struct sockaddr_in con_addr;

	playercount = 0;
	buf_pos = 0;
	playercount = ActivePlayers.Count();

	if (*ResultHost == '\0') {
		strcpy(ResultHost, "games.westwood.com");
	}

	state.TimeLimit = GameParams.TimeLimit;
	state.ScoreLimit = GameParams.ScoreLimit;
	state.LifeLimit = GameParams.LifeLimit;

	if (GameParams.IsCaptureTheFlag) {
		state.GameMode = 1;
	} else if (GameParams.Football) {
		state.GameMode = 3;
	} else {
		state.GameMode = 0;
	}

	state.IsLadderGame = GameParams.IsLadderGame;
	ServerGameResults.Set_Game_State(&state);
	winner = Who_Won_Or_Lost(&GameParams);

	for (index = 0; index < playercount; index++) {
		if (ActivePlayers[index]->HousePtr->Class->House != HOUSE_SPECTATOR && ActivePlayers[index]->HousePtr->Class->House != HOUSE_ADMIN) {
			strcpy(game_result.Name, ActivePlayers[index]->Name);
			game_result.ScoredPoints = ActivePlayers[index]->HousePtr->Int2;
			game_result.TotalDeaths = ActivePlayers[index]->HousePtr->Int3;
			game_result.PlayerCount = ActivePlayers[index]->SomeTiming;
			game_result.TimeIngame = time(0) - ActivePlayers[index]->SomeTiming;
			game_result.timing3_score = Host_Calculate_Total_Score();

			if (winner == index) {
				game_result.dword1C_score = 100;
			} else {
				game_result.dword1C_score = Host_Comm_Calc_Score(index);
			}

			if (ActivePlayers[index]->RTTI == RTTI_INFANTRY ||
				ActivePlayers[index]->RTTI == RTTI_INFANTRYTYPE) {
				game_result.chosentype = ActivePlayers[index]->Type + 100;
			} else {
				game_result.chosentype = ActivePlayers[index]->Type;
			}

			if (GameParams.IsCaptureTheFlag || GameParams.Football) {
				if (ActivePlayers[index]->HousePtr->ActLike == winner) {
					game_result.IsWinnerOrLoser = true;
				} else {
					game_result.IsWinnerOrLoser = false;
				}
			} else {
				if (ActivePlayers[index]->HousePtr->Class->House == winner) {
					game_result.IsWinnerOrLoser = true;
				} else {
					game_result.IsWinnerOrLoser = false;
				}
			}

			ServerGameResults.Push_Result(&game_result);
			memset(&game_result, 0, sizeof(game_result));
		}
	}

	for (index = 0; index < playercount; index++) {
		ActivePlayers[index]->SomeTiming = time(0);
	}

	// TODO All early returns before the final one leak memory by not freeing dataptr
	ServerGameResults.Host_To_Net((void **)&dataptr, &size);

	if (size <=0 ) {
		return -1;
	}

	packet_size = htons(size + 2);
	con_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (con_sock == INVALID_SOCKET) {
		ServerGameResults.Reset();
		return -5;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(0);
	addr.sin_port = htons(0);
	
	if (bind(con_sock, (const struct sockaddr *)&addr, sizeof(addr)) != 0) {
		ServerGameResults.Reset();
		return -3;
	}

	ip_addr = inet_addr(ResultHost);

	if (ip_addr == ULONG_MAX) {
		heptr = gethostbyname(ResultHost);

		if (heptr == NULL) {
			ServerGameResults.Reset();
			return -2;
		}

		ip_addr = **(unsigned long **)heptr->h_addr_list;
	}

	printf("Using host %s for game results\n", ResultHost);
	con_addr.sin_family = AF_INET;
	con_addr.sin_addr.s_addr = ip_addr;
	con_addr.sin_port = htons(4809);

	if (connect(con_sock, (const struct sockaddr *)&con_addr, sizeof(con_addr)) != 0) {
		ServerGameResults.Reset();
		return -4;
	}
	
	if (send(con_sock, (const char *)&packet_size, sizeof(packet_size), 0) <= 0) {
		printf("Game_Results: Error sending packet size\n");
		ServerGameResults.Reset();
		return -5;
	}

	printf("Sending %d bytes\n", size);
	stream = fopen("packet", "wb");

	/*
	** Also dump packet to file system for reference?
	*/
	if (stream != NULL) {
		fwrite(&packet_size, sizeof(packet_size), 1u, stream);
		fwrite(dataptr, size, 1u, stream);
		fclose(stream);
	}

	while (buf_pos < size) {
		send_size = send(con_sock, &dataptr[buf_pos], size - buf_pos, 0);
		if (send_size <= 0) {
			printf("Error in send\n");
		}

		buf_pos += send_size;
	}

	delete[] dataptr; // This should be done for all the early returns?
	ServerGameResults.Reset();
	closesocket(con_sock); // So should this?
	return 1;
}

float Host_Calculate_Total_Score(void)
{
	int players;
	unsigned int timing;
	int count;
	int index;
	
	players = 0;
	count = ActivePlayers.Count();
	if (time(0) - LastScoreCalcTime < 3) {
		return CalculatedScore;
	}

	LastScoreCalcTime = time(0);
	CalculatedScore = 0.0;
	players = 0;

	for (index = 0; index < count; index++) {
		if (ActivePlayers[index]->HousePtr->Class->House != HOUSE_SPECTATOR && ActivePlayers[index]->HousePtr->Class->House != HOUSE_ADMIN) {
			players++;
			timing = (time(0) - ActivePlayers[index]->SomeTiming);
			ActivePlayers[index]->SomeScore = (ActivePlayers[index]->HousePtr->Int2 * 60.0) / timing;
			CalculatedScore += ActivePlayers[index]->SomeScore;    
		}
	}
	CalculatedScore = CalculatedScore / players;
	return CalculatedScore;
}

int Host_Comm_Calc_Score(int index)
{
	int players;
	int unknown;
	bool b1;
	bool b2;
	float s1;
	int count;
	int i;

	Host_Calculate_Total_Score();
	players = 0;
	unknown = 0;
	b1 = ActivePlayers[index]->HousePtr->IsVisionary;
	s1 = ActivePlayers[index]->SomeScore;
	count = ActivePlayers.Count();

	for (i = 0; i < count; i++) {
		if (ActivePlayers[i]->HousePtr->Class->House != HOUSE_SPECTATOR && ActivePlayers[i]->HousePtr->Class->House != HOUSE_ADMIN) {
			players++;
			
			b2 = ActivePlayers[i]->HousePtr->IsVisionary;
			
			if (b1 && !b2) {
				continue;
			}

			if (s1 >= ActivePlayers[i]->SomeScore || !b1 && b2) {
				unknown++;
			}
			
			
		}
	}
	
	unknown--;
	players--;

	if (players == 0) {
		players = 1;
	}

	return 100 * unknown / players;
}

//Added in 1.04
void Host_Send_Password_Packet_To_All(void)
{
	WDTPacketStruct packet;

	packet.Header.Type = PACKET_SERVER_PASSWORD;
	packet.Command.State = 2;

	for (int index = 0; index < ActivePlayers.Count(); index++) {
		if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
			Host_Remove_Player(index);
			index--;
			break;
		}
		
		int size = PacketLength[PACKET_SERVER_PASSWORD];
		ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
		ReliableComms[index]->Send();
		SentBytesSec += size;
		SentTCP++;
	}
}

void Host_Send_Command_Response_Packet(int index, int state)
{
	WDTPacketStruct packet;

	packet.Header.Type = PACKET_SERVER_PASSWORD;
	packet.Command.State = state;

	if (!Host_Wait_Client(ReliableProtocols[index], ActivePlayers[index]->HousePtr->Name)) {
		Host_Remove_Player(index);
		return;
	}
	
	int size = PacketLength[PACKET_SERVER_PASSWORD];
	ReliableProtocols[index]->Queue->Queue_Send(&packet, size);
	ReliableComms[index]->Send();
	SentBytesSec += size;
	SentTCP++;
	
}

void Host_Process_Accepted_List(void)
{
	int idx;
	int index;
	HousesType house;

	for (index = 0; index < ActivePlayers.Count(); index++) {
		house = ActivePlayers[index]->HousePtr->Class->House;
		if (house != HOUSE_SPECTATOR && house != HOUSE_ADMIN) {
			for (idx = 0; idx < 100; idx++) {
				if (!stricmp(AcceptedList[idx], ActivePlayers[index]->HousePtr->Name)) {
					break;
				}
			}
			if (idx == 100) {
				Host_Remove_Player(index);
				Host_Send_Player_Leave_Packet_To_All(house);
				index--;
			}
		}
	}
}

void Host_Print_Accepted_List(void)
{
	char str[200];

	int y = 40;
	Conquer_Clip_Text_Print("AcceptedList:", 20, 20, 5, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
	for (int i = 0; i < 20; i++) {
		if (stricmp(AcceptedList[i], "") != 0) {
			sprintf(str, "%02d: %s", i, AcceptedList[i]);
			Conquer_Clip_Text_Print(str, 20, y, 5, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 600, 0);
		}
		y += 20;
	}
	
	Map.Redraw_Objects();
}
