/* $Header:   F:\projects\c&c0\vcs\code\queue.cpv   2.24   11 Oct 1995 13:47:40   JOE_BOSTIC  $ */
/***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : QUEUE.CPP                                *
 *                                                                         *
 *                   Programmer : Bill R. Randolph                         *
 *                                                                         *
 *                   Start Date : 11/28/95                                 *
 *                                                                         *
 *                  Last Update : November 28, 1995 [BRR]                  *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions for Queueing Events:                                          *
 *   Queue_Mission -- Queue a mega mission event.                          *
 *   Queue_Options -- Queue the options event.                             *
 *   Queue_Exit -- Add the exit game event to the queue.                   *
 *                                                                         *
 * Functions for processing Queued Events:											*
 *   Queue_AI -- Process all queued events.                                *
 *   Queue_AI_Normal -- Process all queued events.                         *
 *   Queue_AI_Multiplayer -- Process all queued events.                    *
 *                                                                         *
 * Main Multiplayer Queue Logic:															*
 *   Wait_For_Players -- Waits for other systems to come on-line           *
 *   Generate_Timing_Event -- computes & queues a RESPONSE_TIME event      *
 *   Process_Send_Period -- timing for sending packets every 'n' frames    *
 *   Send_Packets -- sends out events from the OutList                     *
 *   Send_FrameSync -- Sends a FRAMESYNC packet                            *
 *   Process_Receive_Packet -- processes an incoming packet                *
 *   Process_Serial_Packet -- Handles an incoming serial packet            *
 *   Can_Advance -- determines if it's OK to advance to the next frame     *
 *   Process_Reconnect_Dialog -- processes the reconnection dialog         *
 *   Handle_Timeout -- attempts to reconnect; if fails, bails.             *
 *   Stop_Game -- stops the game															*
 *                                                                         *
 * Packet Compression / Decompression:													*
 *   Build_Send_Packet -- Builds a big packet from a bunch of little ones.	*
 *   Add_Uncompressed_Events -- adds uncompressed events to a packet       *
 *   Add_Compressed_Events -- adds compressed events to a packet        	*
 *   Breakup_Receive_Packet -- Splits a big packet into little ones.			*
 *   Extract_Uncompressed_Events -- extracts events from a packet				*
 *   Extract_Compressed_Events -- extracts events from a packet            *
 *                                                                         *
 * DoList Management:																		*
 *   Execute_DoList -- Executes commands from the DoList                   *
 *   Clean_DoList -- Cleans out old events from the DoList                 *
 *   Queue_Record -- Records the DoList to disk                            *
 *   Queue_Playback -- plays back queue entries from a record file         *
 *                                                                         *
 * Debugging:																					*
 *   Compute_Game_CRC -- Computes a CRC value of the entire game.				*
 *   Add_CRC -- Adds a value to a CRC                                      *
 *   Print_CRCs -- Prints a data file for finding Sync Bugs						*
 *   Init_Queue_Mono -- inits mono display                                 *
 *   Update_Queue_Mono -- updates mono display                             *
 *   Print_Framesync_Values -- displays frame-sync variables               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include "function.h"
//#include 	"tcpip.h"

/********************************** Defines *********************************/
#define SHOW_MONO 1

#if 0
int tmp_flag = 0;

/********************************** Globals *********************************/
//---------------------------------------------------------------------------
//	GameCRC is the current computed CRC value for this frame.
//	CRC[] is a record of our last 32 game CRC's.
// ColorNames is for debug output in Print_CRCs
//---------------------------------------------------------------------------
#ifndef DEMO
static unsigned long GameCRC;
static unsigned long CRC[32] =
	{0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0};

static char *ColorNames[6] = {
	"Yellow",
	"Red",
	"BlueGreen",
	"Orange",
	"Green",
	"Blue",
};
#endif //DEMO

//...........................................................................
// Mono debugging variables:
// NetMonoMode: 0 = show connection output, 1 = flowcount output
// NewMonoMode: set by anything that toggles NetMonoMode; re-inits screen
// IsMono: used for taking control of Mono screen away from the engine
//...........................................................................
#ifndef DEMO
int NetMonoMode = 1;
int NewMonoMode = 1;
static int IsMono = 0;
#endif //DEMO
#endif

//---------------------------------------------------------------------------
// Several routines return various codes; here's an enum for all of them.
//---------------------------------------------------------------------------
typedef enum RetcodeEnum
{
    RC_NORMAL,            // no news is good news
    RC_PLAYER_READY,      // a new player has been heard from
    RC_SCENARIO_MISMATCH, // scenario mismatch
    RC_DOLIST_FULL,       // DoList is full
    RC_SERIAL_PROCESSED,  // modem: SERIAL packet was processed
    RC_PLAYER_LEFT,       // modem: other player left the game
    RC_HUNG_UP,           // modem has hung up
    RC_NOT_RESPONDING,    // other player not responding (timeout/hung up)
    RC_CANCEL,            // user cancelled
} RetcodeType;

/********************************* Prototypes *******************************/
//...........................................................................
// Main multiplayer queue logic
//...........................................................................
extern void Queue_AI_Normal(void);
extern void Client_Queue_AI(void);
extern void Host_Queue_AI(void);

/***************************************************************************
 * Queue_AI -- Process all queued events.                                  *
 *                                                                         *
 * INPUT:                                                                  *
 *		none.																						*
 *                                                                         *
 * OUTPUT:                                                                 *
 *		none.																						*
 *                                                                         *
 * WARNINGS:                                                               *
 *		none.																						*
 *                                                                         *
 * HISTORY:                                                                *
 *   09/21/1995 JLB : Created.                                             *
 *=========================================================================*/
