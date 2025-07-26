#include "function.h"
#include "gprotocol.h"
#include <handleapi.h>

char FacingString[256];
int ProcessedRecieves;
int RecieveSetsProcessed;
int SquadGamePasswordCountDown;
int QuoteToShow;
bool MessageLogging;

const char* KindNames[KIND_COUNT] = {
    "NONE",
    "CELL",
    "UNIT",
    "INFANTRY",
    "BUILDING",
    "TERRAIN",
    "AIRCRAFT",
    "TEMPLATE",
    "BULLET",
    "ANIMATION",
    "TRIGGER",
    "TEAM",
    "TEAMTYPE",
};

//Added in 1.04
void Client_Log_Player_Message(char* msg)
{
#ifdef WIN32
    static HANDLE PlayerMessageLogHandle = INVALID_HANDLE_VALUE;
    char buf[256];
    DWORD written;

    if (MessageLogging) {
        sprintf(buf, "%s\r\n", msg);
        PlayerMessageLogHandle = CreateFileA("log.txt", 0x40000000u, 0, 0, 4u, 0x80u, 0);

        if (PlayerMessageLogHandle != INVALID_HANDLE_VALUE) {
            SetFilePointer(PlayerMessageLogHandle, 0, 0, 2u);
            WriteFile(PlayerMessageLogHandle, buf, strlen(buf), &written, 0);
            CloseHandle(PlayerMessageLogHandle);
        }
    }
#endif
}

void Client_Queue_AI()
{
    char str[1000];

    if (!ReliableProtocols[0]->ConnectionState) {
        Show_Mouse();
        ServerConnectionLost = 1;
        sprintf(str,
                "%s%s%s%s",
                Text_String(TXT_CONNECTION_LOST_1),
                Text_String(TXT_CONNECTION_LOST_2),
                Text_String(TXT_CONNECTION_LOST_3),
                Text_String(TXT_TRY_DIFFERENT_CHANNEL));

        WWMessageBox().Process(str);
        EventClass(EventClass::EXIT).Execute();
    } else {
        ProcessedRecieves = 0;
        while (0 < ReliableProtocols[0]->Queue->Num_Receive()) {
            Client_Process_Packet(0);
            ProcessedRecieves++;
        }

        if (ProcessedRecieves > 0) {
            RecieveSetsProcessed++;
        }

        while (OutList.Count && ReliableProtocols[0]->Queue->Num_Send() < 6) {
            Client_Send_Event_Packet();
        }

        Queue_AI_Normal();

        if (FramerateUpdateTimer.Time() == 0) {
            ClientFPS = (Frame - LastClientFrame) / 2;
            if (!ClientFPS) {
                ClientFPS = 1;
            }
            LastClientFrame = Frame;
            FramerateUpdateTimer.Set(120, 1);
        }

        if (TransmisionStatsTimer.Time() == 0) {
            CommStats_Set_Transmission_Stats(
                RecievedBytesSec / 2, SentBytesSec / 2, SentTCP / 2, SentUDP / 2, RecievedTCP / 2, RecievedUDP / 2);
            RecievedBytesSec = 0;
            SentBytesSec = 0;
            SentTCP = 0;
            SentUDP = 0;
            RecievedTCP = 0;
            RecievedUDP = 0;
            TransmisionStatsTimer.Set(120, 1);
        }
    }
}

extern int RecievedBytesSec;
extern int RecievedTCP;

bool Client_Process_Packet(int connectiontype)
{
    WDTPacketStruct* packet;
    CommBufferClass* queue;
    int index;
    HouseUpdatePacketData* house_packet;
    HealthPacketData* health_packet;
    DamagePacketData* damage_packet;
    SquishPacketData* squish_packet;
    CapturePacketData* capture_packet;
    CargoPacketData* cargo_packet;
    FlagPacketData* flag_packet;
    NewDeletePacketData* nd_packet;
    MovePacketData* move_packet;
    TargetPacketData* tgt_packet;
    FireAtPacketData* fat_packet;
    DoTurnPacketData* turn_packet;
    CratePacketData* crate_packet;
    PerCellPacketData* pcp_packet;
    TechnoPacketData* tech_packet;
    int start_size;
    int game_speed;
    TechnoClass* tc;
    HouseClass* hc;
    CellClass* cc;
    FootClass* uc;
    CTFPacketData* ctf_packet;

    if (connectiontype != -1) {
        queue = ReliableProtocols[0]->Queue;
        packet = (WDTPacketStruct*)queue->Get_Receive(0)->Buffer;
        RecievedBytesSec += queue->Get_Receive(0)->BufLen;
        RecievedTCP++;
    }

    switch (packet->Header.Type) {
    case PACKET_PLAYER_LEAVE:
        hc = HouseClass::As_Pointer(packet->PlayerLeave.House);

        if (hc != NULL) {
            Map.Color_List_Remove_Player(hc->Name);
            hc->IsHuman = false;
            hc->Name[0] = '\0';
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_PLAYER_UNITS: {
        for (index = 0; index < packet->PlayerJoin.Count; index++) {
            tc = As_Techno(packet->PlayerJoin.Objects[index]);

            if (tc != NULL && tc->IsActive) {
                tc->IsOwnedByPlayer = true;
                tc->Revealed(PlayerPtr);
                tc->Look(false);
                AllowVoice = false;
                tc->Select();
                AllowVoice = true;
            }
        }

        if (packet->PlayerJoin.Count > 0) {
            PlayerPtr->IsDefeated = false;
            ++PlayerPtr->Int1;
        }

        queue->UnQueue_Receive(0, 0, 0);
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
        Map.Flag_To_Redraw(true);
        break;
    }
    case PACKET_FRAMERATE:
        CommStats_Set_Frame_Rate(packet->FrameRate.FPS, ClientFPS);

        if (packet->FrameRate.FPS > 0) {
            game_speed = 15360 / packet->FrameRate.FPS;
            game_speed += 128;
            game_speed /= 256;
            Options.GameSpeed = game_speed;
            SpeedScale = (packet->FrameRate.FPS * 256) / ClientFPS;
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_GAME_STATE_START:
        start_size = packet->GameState.Size;
        queue->UnQueue_Receive(0, 0, 0);

        if (!Client_Process_Initial_Game_State_Packet(start_size)) {
            return false;
        }

        break;
    case PACKET_HOUSE_UPDATE:
        for (index = 0; index < packet->HouseUpdate.Count; index++) {
            house_packet = &packet->HouseUpdate.Data[index];
            Client_Process_House_Packet(house_packet);
        }

        queue->UnQueue_Receive(0, 0, 0);
        SquadGameCountdownTimer.Set(0);

        /*
			** This looks very questionable, only last data is checked?
			*/
        if (HouseClass::As_Pointer(house_packet->House) == PlayerPtr) {
            Client_Handle_Sight();
        }

        break;
    case PACKET_NEW_DELETE_OBJ:
        for (index = 0; index < packet->NewDelete.Count; index++) {
            nd_packet = &packet->NewDelete.Data[index];

            if (!nd_packet->IsDeletePacket) {
                Client_Process_New_Object_Packet(nd_packet);
            } else {
                Client_Process_Delete_Object_Packet(nd_packet);
            }
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_HEALTH:
        for (index = 0; index < packet->Health.Count; index++) {
            health_packet = &packet->Health.Data[index];
            Client_Process_Health_Packet(health_packet);
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_DAMAGE:
        for (index = 0; index < packet->Damage.Count; index++) {
            damage_packet = &packet->Damage.Data[index];
            Client_Process_Damage_Packet(damage_packet);
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_SQUISH:
        for (index = 0; index < packet->Crush.Count; index++) {
            squish_packet = &packet->Crush.Data[index];
            Client_Process_Squish_Packet(squish_packet);
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_CAPTURE:
        for (index = 0; index < packet->Capture.Count; index++) {
            capture_packet = &packet->Capture.Data[index];
            Client_Process_Capture_Packet(capture_packet);
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_CARGO:
        for (index = 0; index < packet->Cargo.Count; index++) {
            cargo_packet = &packet->Cargo.Data[index];
            Client_Process_Cargo_Packet(cargo_packet);
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_FLAG:
        for (index = 0; index < packet->Flag.Count; index++) {
            flag_packet = &packet->Flag.Data[index];
            Client_Process_Flag_Packet(flag_packet);
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_CTF:
        for (index = 0; index < packet->CTF.Count; index++) {
            ctf_packet = &packet->CTF.Data[index];
            Client_Process_CTF_Packet(ctf_packet);
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_MOVE:
        for (index = 0; index < packet->Movement.Count; index++) {
            move_packet = &packet->Movement.Data[index];
            Client_Process_Movement_Packet(move_packet);
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_TARGET:
        for (index = 0; index < packet->Target.Count; index++) {
            tgt_packet = &packet->Target.Data[index];
            Client_Process_Target_Packet(tgt_packet);
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_FIRE_AT:
        for (index = 0; index < packet->FireAt.Count; index++) {
            fat_packet = &packet->FireAt.Data[index];
            Client_Process_Fire_At_Packet(fat_packet);
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_DO_TURN:
        for (index = 0; index < packet->DoTurn.Count; index++) {
            turn_packet = &packet->DoTurn.Data[index];
            Client_Process_Do_Turn_Packet(turn_packet);
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_CRATE:
        for (index = 0; index < packet->Crate.Count; index++) {
            crate_packet = &packet->Crate.Data[index];
            cc = &Map[crate_packet->Cell];
            cc->Overlay = crate_packet->Overlay;
            cc->OverlayData = crate_packet->OverlayFrame;
            cc->Redraw_Objects();
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_PCP:
        for (index = 0; index < packet->PerCell.Count; index++) {
            pcp_packet = &packet->PerCell.Data[index];
            uc = (FootClass*)As_Object(pcp_packet->Whom);
            Process_Crate_Pickup(
                (WDTCrateType)pcp_packet->Number, pcp_packet->Cell, uc, pcp_packet->Owner, pcp_packet->_IntNumber);
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_TECHNO:
        for (index = 0; index < packet->Techno.Count; index++) {
            tech_packet = &packet->Techno.Data[index];
            tc = As_Techno(tech_packet->Whom);

            if (tc != NULL && tc->IsActive) {
                Client_Process_Techno_Packet(tc, tech_packet->Type, tech_packet->Data);
            }
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_SPECTATOR:
        if (packet->Spectator.House == PlayerPtr->Class->House) {
            Debug_Unshroud = true;
            Map.Activate(1);
            Add_WDT_Radar();
            Map.Flag_To_Redraw(true);
            Map.IsToDrawUnknown = true;
            Messages.Add_Message(
                Text_String(TXT_HAVE_BECOME_SPECTATOR), 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
            Map.Flag_To_Redraw(false);
        }

        Map.Color_List_Toggle_Spectator(HouseClass::As_Pointer(packet->Spectator.House)->Name, true);
        HouseClass::As_Pointer(packet->Spectator.House)->IsVisionary = true;
        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_GAME_END:
        Map.Color_List_Reset();
        SidebarClass::ColorListInstance1->Draw_Me(true);
        Victory_Dialog(packet->GameEnd.House);
        queue->UnQueue_Receive(0, 0, 0);

        if (SquadPostAcceptanceState == 2) {
            ClientEvent1_BattleState_Was_2 = true;
        }

        if (DebugLogTeams) {
            CCDebugString("*ClearTeamScore C:\n");
        }

        Clear_Team_Scores();
        break;

    case PACKET_SCENARIO_CHANGE:
        Clear_Team_Scores();
        GameOptionsBitfield = packet->Scenario._SomeBit;
        Client_Process_Change_Scenario(packet->Scenario.Scenario);

        if (!ClientEvent1_BattleState_Was_2) {
            Sound_Effect(VOC_RADAR_OFF);
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;

    case PACKET_MESSAGE: {
        char message[100];
        hc = HouseClass::As_Pointer(packet->Message.From);

        if (Options.TypingSound) {
            char* str1 = strupr(strdup(packet->Message.MessageBuffer)); // TODO Leak;
            char* str2 = strupr(strdup(PlayerPtr->Name));

            if (strstr(str1, str2)) {
                Sound_Effect(VOC_RAMBO_YO);
            }
        }

        if (packet->Message.From == HOUSE_ADMIN) {
            Sound_Effect(VOC_BLEEP);
            sprintf(message, Text_String(TXT_COMMAND_HQ_COLON), packet->Message.MessageBuffer);

            if (packet->Message.To == HOUSE_NONE) {
                Messages.Add_Message(message, 5, TPF_6POINT | TPF_FULLSHADOW | TPF_BRIGHT_COLOR, 0);
            } else {
                Messages.Add_Message(message, 210, TPF_6PT_GRAD | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
            }

            Client_Log_Player_Message(message);
        } else {
            if (packet->Message.Team == PlayerPtr->Int4 && packet->Message.Team != -1) {
                sprintf(message, "(%s): %s", hc->Name, packet->Message.MessageBuffer);
                Messages.Add_Message(
                    message, MPlayerTColors[packet->Message.Team + 1], TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
                Client_Log_Player_Message(message);
            } else {
                if (packet->Message.To == HOUSE_NONE) {
                    sprintf(message, "%s: %s", hc->Name, packet->Message.MessageBuffer);
                    Messages.Add_Message(message, 7, TPF_6PT_GRAD | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
                } else {
                    sprintf(message, "[%s]: %s", hc->Name, packet->Message.MessageBuffer);

                    if (Options.TypingSound) {
                        Sound_Effect(VOC_RAMBO_YO);
                    }

                    Messages.Add_Message(message, 210, TPF_6PT_GRAD | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);

                    if (Messages.Is_Private_Message()) {
                        Messages.Add_Private_Message(hc->Name);
                    }
                }

                Client_Log_Player_Message(message);
            }
        }

        Map.Flag_To_Redraw(false);
        queue->UnQueue_Receive(0, 0, 0);
        break;
    }
    case PACKET_SERVER_PASSWORD:
        switch (packet->ServerPassword.Status) {
        case 0:
            Messages.Add_Message(
                Text_String(TXT_NOT_CORRECT_PASSWORD), 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
            Map.Flag_To_Redraw(true);
            Sound_Effect(VOC_SCOLD, VOL_FULL);
            break;
        case 1:
            Messages.Add_Message(
                Text_String(TXT_CORRECT_PASSWORD), 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
            Map.Flag_To_Redraw(true);
            Sound_Effect(VOC_RAMBO_UGOTIT);
            SquadGamePasswordCountDown = 1;
            QuoteToShow = rand() % 64;
            SquadAcceptanceState = 2;
            break;
        case 2:
            Messages.Add_Message("Command HQ: A Squad Game is about to start! Enter the password.",
                                 5,
                                 TPF_6POINT | TPF_FULLSHADOW | TPF_BRIGHT_COLOR,
                                 0);
            Map.Flag_To_Redraw(true);
            Sound_Effect(VOC_BLEEP);
            SquadGameCountdownTimer.Reset(0);
            SquadGameCountdownTimer.Set(60 * GameParams.PasswordCountdownSeconds);
            SquadGamePasswordCountDown = 0;
            SquadAcceptanceState = 1;
            SquadPostAcceptanceState = 0;
            break;
        default:
            CCDebugString("Error: Invalid PACKET_SERVER_PASSWORD status received.\n");
            break;
        }

        queue->UnQueue_Receive(0, 0, 0);
        break;
    case PACKET_EVENT:
    case PACKET_GAME_OPTIONS:
    case PACKET_GAME_STATE:
    case PACKET_GAME_STATE_DONE:
    case PACKET_COMMAND_MESSAGE:
    default:
        Mono_Clear_Screen();
        Mono_Printf("UNKNOWN PACKET RECEIVED (Type:%d) (%s) !-!-!-!-!\n",
                    packet->Header.Type,
                    connectiontype == -1 ? "UDP" : "TCP");
        queue->UnQueue_Receive(0, 0, 0);
        break;
    }

    return true;
}

static int Client_Add_Compressed_Events(void* buf, int bufsize, int size);
extern int PacketLength[PACKET_COUNT];

void Client_Send_Event_Packet(void)
{
    int size;
    WDTPacketStruct packet;

    size = 0;
    EventClass* e = (EventClass*)packet.Event.Data;
    e[0].Type = EventClass::FRAMEINFO;
    e[0].ID = PlayerPtr->Class->House;
    size += 7;
    size = Client_Add_Compressed_Events(packet.Event.Data, MAX_PACKET_DATA_SIZE, size);
    packet.Header.Type = PACKET_EVENT;
    packet.Event.Size = size;

    ReliableProtocols[0]->Queue->Queue_Send(&packet, (PacketLength[PACKET_EVENT] - MAX_PACKET_DATA_SIZE) + size);
    ReliableComms[0]->Send();
    SentBytesSec += (PacketLength[PACKET_EVENT] - MAX_PACKET_DATA_SIZE) + size;
    SentTCP++;
}

/***************************************************************************
 * Add_Compressed_Events -- adds an compressed events to a packet          *
 *                                                                         *
 * INPUT:                                                                  *
 *		buf				buffer to store packet in										*
 *		bufsize			max size of buffer												*
 *		size				reference to current packet size								*
 *                                                                         *
 * OUTPUT:                                                                 *
 *		new size value																			*
 *                                                                         *
 * WARNINGS:                                                               *
 *		This routine MUST check to be sure it doesn't overflow the buffer.	*
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 DRD : Created.                                             *
 *=========================================================================*/
static int Client_Add_Compressed_Events(void* buf, int bufsize, int size)
{
    int num = 0;                     // # of events processed
    EventClass::EventType eventtype; // type of event being compressed
    EventClass prevevent;            // last event processed
    int datasize;                    // size of element plucked from event union
    int storedsize;                  // actual # bytes stored from event
    unsigned char* unitsptr = NULL;  // ptr to buffer pos to store mega. rep count
    unsigned char numunits = 0;      // megamission rep count value
    bool missiondup = false;         // flag: is this event a megamission repeat?

    //------------------------------------------------------------------------
    // clear previous event
    //------------------------------------------------------------------------
    memset(&prevevent, 0, sizeof(EventClass));

    //------------------------------------------------------------------------
    // Loop until there are no more events or the buffer is full.
    //------------------------------------------------------------------------
    while (OutList.Count) {
        eventtype = OutList.First().Type;
        datasize = EventClass::EventLength[eventtype];
        //.....................................................................
        // For a variable-sized event, pull the size from the event; otherwise,
        // the size will be the data element size plus the event type value.
        //.....................................................................
        storedsize = datasize + sizeof(EventClass::EventType);

        //.....................................................................
        // MegaMission compression:  MegaMissions are stored as:
        //   EventType
        //   Rep Count
        //   MegaMission structure (event # 1 only)
        //   Whom #2
        //   Whom #3
        //   Whom #4
        //   ...
        //   Whom #n
        //.....................................................................
        if (prevevent.Type == EventClass::MEGAMISSION) {
            //..................................................................
            // If previous & current events are both MegaMissions:
            //..................................................................
            if (eventtype == EventClass::MEGAMISSION) {
                //...............................................................
                // If the Mission, Target, & Destination are the same, compress
                // the events into one:
                // - Change datasize to the size of the 'Whom' field only
                // - set total # bytes to store to the size of the 'Whom' only
                // - increment the MegaMission rep count
                // - set the MegaMission rep flag
                //...............................................................
                if (OutList.First().Data.MegaMission.Mission == prevevent.Data.MegaMission.Mission
                    && OutList.First().Data.MegaMission.Target == prevevent.Data.MegaMission.Target
                    && OutList.First().Data.MegaMission.Destination == prevevent.Data.MegaMission.Destination) {

                    datasize = sizeof(prevevent.Data.MegaMission.Whom);
                    storedsize = datasize;
                    numunits++;
                    missiondup = true;
                }
                //...............................................................
                // Data doesn't match; start a new run of MegaMissions:
                // - Store previous MegaMission rep count
                // - Init 'unitsptr' to buffer pos after next EventType
                // - set total # bytes to store to 'datasize' + sizeof(EventType) +
                //   sizeof (numunits)
                // - init the MegaMission rep count to 1
                // - clear the MegaMission rep flag
                //...............................................................
                else {
                    *unitsptr = numunits;
                    unitsptr = ((unsigned char*)buf) + size + sizeof(EventClass::EventType);
                    storedsize += sizeof(numunits);
                    numunits = 1;
                    missiondup = false;
                }
            }
            //..................................................................
            // Previous event was a MegaMission, but this one isn't: end the
            // run of MegaMissions:
            // - Store previous MegaMission rep count
            // - Clear variables
            //..................................................................
            else {
                *unitsptr = numunits; // save # events in our run
                unitsptr = NULL;      // init other values
                numunits = 0;
                missiondup = false;
            }
        }

        //.....................................................................
        // The previous event is not a MEGAMISSION but the current event is:
        // Set up a new run of MegaMissions:
        // - Init 'unitsptr' to buffer pos after next EventType
        // - set total # bytes to store to 'datasize' + sizeof(EventType) +
        //   sizeof (numunits)
        // - init the MegaMission rep count to 1
        // - clear the MegaMission rep flag
        //.....................................................................
        else if (eventtype == EventClass::MEGAMISSION) {
            unitsptr = ((unsigned char*)buf) + size + sizeof(EventClass::EventType);
            storedsize += sizeof(numunits);
            numunits = 1;
            missiondup = false;
        }

        //.....................................................................
        // Will the next event exceed the size of the buffer?  If so,
        // stop compressing.
        //.....................................................................
        if ((size + storedsize) > bufsize)
            break;

        //.....................................................................
        // Set the event's ID
        //.....................................................................
        OutList.First().ID = PlayerPtr->Class->House;

        //.....................................................................
        // Transfer the event in OutList to DoList, un-queue the OutList event.
        // If the DoList is full, stop transferring immediately.
        //.....................................................................
        OutList.First().IsExecuted = 0;
        if (eventtype == EventClass::EXIT || eventtype == EventClass::OPTIONS || eventtype == EventClass::GAMESPEED
            || eventtype == EventClass::ANIMATION) {
            //if ( !DoList.Add( OutList.First() ) ) {
            //	break;
            //}
            DoList.Add(OutList.First());
            num++;
            memcpy(&prevevent, &OutList.First(), sizeof(EventClass));
            OutList.Next();
            continue;
        }
        //---------------------------------------------------------------------
        // Compress the event into the send packet buffer
        //---------------------------------------------------------------------
        switch (eventtype) {
        //..................................................................
        // MEGAMISSION:
        //..................................................................
        case (EventClass::MEGAMISSION):
            //...............................................................
            // Repeated mission in a run:
            //   - Update the rep count (in case we break out)
            //   - Copy the Whom field only
            //...............................................................
            if (missiondup) {
                *unitsptr = numunits;

                memcpy(((char*)buf) + size, &OutList.First().Data.MegaMission.Whom, datasize);

                size += datasize;
            }
            //...............................................................
            // 1st mission in a run:
            //   - Init the rep count (in case we break out)
            //   - Set the EventType
            //   - Copy the MegaMission structure, leaving room for 'numunits'
            //...............................................................
            else {
                *unitsptr = numunits;

                *(EventClass::EventType*)(((char*)buf) + size) = eventtype;

                memcpy(((char*)buf) + size + sizeof(EventClass::EventType) + sizeof(numunits),
                       &OutList.First().Data.MegaMission,
                       datasize);

                size += (datasize + sizeof(EventClass::EventType) + sizeof(numunits));
            }
            break;

        //..................................................................
        // Default case: Just copy over the data field from the union
        //..................................................................
        default:
            *(EventClass::EventType*)(((char*)buf) + size) = eventtype;

            memcpy(((char*)buf) + size + sizeof(EventClass::EventType), &OutList.First().Data, datasize);

            size += (datasize + sizeof(EventClass::EventType));

            break;
        }

        //---------------------------------------------------------------------
        // update # events processed
        //---------------------------------------------------------------------
        num++;

        //---------------------------------------------------------------------
        // Update 'prevevent'
        //---------------------------------------------------------------------
        memcpy(&prevevent, &OutList.First(), sizeof(EventClass));

        //---------------------------------------------------------------------
        // Go to the next event to process
        //---------------------------------------------------------------------
        OutList.Next();
    }

    return (size);

} // end of Add_Compressed_Events

ObjectClass* Client_Process_New_Object_Packet(NewDeletePacketData* packet)
{
    ObjectClass* obj;
    int value;
    TechnoClass* cell_obj;
    BuildingClass* bld;
    AircraftClass* aobj;
    CELL cell;
    KindType kind;

    obj = NULL;
    kind = Target_Kind(packet->Whom);
    value = Target_Value(packet->Whom);

    switch (kind) {
    case KIND_UNIT:
        UnitClass::Set_New_Allowed(true);
        obj = new (value) UnitClass((UnitType)packet->Type, (HousesType)packet->Owner);
        UnitClass::Set_New_Allowed(false);
        break;

    case KIND_INFANTRY:
        InfantryClass::Set_New_Allowed(true);
        obj = new (value) InfantryClass((InfantryType)packet->Type, (HousesType)packet->Owner);
        InfantryClass::Set_New_Allowed(false);
        break;

    case KIND_BUILDING:
        BuildingClass::Set_New_Allowed(true);
        obj = new (value) BuildingClass((StructType)packet->Type, (HousesType)packet->Owner);
        BuildingClass::Set_New_Allowed(false);
        break;

    case KIND_AIRCRAFT:
        AircraftClass::Set_New_Allowed(true);
        obj = new (value) AircraftClass((AircraftType)packet->Type, (HousesType)packet->Owner);
        AircraftClass::Set_New_Allowed(false);
        break;
    }

    if (obj) {
        if (packet->Coord != -1) {
            cell = Coord_Cell(packet->Coord);
            cell_obj = Map[cell].Cell_Techno();

            // Special case for an aircraft being created.
            if (obj->What_Am_I() == RTTI_AIRCRAFT && cell_obj && cell_obj->What_Am_I() == RTTI_BUILDING
                && ((BuildingClass*)cell_obj)->Class->Type == STRUCT_HELIPAD && cell_obj->Owner() == obj->Owner()) {
                aobj = (AircraftClass*)obj;
                bld = (BuildingClass*)cell_obj;
                ++ScenarioInit;
                aobj->Altitude = 0;
                aobj->Unlimbo(bld->Docking_Coord(), aobj->Pose_Dir());
                aobj->Assign_Mission(MISSION_GUARD);
                aobj->Transmit_Message(RADIO_HELLO, bld);
                bld->Transmit_Message(RADIO_TETHER);
                --ScenarioInit;
                return obj;
            }

            ++ScenarioInit;
            obj->Unlimbo(packet->Coord);
            --ScenarioInit;
        }

        if (obj->Is_Techno()) {
            ((TechnoClass*)obj)->Assign_Mission(packet->Mission);
            ((TechnoClass*)obj)->Commence();

            if (obj->What_Am_I() == RTTI_BUILDING) {
                if (packet->Mission == MISSION_CONSTRUCTION) {
                    ((BuildingClass*)obj)->BState = BSTATE_CONSTRUCTION;
                }
            }

            if (packet->Coord != -1) {
                if (cell_obj && cell_obj->Owner() == packet->Owner && cell_obj->What_Am_I() == RTTI_BUILDING) {
                    cell_obj->Exit_Object((TechnoClass*)obj);
                }
            }
        }

        if (GameParams.IsCrates) {
            obj->Mod1 = Scale_Value_Up(sole_array[SOLE_ARRAY_STRENGTH][2], packet->Strength);
            obj->Mod2 = Scale_Value_Up(sole_array[SOLE_ARRAY_SPEED][2], packet->Speed);
            obj->Mod3 = Scale_Value_Up(sole_array[SOLE_ARRAY_DAMAGE][2], packet->Damage);
            obj->Mod4 = Scale_Value_Up(sole_array[SOLE_ARRAY_ROF][2], packet->ROF);
            obj->Mod5 = Scale_Value_Up(sole_array[SOLE_ARRAY_RANGE][2], packet->Range);
        } else {
            obj->Mod1 = Get_Stat(SOLE_ARRAY_STRENGTH, sole_array[SOLE_ARRAY_STRENGTH][2], obj);
            obj->Mod2 = Get_Stat(SOLE_ARRAY_SPEED, sole_array[SOLE_ARRAY_SPEED][2], obj);
            obj->Mod3 = Get_Stat(SOLE_ARRAY_DAMAGE, sole_array[SOLE_ARRAY_DAMAGE][2], obj);
            obj->Mod4 = Get_Stat(SOLE_ARRAY_ROF, sole_array[SOLE_ARRAY_ROF][2], obj);
            obj->Mod5 = Get_Stat(SOLE_ARRAY_RANGE, sole_array[SOLE_ARRAY_RANGE][2], obj);
        }
        obj->Strength = obj->Class_Of().MaxStrength + obj->Mod1;
    }

    return obj;
}

void Client_Process_Delete_Object_Packet(NewDeletePacketData* packet)
{
    ObjectClass* obj = NULL;
    KindType kind = Target_Kind(packet->Whom);
    switch (kind) {
    case KIND_UNIT:
        UnitClass::Set_Delete_Allowed(true);
        obj = As_Unit(packet->Whom);
        delete obj;
        UnitClass::Set_Delete_Allowed(false);
        break;

    case KIND_INFANTRY:
        InfantryClass::Set_Delete_Allowed(true);
        obj = As_Infantry(packet->Whom);
        delete obj;
        InfantryClass::Set_Delete_Allowed(false);
        break;

    case KIND_BUILDING:
        BuildingClass::Set_Delete_Allowed(true);
        obj = As_Building(packet->Whom);
        delete obj;
        BuildingClass::Set_Delete_Allowed(false);
        break;

    case KIND_AIRCRAFT:
        AircraftClass::Set_Delete_Allowed(true);
        obj = As_Aircraft(packet->Whom);
        delete obj;
        AircraftClass::Set_Delete_Allowed(false);
        break;
    }
}

void Add_WDT_Radar(void);

void Client_Process_House_Packet(HouseUpdatePacketData* packet)
{
    HouseClass* house;
    FootClass* obj;
    house = HouseClass::As_Pointer(packet->House);

    if (house) {
        strcpy(house->Name, packet->PlayerName);
        house->Init_Data(packet->Color, packet->ActLike, packet->Credits);

        if (packet->House < HOUSE_BLUE_TEAM || packet->House > HOUSE_GREY_TEAM) {
            house->IsHuman = true;
        }

        if (stricmp(house->Name, Text_String(TXT_HUNTER)) != 0) {
            house->Int2 = packet->ScoredPoints;
        }

        house->Int3 = packet->TotalDeaths;
        if (packet->ActLike == HOUSE_BLUE_TEAM || packet->ActLike == HOUSE_ORANGE_TEAM
            || packet->ActLike == HOUSE_GREEN_TEAM || packet->ActLike == HOUSE_GREY_TEAM) {
            house->Int4 = packet->ActLike - 5;
        } else {
            house->Int4 = HOUSE_NONE;
        }

        house->IsVisionary = packet->IsVisionary;

        if (packet->House >= HOUSE_BLUE_TEAM && packet->House <= HOUSE_GREY_TEAM) {
            if (packet->FlagHome) {
                house->Make_CTF_Packet_Dropped(packet->FlagHome, true);
            }

            if (Target_Legal(packet->FlagLocation)) {
                if (Target_Kind(packet->FlagLocation) == KIND_CELL) {
                    house->Flag_Attach(As_Cell(packet->FlagLocation), true);
                } else {
                    obj = As_Unit(packet->FlagLocation);

                    if (obj && obj->IsActive) {
                        house->Flag_Attach(obj, true);
                    }
                }
            }
        }

        if (packet->House != HOUSE_ADMIN && house->IsHuman) {
            Map.Color_List_Add_Player(packet->PlayerName, packet->House);
            Map.Color_List_Update_Points(packet->PlayerName, packet->ScoredPoints);

            if (house->IsVisionary) {
                Map.Color_List_Toggle_Spectator(house->Name, 1);

                if (house == PlayerPtr) {
                    Debug_Unshroud = 1;
                    Map.Activate(1);
                    Add_WDT_Radar();
                    Map.Flag_To_Redraw(true);
                }
            }
        }
    }
}

void Client_Process_Health_Packet(HealthPacketData* packet)
{
    ObjectClass* obj;

    obj = As_Object(packet->Whom);

    if (!obj || !obj->IsActive)
        return;

    int health;
    if (obj->Strength > packet->Health) {
        health = obj->Strength - packet->Health;
        obj->Take_Damage(health, 0, WARHEAD_NONE, 0, true);
    }

    obj->Strength = packet->Health;

    if (obj->Strength < 2) {
        obj->Strength = 2;
    }

    obj->Mark(MARK_CHANGE);
}

void Client_Process_Damage_Packet(DamagePacketData* packet)
{
    TechnoClass* obj;
    int damage;
    TechnoClass* src;
    int rnd_pick;
    HouseClass* house;
    HousesType player;
    HousesType team;
    HousesType opponent;
    HousesType index;

    obj = NULL;
    KindType kind = Target_Kind(packet->Whom);

    switch (kind) {
    case KIND_UNIT:
        obj = As_Unit(packet->Whom);

        if (obj && obj->IsActive) {
            obj->Strength = 1;
            damage = 0x7FFF;

            if (packet->Source) {
                src = As_Techno(packet->Source);
            } else {
                src = NULL;
            }

            obj->TechnoUnk2 = false;
            ((UnitClass*)obj)->Take_Damage(damage, 0, packet->Warhead, src, true);
        }
        break;
    case KIND_INFANTRY:
        obj = As_Infantry(packet->Whom);

        if (obj && obj->IsActive) {
            obj->Strength = 1;
            damage = 0x7FFF;

            if (packet->Source) {
                src = As_Techno(packet->Source);
            } else {
                src = NULL;
            }

            obj->TechnoUnk2 = false;
            ((InfantryClass*)obj)->Take_Damage(damage, 0, packet->Warhead, src, true);
        }
        break;
    case KIND_BUILDING:
        obj = As_Building(packet->Whom);

        if (obj && obj->IsActive) {
            obj->Strength = 1;
            damage = 0x7FFF;

            if (packet->Source) {
                src = As_Techno(packet->Source);
            } else {
                src = NULL;
            }

            obj->TechnoUnk2 = false;
            ((BuildingClass*)obj)->Take_Damage(damage, 0, packet->Warhead, src, true);
        }
        break;
    case KIND_AIRCRAFT:
        obj = As_Aircraft(packet->Whom);

        if (obj && obj->IsActive) {
            obj->Strength = 1;
            damage = 0x7FFF;

            if (packet->Source) {
                src = As_Techno(packet->Source);
            } else {
                src = NULL;
            }

            obj->TechnoUnk2 = false;
            ((AircraftClass*)obj)->Take_Damage(damage, 0, packet->Warhead, src, true);
        }
        break;
    case KIND_TEAM: {
        player = (HousesType)Target_Value(packet->Source);
        team = HouseClass::As_Pointer(player)->ActLike;
        opponent = (HousesType)(Target_Value(packet->Whom) + HOUSE_FIRST_TEAM);
        Announce_Goal(player, team, opponent);
        int points = Calculate_Points(team);

        for (index = HOUSE_MULTI1; index < HOUSE_MULTI100; index++) {
            house = HouseClass::As_Pointer(index);

            if (house->Is_Ally(team) && house->IsHuman && house->IsActive
                && stricmp(house->Name, Text_String(TXT_HUNTER)) != 0) {
                if (GameParams.IsCaptureTheFlag) {
                    house->Int2 += 6 * points;
                    TeamPoints[team] += 6 * points;
                    Map.Color_List_Draw_Points(team - 6);
                    if (DebugLogTeams) {
                        char buffer[300];
                        sprintf(buffer,
                                "*Added %d to Team %d for player %s (flag cap)\n",
                                6 * points,
                                team - 6,
                                house->Name);
                        CCDebugString(buffer);
                    }
                } else {
                    house->Int2 += 3 * points;
                    TeamPoints[team] += 3 * points;
                    Map.Color_List_Draw_Points(team - 6);

                    if (DebugLogTeams) {
                        char buffer[300];
                        sprintf(
                            buffer, "*Added %d to Team %d for player %s (goal)\n", 3 * points, team - 6, house->Name);
                        CCDebugString(buffer);
                    }
                }

                Map.Color_List_Update_Points(house->Name, house->Int2);
                Map.Flag_To_Redraw();
            }

            if (index == player && stricmp(house->Name, Text_String(TXT_HUNTER)) != 0) {
                if (GameParams.IsCaptureTheFlag) {
                    house->Int2 += 3 * points;
                    TeamPoints[team] += 3 * points;
                    Map.Color_List_Draw_Points(team - 6);
                    if (DebugLogTeams) {
                        char buffer[300];
                        sprintf(buffer,
                                "*Added %d to Team %d for player %s (hero cap)\n",
                                3 * points,
                                team - 6,
                                house->Name);
                        CCDebugString(buffer);
                    }
                } else {
                    house->Int2 += points;
                    TeamPoints[team] += points;
                    Map.Color_List_Draw_Points(team - 6);
                    if (DebugLogTeams) {
                        char buffer[300];
                        sprintf(
                            buffer, "*Added %d to Team %d (hero goal) for player %s \n", points, team - 6, house->Name);
                        CCDebugString(buffer);
                    }
                }

                Map.Color_List_Update_Points(house->Name, house->Int2);
                Map.Flag_To_Redraw();
            }

            if (GameParams.IsCaptureTheFlag && (house->Is_Ally(team) || house->Is_Ally(opponent))) {
                ++house->Int3;
            }
        }

        break;
    }
    default:
        break;
    }

    if (obj && obj->IsActive && HouseClass::As_Pointer(obj->Owner()) == PlayerPtr) {
        rnd_pick = WDT_Random_Pick(0, 20);

        if (rnd_pick == 2) {
            VoiceThemes[CurrentVoiceTheme]->Play(VOX_THEME_SND_YOULOSE);
        }

        if (GameParams.LifeLimit > 0) {
            StatPanel.Enable();
        }
    }
}

void Client_Process_Squish_Packet(SquishPacketData* packet)
{
    TechnoClass* trgt1 = As_Techno(packet->field_0);
    TechnoClass* trgt2 = As_Techno(packet->field_4);

    if (!trgt1 || !trgt1->IsActive || !trgt2 || !trgt2->IsActive)
        return;

    Sound_Effect(VOC_SQUISH2, trgt1->Coord);
    trgt1->Record_The_Kill(trgt2);
    trgt1->Destruct();
    new OverlayClass(OVERLAY_SQUISH, Coord_Cell(trgt1->Coord));
}

void Client_Process_Capture_Packet(CapturePacketData* packet)
{
    TechnoClass* obj;
    KindType kind = Target_Kind(packet->Whom);
    switch (kind) {
    case KIND_UNIT:
        obj = As_Unit(packet->Whom);
        if (obj && obj->IsActive) {
            obj->Captured(HouseClass::As_Pointer((HousesType)packet->NewHouse), true);
        }
        break;

    case KIND_INFANTRY:
        obj = As_Infantry(packet->Whom);
        if (obj && obj->IsActive) {
            obj->Captured(HouseClass::As_Pointer((HousesType)packet->NewHouse), true);
        }
        break;

    case KIND_BUILDING:
        obj = As_Building(packet->Whom);
        if (obj && obj->IsActive) {
            obj->Captured(HouseClass::As_Pointer((HousesType)packet->NewHouse), true);
        }
        break;

    case KIND_AIRCRAFT:
        obj = As_Aircraft(packet->Whom);
        if (obj && obj->IsActive) {
            obj->Captured(HouseClass::As_Pointer((HousesType)packet->NewHouse), true);
        }
        break;
    }
}

void Client_Process_Cargo_Packet(CargoPacketData* packet)
{
    TechnoClass* trgt = As_Techno(packet->Whom);
    TechnoClass* cargo = As_Techno(packet->Cargo);

    if (trgt && trgt->IsActive && cargo && cargo->IsActive) {
        if (packet->State) {
            cargo->Add_To_Cargo(trgt, 1);
        } else {
            cargo->Remove_From_Cargo(trgt, 1);
        }
    }
}

void Client_Process_Flag_Packet(FlagPacketData* packet)
{
    UnitClass* uptr;
    InfantryClass* iptr;
    HouseClass* hptr;
    CELL cell;
    KindType kind;

    hptr = HouseClass::As_Pointer((HousesType)packet->House);
    kind = Target_Kind(packet->Whom);
    if (kind == KIND_CELL) {
        cell = As_Cell(packet->Whom);

        if (packet->Attached) {
            hptr->Flag_Attach(cell, 1);
        } else {
            hptr->Flag_Remove(packet->Whom, 1);
        }
    } else if (kind == KIND_UNIT) {
        uptr = (UnitClass*)As_Unit(packet->Whom);

        if (packet->Attached) {
            hptr->Flag_Attach(uptr, 1);
        } else {
            hptr->Flag_Remove(packet->Whom, 1);
        }
    } else if (kind == KIND_INFANTRY) {
        iptr = As_Infantry(packet->Whom);

        if (packet->Attached) {
            hptr->Flag_Attach(iptr, 1);
        } else {
            hptr->Flag_Remove(packet->Whom, 1);
        }
    }
}

void Client_Process_CTF_Packet(CTFPacketData* packet)
{
    HouseClass* hptr;
    CELL cell;

    hptr = HouseClass::As_Pointer((HousesType)packet->House);
    cell = packet->Cell;

    if (packet->State) {
        hptr->Make_CTF_Packet_Dropped(cell, true);
    } else {
        hptr->Make_CTF_Packet_Picked_Up(cell, true);
    }
}

void Client_Process_Infantry_Movement_Packet(MovePacketData* packet);
void Client_Process_Unit_Movement_Packet(MovePacketData* packet);

void Client_Process_Movement_Packet(MovePacketData* packet)
{
    KindType kind = Target_Kind(packet->Whom);
    switch (kind) {
    case KIND_AIRCRAFT:
    case KIND_UNIT:
        Client_Process_Unit_Movement_Packet(packet);
        break;

    case KIND_INFANTRY:
        Client_Process_Infantry_Movement_Packet(packet);
        break;
    }
}

void Client_Process_Unit_Movement_Packet(MovePacketData* packet)
{
    FootClass* obj;
    int i;
    CELL current_cell;
    KindType kind = Target_Kind(packet->Whom);

    switch (kind) {
    case KIND_UNIT:
        obj = As_Unit(packet->Whom);
        break;
    case KIND_AIRCRAFT:
        obj = As_Aircraft(packet->Whom);
        break;
    default:
        return;
    }

    if (obj && obj->IsActive) {
        if (obj->Head_To_Coord()) {
            current_cell = Coord_Cell(obj->Head_To_Coord());
        } else {
            current_cell = Coord_Cell(obj->Coord);
        }

        if (current_cell == packet->Cell) {
            memset(obj->Path, FACING_NONE, 9);
            obj->Path[0] = packet->Facing;
            obj->NavCom = As_Target(Adjacent_Cell(current_cell, packet->Facing));
        } else {
            i = 0;
            while (obj->Path[i] != FACING_NONE) {
                if (i >= 8) {
                    Client_Comm_Movement(obj, packet->Cell);
                    memset(obj->Path, FACING_NONE, 9);
                    obj->Path[0] = packet->Facing;
                    return;
                }

                current_cell = Adjacent_Cell(current_cell, obj->Path[i]);

                if (current_cell == packet->Cell) {
                    memset(&obj->Path[i + 1], FACING_NONE, 9 - (i + 1));
                    obj->Path[i + 1] = packet->Facing;
                    obj->NavCom = As_Target(Adjacent_Cell(current_cell, packet->Facing));
                    return;
                }
                i++;
            }

            Client_Comm_Movement(obj, packet->Cell);
            memset(obj->Path, FACING_NONE, 9);
            obj->Path[0] = packet->Facing;
            obj->NavCom = As_Target(Adjacent_Cell(packet->Cell, packet->Facing));
        }
    }
}

// Forward declare function defined later.
int Client_Process_Paths(CELL cell1, CELL cell2, FacingType* path, int length);

void Client_Process_Infantry_Movement_Packet(MovePacketData* packet)
{
    FootClass* obj;
    int start_step;
    int i;
    CELL current_cell;
    CELL adj_cell;

    obj = As_Infantry(packet->Whom);

    if (!obj || !obj->IsActive) {
        return;
    }

    if (obj->Head_To_Coord() && obj->Path[0] != FACING_NONE) {
        start_step = 1;
    } else {
        start_step = 0;
    }

    if (obj->Head_To_Coord()) {
        current_cell = Coord_Cell(obj->Head_To_Coord());
    } else {
        current_cell = Coord_Cell(obj->Coord);
    }

    if (current_cell == packet->Cell) {
        memset(&obj->Path[start_step], FACING_NONE, 9 - start_step);
        obj->Path[start_step] = packet->Facing;
        obj->NavCom = As_Target(Adjacent_Cell(current_cell, packet->Facing));
        return;
    }

    i = start_step;
    while (obj->Path[i] != FACING_NONE) {
        if (i >= 8) {
            Client_Comm_Movement(obj, packet->Cell);
            memset(obj->Path, FACING_NONE, 9);
            obj->Path[0] = packet->Facing;
            return;
        }

        current_cell = Adjacent_Cell(current_cell, obj->Path[i]);

        if (current_cell == packet->Cell) {
            memset(&obj->Path[i + 1], FACING_NONE, 9 - (i + 1));
            obj->Path[i + 1] = packet->Facing;
            obj->NavCom = As_Target(Adjacent_Cell(current_cell, packet->Facing));
            return;
        }
        i++;
    }

    if (obj->Head_To_Coord() && Coord_Cell(obj->Coord) == packet->Cell) {
        obj->Stop_Driver();
        memset(obj->Path, FACING_NONE, 9);
        obj->Path[0] = packet->Facing;
        obj->NavCom = As_Target(Adjacent_Cell(packet->Cell, packet->Facing));
        return;
    }

    current_cell = Coord_Cell(obj->Coord);
    adj_cell = Adjacent_Cell(packet->Cell, packet->Facing);
    obj->Stop_Driver();
    Client_Process_Paths(current_cell, adj_cell, obj->Path, 9);
    obj->NavCom = As_Target(adj_cell);
}

int Client_Process_Paths(CELL cell1, CELL cell2, FacingType* path, int length)
{
    FacingType f;
    int i;

    memset(path, FACING_NONE, length);
    CELL cell = cell1;
    i = 0;
    while (true) {
        if (cell == cell2) {
            break;
        }

        if (length - 1 <= i) {
            break;
        }
        f = Dir_Facing(Direction(cell, cell2));
        path[i] = f;
        i++;
        cell = Adjacent_Cell(cell, f);
    }
    return i;
}

void Client_Process_Target_Packet(TargetPacketData* packet)
{
    TechnoClass* whom = As_Techno(packet->Whom);

    if (!whom || !whom->IsActive)
        return;

    whom->Assign_Target(packet->Target, 1);
}

void Client_Process_Fire_At_Packet(FireAtPacketData* packet)
{
    TechnoClass* whom = As_Techno(packet->Whom);
    if (!whom || !whom->IsActive)
        return;

    ObjectClass* target = As_Object(packet->Target);

    if (target && !target->IsActive)
        return;

    whom->Fire_At(packet->Target, (unsigned char)packet->Which, 1);
}

void Client_Process_Do_Turn_Packet(DoTurnPacketData* packet)
{
    DriveClass* whom = (DriveClass*)As_Techno(packet->Whom);
    if (!whom || !whom->IsActive)
        return;

    whom->Do_Turn(packet->Dir);
}

void Client_Process_Techno_Packet(TechnoClass* obj, TechnoPacketDataType type, unsigned char value)
{
    switch (type) {
    case TECHNO_PACKET_DATA_0:
        if (value) {
            obj->TechnoUnk2 = true;
        } else {
            obj->TechnoUnk2 = false;
        }
        break;

    case TECHNO_PACKET_DATA_1:
        if (value) {
            obj->TechnoUnk3 = true;
            obj->IsCloakable = true;
        } else {
            obj->TechnoUnk3 = false;
            obj->Do_Uncloak();
            obj->IsCloakable = false;
        }
        break;

    case TECHNO_PACKET_DATA_ORANGE_CRATE:
        if (value) {
            obj->TechnoUnk4 = true;
        } else {
            //orange crate expired
            if (obj->House == PlayerPtr) {
                //inform player it expired
                new AnimClass(ANIM_CRATE_TIMEQK2, obj->Coord);
                Sound_Effect(VOC_MOTOR);
            }

            //restore strength to initial strength
            if (obj->Strength > obj->Class_Of().MaxStrength) {
                obj->Strength = obj->Class_Of().MaxStrength;
            }

            //clear all modifications to stats
            obj->Mod1 = 0;
            obj->Mod2 = 0;
            obj->Mod3 = 0;
            obj->Mod4 = 0;
            obj->Mod5 = 0;

            Map.Redraw_Tab();

            obj->TechnoUnk4 = false;
        }
        obj->Mark(MARK_CHANGE);

        break;

    case TECHNO_PACKET_DATA_STRENGTH:
        obj->Mod1 = Scale_Value_Up(sole_array[0][2], value);
        Map.Redraw_Tab();
        break;

    case TECHNO_PACKET_DATA_SPEED:
        obj->Mod2 = Scale_Value_Up(sole_array[2][2], value);
        Map.Redraw_Tab();
        break;

    case TECHNO_PACKET_DATA_DAMAGE:
        obj->Mod3 = Scale_Value_Up(sole_array[1][2], value);
        Map.Redraw_Tab();
        break;

    case TECHNO_PACKET_DATA_ROF:
        obj->Mod4 = Scale_Value_Up(sole_array[3][2], value);
        Map.Redraw_Tab();
        break;

    case TECHNO_PACKET_DATA_RANGE:
        obj->Mod5 = Scale_Value_Up(sole_array[4][2], value);
        Map.Redraw_Tab();
        break;

    case TECHNO_PACKET_DATA_CLOAKABLE:
        if (value) {
            obj->IsCloakable = true;
        } else {
            obj->Do_Uncloak();
            obj->IsCloakable = false;
        }
        break;

    case TECHNO_PACKET_DATA_DEMOLITION:
        if (value) {
            ((UnitClass*)obj)->IsGoingToBlow = true;
            ((UnitClass*)obj)->CountDown.Set(20);
        }
        break;
    }
}

bool Client_Process_Initial_Game_State_Packet(int size)
{
    WDTPacketStruct* wdt_packet;
    FootClass* obj;
    int current_size;
    int color;
    GameStatePacketData* packet;
    int packet_pos;
    int i;

    current_size = 0;
    packet_pos = 0;
    packet = (GameStatePacketData*)TempPacketBuffer;

    do {
        Keyboard->Check();

        if (ReliableProtocols[0]->Queue->Num_Receive() > 0) {
            wdt_packet = (WDTPacketStruct*)ReliableProtocols[0]->Queue->Get_Receive(0)->Buffer;

            if (wdt_packet->Header.Type != PACKET_GAME_STATE) {
                if (wdt_packet->Header.Type == PACKET_GAME_STATE_DONE) {
                    ReliableProtocols[0]->Queue->UnQueue_Receive(0, 0, 0);
                    return true;
                }

                while (Get_Mouse_State() > 1) {
                    Show_Mouse();
                }

                WWMessageBox().Process(TXT_INVALID_STATE_RECEIVED);
                Hide_Mouse();
                return false;
            }

            RecievedBytesSec += ReliableProtocols[0]->Queue->Get_Receive(0)->BufLen;
            RecievedTCP++;

            for (i = 0; i < wdt_packet->GameState.Size / sizeof(GameStatePacketData); i++) {
                packet[packet_pos] = wdt_packet->GameState.Data[i];
                packet_pos++;
                current_size += 18;
            }

            ReliableProtocols[0]->Queue->UnQueue_Receive(0, 0, 0);
        }
    } while (current_size < size);

    if (current_size > size) {
        WWMessageBox().Process(TXT_TOO_MANY_BYTES);
        return false;
    }

    NewDeletePacketData new_packet;

    for (i = 0; i < packet_pos; i++) {
        new_packet.IsDeletePacket = false;
        new_packet.Whom = packet[i].Whom;
        new_packet.Coord = packet[i].Coord;
        new_packet.Owner = packet[i].Owner;
        new_packet.Type = packet[i].Type;
        new_packet.Mission = MISSION_NONE;
        new_packet.Strength = packet[i].Strength;
        new_packet.Speed = packet[i].Speed;
        new_packet.Damage = packet[i].Damage;
        new_packet.ROF = packet[i].ROF;
        new_packet.Range = packet[i].Range;
        obj = (FootClass*)Client_Process_New_Object_Packet(&new_packet);

        if (obj) {
            obj->Strength = packet[i].Health;

            if (obj->Is_Techno()) {
                obj->IsCloakable = packet[i]._bit_C;

                if (!packet[i]._bit_C) {
                    obj->Cloak = UNCLOAKED;
                }
            }
        }
    }

    while (true) {
        Keyboard->Check();

        if (ReliableProtocols[0]->Queue->Num_Receive() > 0) {
            wdt_packet = (WDTPacketStruct*)ReliableProtocols[0]->Queue->Get_Receive(0)->Buffer;
            RecievedBytesSec += ReliableProtocols[0]->Queue->Get_Receive(0)->BufLen;
            RecievedTCP++;

            if (wdt_packet->Header.Type != PACKET_GAME_STATE_DONE) {
                WWMessageBox().Process(TXT_INVALID_STATE_DONE);
                return false;
            }
            ReliableProtocols[0]->Queue->UnQueue_Receive(0, 0, 0);
            break;
        }
    }

    char str[200];
    char message[200];
    char team[200];

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
    Map.Flag_To_Redraw(true);
    sprintf(str, "\r\n>>> Joined game at %s", Local_Time_As_String());
    Client_Log_Player_Message(str);

    if (GameParams.NumTeams > 1) {
        if (MPlayerHouse == HOUSE_BLUE_TEAM) {
            strcpy(team, Text_String(TXT_BLUE_TEAM));
            color = 0xFFFFFFDB;
        } else if (MPlayerHouse == HOUSE_ORANGE_TEAM) {
            strcpy(team, Text_String(TXT_ORANGE_TEAM));
            color = 0x19;
        } else if (MPlayerHouse == HOUSE_GREEN_TEAM) {
            strcpy(team, Text_String(TXT_GREEN_TEAM));
            color = 3;
        } else if (MPlayerHouse == HOUSE_GREY_TEAM) {
            strcpy(team, Text_String(TXT_GREY_TEAM));
            color = 0xFFFFFFD2;
        }

        sprintf(message, Text_String(TXT_COMMAND_HQ_WELCOME_TO), team, MPlayerName);
    } else {
        color = 15;
        sprintf(message, Text_String(TXT_COMMAND_HQ_WELCOME), MPlayerName);
    }

    Messages.Add_Message(message, color, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
    Map.Flag_To_Redraw(false);
    sprintf(message, Text_String(TXT_HELP_INFO_F1));
    Messages.Add_Message(message, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
    Map.Flag_To_Redraw(false);

    if (!GameParams.IsLadderGame) {
        sprintf(message, Text_String(TXT_ISNT_LADDER_GAME));
        Messages.Add_Message(message, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
        Map.Flag_To_Redraw(false);
    }

    if (MessageLogging) {
        sprintf(message, Text_String(TXT_MSG_LOGGING_ON_REMINDER));
        Messages.Add_Message(message, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
        Map.Flag_To_Redraw(false);
    }

    Messages.Add_Message(" ", 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
    return true;
}

void Client_Comm_Movement(FootClass* obj, CELL cell)
{
    if (obj->What_Am_I() == RTTI_INFANTRY || obj->What_Am_I() == RTTI_UNIT || obj->What_Am_I() == RTTI_AIRCRAFT) {
        obj->Stop_Driver();
        obj->NavCom = 0;
        obj->IsNewNavCom = false;
        obj->SuspendedNavCom = 0;

        if (obj->What_Am_I() == RTTI_UNIT) {
            ((DriveClass*)obj)->Reset_Track();
        }
    }

    if (obj->What_Am_I() == RTTI_INFANTRY) {
        ((InfantryClass*)obj)->Clear_Occupy_Bit(obj->Coord);
    }

    obj->Mark(MARK_UP);
    obj->Coord = Cell_Coord(cell);
    obj->Mark(MARK_DOWN);
    obj->Per_Cell_Process(true);
    obj->Look(0);
}

const char* Facing_List_As_String(FacingType* facing)
{
    static const char* const names[8] = {"N ", "NE ", "E ", "SE ", "S ", "SW ", "W ", "NW "};

    int i = 0;
    FacingString[0] = 0;

    //FACING_COUNT + 1... what the fuck
    while (facing[i] != FACING_NONE && i < (int)FACING_COUNT + 1) {
        strcat(FacingString, names[facing[i]]);
        i++;
    }

    strcat(FacingString, "X");
    return FacingString;
}

const char* Facing_Name(FacingType facing)
{
    static const char* const names[8] = {"N ", "NE ", "E ", "SE ", "S ", "SW ", "W ", "NW "};

    return names[facing];
}
extern bool Handle_Typed_Message(char* message);
extern bool Handle_Squad_Game_Typed_Message(const char* message);

void Client_Process_Message(void)
{
    WDTPacketStruct packet;
    char str[100];
    char privmsg[100];
    char* msg;
    int i;
    bool is_to;
    int size;
    int index;

    is_to = false;
    msg = Messages.Get_Edit_Buf();
    if (!strlen(msg)) {
        return;
    }

    if (Obfuscate(msg) == PARM_SECRET_CREDITS) {
        SpecialDialog = SDLG_SECRET_CREDITS;
        return;
    }

    if (*msg == '#') {
        is_to = true;
        IsTeamMessage = false;
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
            Messages.Add_Message(str, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
            Map.Flag_To_Redraw(false);
            return;
        }

        msg += strlen(privmsg) + 1;

        if (!strlen(msg)) {
            return;
        }

        index = 0;
        while (true) {
            if (index >= Houses.Count() || stricmp(Houses.Ptr(index)->Name, privmsg) == 0) {
                break;
            }

            index++;
        }

        if (Houses.Count() == index) {
            sprintf(str, Text_String(TXT_NO_PLAYER_NAMED), privmsg);
            Messages.Add_Message(str, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
            Map.Flag_To_Redraw(false);
            return;
        }

        Messages.Add_Private_Message(privmsg);
    } else {
        if (Handle_Typed_Message(msg)) {
            return;
        }

        if (Handle_Squad_Game_Typed_Message(msg)) {
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
    if (IsTeamMessage) {
        packet.Message.Team = PlayerPtr->Int4;
    } else {
        packet.Message.Team = -1;
    }

    strcpy(packet.Message.MessageBuffer, msg);
    size = (PacketLength[PACKET_MESSAGE] - 80) + strlen(msg) + 1;
    ReliableProtocols[0]->Queue->Queue_Send(&packet, size);
    ReliableComms[0]->Send();
    SentBytesSec += size;
    SentTCP++;

    if (IsTeamMessage) {
        sprintf(str, "(%s): %s", PlayerPtr->Name, msg);
        Messages.Add_Message(
            str, MPlayerTColors[packet.Message.Team + 1], TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
    } else if (is_to) {
        sprintf(str, "%s [to %s]: %s", PlayerPtr->Name, privmsg, msg);
        Messages.Add_Message(str, 210, TPF_6PT_GRAD | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
    } else {
        sprintf(str, "%s: %s", PlayerPtr->Name, msg);
        Messages.Add_Message(str, 7, TPF_6PT_GRAD | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
    }

    Client_Log_Player_Message(str);

    Map.Flag_To_Redraw(false);
}

void Client_Process_Change_Scenario(int scenario_index)
{
    char str[200];

    sprintf(str, "\r\n>>> Scenario Change at %s", Local_Time_As_String());
    Client_Log_Player_Message(str);

    bool had_sidebar = Map.IsSidebarActive;

    Scen.Scenario = scenario_index;

    Set_Scenario_Name(Scen.ScenarioName, Scen.Scenario, ScenPlayer, ScenDir, SCEN_VAR_A);

    Hide_Mouse();

    Fade_Palette_To(BlackPalette, 15, Call_Back);

    HiddenPage.Clear();
    VisiblePage.Clear();

    Remove_WDT_Radar();

    Debug_Unshroud = false;
    Frame = 0;

    if (Start_Scenario(Scen.ScenarioName)) {
        Fade_Palette_To(BlackPalette, 15, Call_Back);

        Map.Set_Cursor_Shape(NULL);
        Map.Set_Default_Mouse(MOUSE_NORMAL, false);

        Map.PendingObjectPtr = NULL;
        Map.PendingObject = NULL;
        Map.PendingHouse = HOUSE_NONE;

        Set_Logic_Page(HidPage);

        Map.Flag_To_Redraw(true);
        Map.IsToDrawUnknown = true;

        Map.Render();

        Fade_Palette_To(GamePalette, 15, 0);

        Show_Mouse();

        GameActive = true;

        DoList.Init();
        OutList.Init();

        Keyboard->Clear();

        InMainLoop = true;
        Clear_Packet_Data_Vectors();

        ClientFPS = 30;
        SpeedScale = 256;

        Options.GameSpeed = 2;
        LastClientFrame = 0;
        FramerateUpdateTimer.Set(120, true);
        RecievedBytesSec = 0;
        SentBytesSec = 0;
        SentTCP = 0;
        SentUDP = 0;
        RecievedTCP = 0;
        RecievedUDP = 0;
        TransmisionStatsTimer.Set(120, true);

        if (had_sidebar) {
            Map.Activate(true);
        }

        Map.Color_List_Clear();

        if (IsServerAdmin && !OfflineMode || PlayerPtr->Class->House == HOUSE_SPECTATOR) {
            Debug_Unshroud = true;
            Map.Activate(true);
            Add_WDT_Radar();
        }

        if (GameParams.FreeRadarForAll && !WDTRadarAdded) {
            Map.Activate(true);
            Add_WDT_Radar();
            PlayerPtr->IsUnk2 = true;
        }

        WDTGameTimer.Set(0, 1);

        if (SpecialDialog != SDLG_NONE) {
            Set_Logic_Page(SeenBuff);
            AllSurfaces.SurfacesRestored = true;
        }

        Map.Color_List_Add_Teams();

        if (SquadPostAcceptanceState == 1) {
            Messages.Add_Message("Command HQ: Let the Battle commence. May the Best Squad win!",
                                 5,
                                 TPF_6POINT | TPF_FULLSHADOW | TPF_BRIGHT_COLOR,
                                 0);
            Map.Flag_To_Redraw(true);
            Sound_Effect(VOC_BLEEP);
            SquadPostAcceptanceState = 2;
        }
    }
}

extern int SentBytesSec;
extern int SentTCP;
void Client_Send_Command(int state, int obfuscated)
{
    WDTPacketStruct packet;
    int length;

    packet.Header.Type = PACKET_COMMAND_MESSAGE;
    packet.Command.State = state;
    packet.Command.Obfuscated = obfuscated;
    length = PacketLength[PACKET_COMMAND_MESSAGE];
    ReliableProtocols[0]->Queue->Queue_Send(&packet, length);
    ReliableComms[0]->Send();
    SentBytesSec += length;
    ++SentTCP;
}

//Added in 1.04
bool Handle_Squad_Game_Typed_Message(const char* message)
{
    if (GameParams.IsSquadChannel && SquadGameCountdownTimer.Time() > 0) {
        if (!SquadGamePasswordCountDown) {
            int state = 1;
            int o = Obfuscate(message);
            Client_Send_Command(state, o);
        } else {
            Messages.Add_Message("Communication is not permitted during the countdown.",
                                 15,
                                 TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
                                 0);
            Map.Flag_To_Redraw(true);
            Sound_Effect(VOC_SCOLD, VOL_FULL);
        }

        return true;
    }

    return false;
}

//Added in 1.04
void Show_Quote(void)
{
    const char* _quotes[100 * 3] = {

        "\"The object of war is not to die for your country",
        " but to make the other bastard die for his.\"",
        "General George S. Patton",

        "",
        "\"War is based on deception.\"",
        "Sun Tzu",

        "\"It matters not whether you win or lose;",
        " what matters is whether I win or lose.\"",
        "Darrin Weinberg",

        "\"You can get more with a kind word and a gun",
        " than you can with a kind word alone.\"",
        "Al Capone",

        "",
        "\"I have nothing to offer but blood, toil, tears and sweat.\"",
        "Winston Churchill",

        "\"We are still masters of our fate.",
        " We are still captain of our souls.\"",
        "Winston Churchill",

        "",
        "\"...men will still say, This was their finest hour.\"",
        "Winston Churchill",

        "\"Never in the field of human conflict was",
        " so much owed by so many to so few.\"",
        "Winston Churchill",

        "",
        "\"We shall fight them on the beaches...\"",
        "Winston Churchill",

        "",
        "\"I shall return.\"",
        "General Douglas MacArthur",

        "",
        "\"...give me liberty or give me death!\"",
        "Patrick Henry",

        "",
        "\"Sink or swim, live or die, survive or perish.\"",
        "John Adams",

        "",
        "\"If a man does his best, what else is there?\"",
        "General George S. Patton",

        "",
        "\"Glory is fleeting, but obscurity is forever.\"",
        "Napoleon Bonaparte",

        "\"A man can't be too careful",
        " in the choice of his enemies.\"",
        "Oscar Wilde",

        "",
        "\"Forgive your enemies, but never forget their names.\"",
        "John F. Kennedy",

        "",
        "\"The graveyards are full of indispensable men.\"",
        "Charles de Gaulle",

        "\"Men and nations behave wisely once they",
        " have exhausted all the other alternatives.\"",
        "Abba Eban",

        "",
        "\"Opportunities multiply as they are seized.\"",
        "Sun Tzu",

        "\"A pessimist sees the difficulty in every opportunity;",
        " an optimist sees the opportunity in every difficulty.\"",
        "Winston Churchill",

        "\"The only thing necessary for the triumph of evil",
        " is for good men to do nothing.\"",
        "Edmund Burke",

        "",
        "\"The backbone of surprise is fusing speed with secrecy.\"",
        "Von Clausewitz",

        "",
        "\"A pint of sweat, saves a gallon of blood.\"",
        "General George S. Patton",

        "\"Its not the size of the dog in the fight,",
        " its the size of the fight in the dog.\"",
        "Mark Twain",

        "",
        "\"The more you sweat in peace, the less you bleed in war.\"",
        "Sun Tzu",

        "\"You must not fight too often with one enemy,",
        " or you will teach him all your art of war.\"",
        "Napoleon Bonaparte",

        "",
        "\"I only regret that I have but one life to lose for my country.\"",
        "Nathan Hale",

        "",
        "\"Let us die here rather than retreat.\"",
        "Alexander Hamilton",

        "",
        "\"I have not yet begun to fight!\"",
        "John Paul Jones",

        "",
        "\"I request you will witness to the world that I die like a brave man.\"",
        "John Andre",

        "",
        "\"Those who cannot remember the past are condemned to repeat it.\"",
        "George Santayana",

        "\"Some are born great; Some achieve greatness;",
        " And some have greatness thrust upon them.\"",
        "William Shakespeare",

        "",
        "\"Pick battles big enough to matter; small enough to win.\"",
        "Jonathan Kozol",

        "",
        "\"When you have to kill a man it costs nothing to be polite.\"",
        "Winston Churchill",

        "\"Supreme excellence consists in breaking",
        " the enemy's resistance without fighting.\"",
        "Sun Tzu",

        "",
        "\"I'm not a mercenary - killings more of a hobby with me.\"",
        "Unknown",

        "",
        "\"In war there is no substitute for victory.\"",
        "General Douglas MacArthur",

        "",
        "\"Cry havoc! Let slip the dogs of war!\"",
        "William Shakespeare",

        "\"It is well that war is so terrible,",
        " or we should get too fond of it.\"",
        "Robert E. Lee",

        "",
        "\"War is a series of catastrophes that results in a victory.\"",
        "Georges Clemenceau",

        "",
        "\"War is the science of destruction.\"",
        "John Abbott",

        "",
        "\"War is hell.\"",
        "William Tecumseh Sherman",

        "\"I don't know whether war is an interlude during peace,",
        " or peace is an interlude during war.\"",
        "Georges Clemenceau",

        "\"Stand your ground. Don't fire unless fired upon,",
        " but if they mean to have a war, let it begin here.\"",
        "John Parker",

        "",
        "\"...There is a time to fight, and that time has now come.\"",
        "Peter Muhlenberg",

        "\"Soldiers usually win the battles",
        " and generals get the credit for them.\"",
        "Napoleon Bonaparte",

        "\"The essence of war is violence.",
        " Moderation in war is imbecility.\"",
        "John A. Fisher",

        "",
        "\"No one can guarantee success in war, but only deserve it.\"",
        "Winston Churchill",

        "",
        "\"You cannot love a thing without wanting to fight for it.\"",
        "G. K. Chesterton",

        "\"If you are losing a tug-of-war with a tiger, give him the rope",
        " before he gets to your arm. You can always buy a new rope.\"",
        "Max Gunther",

        "",
        "\"He who attacks must vanquish. He who defends must merely survive.\"",
        "Master Kahn",

        "",
        "\"...make war upon them until you have wiped them out!\"",
        "Samuel 15:18",

        "",
        "\"Let us go boldly forward.\"",
        "Winston Churchill",

        "",
        "\"There are few problems that can't be solved with high explosives.\"",
        "Anonymous",

        "",
        "\"Never draw fire; it irritates the people around you.\"",
        "Anonymous",

        "",
        "\"War, that mad game the world so loves to play.\"",
        "Swift",

        "",
        "\"Ten good soldiers, wisely led, will beat a hundred without a head.\"",
        "D. W. Thompson",

        "",
        "\"It is said that God is always on the side of the heaviest battalions.\"",
        "Voltaire",

        "",
        "\"Send em to hell boys.\"",
        "John Wayne",

        "",
        "\"Veni, Vedi, Veci.\"",
        "Julius Caesar",

        "",
        "\"We made war to the end - to the very end of the end.\"",
        "Clemenceau",

        "\"The great questions of the day are not decided by speeches",
        " and majority votes, but by blood and iron.\"",
        "Bismarck",

        "",
        "\"Tis better to have fought and lost, than never to have fought at all.\"",
        "A. H. Clough",

        "\"War is elevation, because the individual disappears",
        " before the great conception of the state.\"",
        "Treitschke",
    };

    Conquer_Clip_Text_Print(_quotes[QuoteToShow * 3 + 0],
                            240,
                            300,
                            5,
                            0,
                            TPF_CENTER | TPF_6POINT | TPF_FULLSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    Conquer_Clip_Text_Print(_quotes[QuoteToShow * 3 + 1],
                            240,
                            315,
                            5,
                            0,
                            TPF_CENTER | TPF_6POINT | TPF_FULLSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);
    Conquer_Clip_Text_Print(_quotes[QuoteToShow * 3 + 2],
                            240,
                            330,
                            15,
                            0,
                            TPF_CENTER | TPF_6POINT | TPF_FULLSHADOW | TPF_BRIGHT_COLOR,
                            600,
                            0);

    Map.Redraw_Objects();
}