void Queue_AI(void)
{
    switch (GameToPlay) {

    case GAME_NORMAL:
        Queue_AI_Normal();
        break;
    case GAME_CLIENT:
        Client_Queue_AI();
        break;
    case GAME_HOST:
        Host_Queue_AI();
        break;
    }
} /* end of Queue_AI */

/***************************************************************************
 * Queue_AI_Normal -- Process all queued events.                           *
 *                                                                         *
 * This is the "normal" version of the queue management routine.  It does 	*
 * the following:		  																		*
 * - Transfers items in the OutList to the DoList									*
 * - Executes any commands in the DoList that are supposed to be done on 	*
 *   this frame #			  																	*
 * - Cleans out the DoList																	*
 *                                                                         *
 * INPUT:                                                                  *
 *		none.																						*
 *                                                                         *
 * OUTPUT:                                                                 *
 *		none.																						*
 *                                                                         *
 * WARNINGS:                                                               *
 *		none.																						*
 *                                                                         *
 * HISTORY:                                                                *
 *   09/21/1995 JLB : Created.                                             *
 *=========================================================================*/
void Queue_AI_Normal(void)
{
    //------------------------------------------------------------------------
    //	Move events from the OutList (events generated by this player) into the
    //	DoList (the list of events to execute).
    //------------------------------------------------------------------------
    while (OutList.Count) {
        OutList.First().IsExecuted = false;
        if (!DoList.Add(OutList.First())) {
            ;
        }
        OutList.Next();
    }

    //------------------------------------------------------------------------
    // Execute the DoList
    //------------------------------------------------------------------------
    for (int i = 0; i < DoList.Count; i++) {
        if (!DoList[i].IsExecuted) {
            DoList[i].Execute();         // execute it
            DoList[i].IsExecuted = true; // mark as having been executed
            DoList.Next();
        }
    }

} /* end of Queue_AI_Normal */

/***************************************************************************
 * Queue_Mission -- Queue a mega mission event.                            *
 *                                                                         *
 * This routine is called when the player causes a change to a game unit. 	*
 * The event that initiates the change is queued to as a result of a call 	*
 * to this routine.                 													*
 *                                                                         *
 * INPUT:                                                                  *
 *		whom		Whom this mission request applies to (a friendly unit).     *
 *    mission	The mission to assign to this object.                       *
 *    target	The target of this mission (if any).                        *
 *    dest		The movement destination for this mission (if any).         *
 *                                                                         *
 * OUTPUT:                                                                 *
 *		Was the mission request queued successfully?                         *
 *                                                                         *
 * WARNINGS:                                                               *
 *		none.                                                                *
 *                                                                         *
 * HISTORY:                                                                *
 *   09/21/1995 JLB : Created.                                             *
 *=========================================================================*/
bool Queue_Mission(TARGET whom, MissionType mission, TARGET target, TARGET destination)
{
    if (!OutList.Add(EventClass(whom, mission, target, destination))) {
        return (false);
    } else {
        return (true);
    }

} /* end of Queue_Mission */

/***************************************************************************
 * Queue_Options -- Queue the options event.                               *
 *                                                                         *
 * INPUT:                                                                  *
 *		none.																						*
 *                                                                         *
 * OUTPUT:                                                                 *
 *		Was the options screen event queued successfully?                    *
 *                                                                         *
 * WARNINGS:                                                               *
 *		none.																						*
 *                                                                         *
 * HISTORY:                                                                *
 *   09/21/1995 JLB : Created.                                             *
 *=========================================================================*/
bool Queue_Options(void)
{
    if (!OutList.Add(EventClass(EventClass::OPTIONS))) {
        return (false);
    } else {
        return (true);
    }

} /* end of Queue_Options */

/***************************************************************************
 * Queue_Exit -- Add the exit game event to the queue.                     *
 *                                                                         *
 * INPUT:                                                                  *
 *		none.																						*
 *                                                                         *
 * OUTPUT:                                                                 *
 *		Was the exit event queued successfully?                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *		none.																						*
 *                                                                         *
 * HISTORY:                                                                *
 *   09/21/1995 JLB : Created.                                             *
 *=========================================================================*/
bool Queue_Exit(void)
{
    if (!OutList.Add(EventClass(EventClass::EXIT))) {
        return (false);
    } else {
        return (true);
    }

} /* end of Queue_Exit */

/***************************************************************************
 * Add_CRC -- Adds a value to a CRC                                        *
 *                                                                         *
 * INPUT:                                                                  *
 *		crc		ptr to crc																	*
 *		val		value to add																*
 *                                                                         *
 * OUTPUT:                                                                 *
 *		none																						*
 *                                                                         *
 * WARNINGS:                                                               *
 *		none																						*
 *                                                                         *
 * HISTORY:                                                                *
 *   05/09/1995 BRR : Created.                                             *
 *=========================================================================*/
void OLD_Add_CRC(unsigned long* crc, unsigned long val)
{
    int hibit;

    if ((*crc) & 0x80000000) {
        hibit = 1;
    } else {
        hibit = 0;
    }

    (*crc) <<= 1;
    (*crc) += val;
    (*crc) += hibit;

} /* end of Add_CRC */

/*************************** end of queue.cpp ******************************/