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

/* $Header:   F:\projects\c&c\vcs\code\msglist.cpv   1.4   16 Oct 1995 16:48:20   JOE_BOSTIC  $ */
/***************************************************************************
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : MSGLIST.CPP                              *
 *                                                                         *
 *                   Programmer : Bill R. Randolph                         *
 *                                                                         *
 *                   Start Date : 05/22/95                                 *
 *                                                                         *
 *                  Last Update : June 26, 1995 [BRR]                      *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   MessageListClass::Add_Edit -- Adds editable string to message list    *
 *   MessageListClass::Add_Message -- displays the given message           *
 *   MessageListClass::Draw -- Draws the messages                          *
 *   MessageListClass::Get_Edit_Buf -- gets edit buffer                    *
 *   MessageListClass::Init -- Inits message system, sets options          *
 *   MessageListClass::Input -- Handles input for sending messages         *
 *   MessageListClass::Manage -- Manages multiplayer messages              *
 *   MessageListClass::MessageListClass -- constructor                     *
 *   MessageListClass::~MessageListClass -- destructor                     *
 *   MessageListClass::Num_Messages -- returns # messages in the list      *
 *   MessageListClass::Set_Width -- sets allowable width of messages       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"
#include "langfilt.h"

// ST = 12/17/2018 5:44PM
//#ifndef WinTickCount
//extern TimerClass WinTickCount;
//#endif

char MessageListClass::MessageBuffers[MAX_NUM_MESSAGES][MAX_MESSAGE_LENGTH];
int MessageColors[MAX_NUM_MESSAGES];
TextPrintType MessageStyles[MAX_NUM_MESSAGES];
int MessageTimings[MAX_NUM_MESSAGES];

char MessageListClass::EditBuf[MAX_MESSAGE_LENGTH];
char MessageListClass::ToBuf[MAX_MESSAGE_LENGTH];

char MessageListClass::PrivateMessageBuffer[52];

LanguageFilterClass MessageFilter;
bool MessageFilterInited;

/***************************************************************************
 * MessageListClass::MessageListClass -- constructor                       *
 *                                                                         *
 * INPUT:                                                                  *
 *      x,y         coord of upper-left of top message                     *
 *      max_msg      max messages allowed, including edit message          *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/21/1995 BRR : Created.                                             *
 *=========================================================================*/
MessageListClass::MessageListClass(void)
{
    int i;

	MessageX = 0;
	MessageY = 0;
	MaxMessages = 0;
	MessageIndex = 0;
	MaxChars = 0;
	Height = 0;
	IsEditing = false;
	EditCurPos = 0;
	EditCurX = 0;
	EditCurY = 0;
	EditCurStyle = TPF_NOSHADOW;
	EditCurColor = TBLACK;
	ToRedraw = false;

	PrivateMessageBuffer[0] = '\0';

	for (i = 0; i < MAX_NUM_MESSAGES; i++) {
		MessageTiming[i] = -1;
	}

	MessageTimingIndex = 0;
}

/***************************************************************************
 * MessageListClass::~MessageListClass -- destructor                       *
 *                                                                         *
 * INPUT:                                                                  *
 *      x,y         coord of upper-left of top message                     *
 *      max_msg      max messages allowed, including edit message          *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/21/1995 BRR : Created.                                             *
 *=========================================================================*/
MessageListClass::~MessageListClass()
{
    Init(0, 0, 0, 0, 0);
}

/***************************************************************************
 * MessageListClass::Init -- Inits message system, sets options            *
 *                                                                         *
 * INPUT:                                                                  *
 *      x,y         coord of upper-left of top message                     *
 *      max_msg      max messages allowed, including edit message          *
 *      maxchars      max # characters allowed per message                 *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/21/1995 BRR : Created.                                             *
 *=========================================================================*/
void MessageListClass::Init(int x, int y, int max_msg, int maxchars, int height)
{
    /*------------------------------------------------------------------------
	Init variables
	------------------------------------------------------------------------*/
	MessageIndex = 0;
	IsEditing = false;
	EditCurPos = 0;
	MessageX = x;
	MessageY = y;

	MaxMessages = max_msg;
	if (MaxMessages > MAX_NUM_MESSAGES)
		MaxMessages = MAX_NUM_MESSAGES;
	MaxChars = maxchars;
	if (MaxChars > MAX_MESSAGE_LENGTH)
		MaxChars = MAX_MESSAGE_LENGTH;

	Height = height;

	PrivateMessageBuffer[0] = '\0';
}

/***************************************************************************
 * MessageListClass::Add_Message -- displays the given message             *
 *                                                                         *
 * INPUT:                                                                  *
 *      txt         text to display                                        *
 *      color         color to draw text in                                *
 *      style         style to use                                         *
 *      timeout      # of ticks the thing is supposed to last (-1 = forever)*
 *                                                                         *
 * OUTPUT:                                                                 *
 *      ptr to new TextLabelClass object.                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 * The TextLabelClass's text buffer is free'd when the class is free'd,    *
 * so never pass it a static buffer.                                       *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/05/1995 BRR : Created.                                             *
 *=========================================================================*/
void MessageListClass::Add_Message(const char *txt, int color, TextPrintType style, int timeout)
{
	if (MessageIndex == MaxMessages) {
		MessageListClass::Move_Old_Messages(0);
	}

	//strcpy?
	strncpy(MessageBuffers[MessageIndex], txt, MAX_MESSAGE_LENGTH - 1);
	MessageBuffers[MessageIndex][MAX_MESSAGE_LENGTH - 1] = 0;

	//BUG reloads every time as LanguageFilterInited isn't flagged
	if (!MessageFilterInited) {
		MessageFilter.Init("nl.cfg");
	}

	MessageFilter.Filter(MessageBuffers[MessageIndex], 3);
	MessageColors[MessageIndex] = color;
	MessageStyles[MessageIndex] = style;

	if ( timeout > 0 ) {
		MessageTimings[MessageIndex] = WinTickCount.Time() + timeout;
	} else {
		MessageTimings[MessageIndex] = 0;
	}
	MessageIndex++;
	ToRedraw = true;
}

void MessageListClass::Move_Old_Messages(int index)
{
	if (MessageIndex && index < MessageIndex && index >= 0) {
		for (int i = index; MessageIndex - 1 > i; i++)
		{
			strcpy(MessageBuffers[i], MessageBuffers[i + 1]);
			MessageColors[i] = MessageColors[i + 1];
			MessageStyles[i] = MessageStyles[i + 1];
			MessageTimings[i] = MessageTimings[i + 1];
		}
		MessageIndex--;
		ToRedraw = true;
	}
}

/***************************************************************************
 * MessageListClass::Add_Edit -- Adds editable string to message list      *
 *                                                                         *
 * INPUT:                                                                  *
 *      color         color of edit message                                *
 *      style         style of edit message                                *
 *      to            string: who to send to                               *
 *      width			width of editbox in pixels                           *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      ptr to new TextLabelClass                                          *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/22/1995 BRR : Created.                                             *
 *=========================================================================*/
void MessageListClass::Add_Edit(int x, int y, int color, TextPrintType style, const char *to)
{
	IsEditing = true;

	EditCurPos = 0;
	memset(EditBuf, 0, MAX_MESSAGE_LENGTH);
	EditCurX = x;
	EditCurY = y;
	EditCurColor = color;
	EditCurStyle = style;

	strcpy(ToBuf, to);

	ToRedraw = true;
}

/***************************************************************************
 * MessageListClass::Manage -- Manages multiplayer messages                *
 *                                                                         *
 * If this routine returns TRUE, the caller should update the display.     *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      0 = no change has occurred, 1 = changed                            *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/05/1995 BRR : Created.                                             *
 *=========================================================================*/
int MessageListClass::Manage (void)
{
	int i;
	bool changed = false;

	for (i = 0; i < MessageIndex; i++) {
		/*.....................................................................
		If this message's time is up, remove it from the list
		.....................................................................*/
		if (MessageTimings[i] != 0 && WinTickCount.Time() > MessageTimings[i]) {
			Move_Old_Messages(i);
			i--;
			changed = true;
			ToRedraw = true;
		}
	}
	
	return(changed);
}

/***************************************************************************
 * MessageListClass::Input -- Handles input for sending messages           *
 *                                                                         *
 * INPUT:                                                                  *
 *      input         key value to process                                 *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = caller should redraw the message list (no need to complete     *
 *        refresh, though)                                                 *
 *      2 = caller should completely refresh the display.                  *
 *      3 = caller should send the edit message.                           *
 *      (sets 'input' to 0 if it processes it.)                            *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/05/1995 BRR : Created.                                             *
 *=========================================================================*/
int MessageListClass::Input(KeyNumType &input)
{
	int ascii;
	int retcode = 0;

	/*------------------------------------------------------------------------
	Do nothing if nothing to do.
	------------------------------------------------------------------------*/
	if (input == KN_NONE)
		return(0);
	/*------------------------------------------------------------------------
	Leave mouse events alone.
	------------------------------------------------------------------------*/
	if ( (input & (~KN_RLSE_BIT))==KN_LMOUSE ||
		(input & (~KN_RLSE_BIT))==KN_RMOUSE)
		return(0);
	/*------------------------------------------------------------------------
	If we're in 'edit mode', handle keys
	------------------------------------------------------------------------*/
	if (IsEditing) {
		int timing3;
		int timing2;
		int timing1;
		int time;

		ascii = Keyboard->To_ASCII(input);

		switch (ascii) {
			/*------------------------------------------------------------------
			ESC = abort message
			------------------------------------------------------------------*/
			case KA_ESC:				
				IsEditing = false;
				input = KN_NONE;
				ToRedraw = true;
				Sound_Effect(VOC_SCOLD, VOL_2);
				break;

			/*------------------------------------------------------------------
			RETURN = send the message
			------------------------------------------------------------------*/
			case KA_RETURN:
				time = WinTickCount.Time();
				timing1 = MessageTimingIndex + 1;

				if (timing1 > 9) {
					timing1 = 0;
				}

				timing2 = MessageTiming[timing1];

				if (timing2 == -1) {
					timing3 = 1000000;
				} else {
					timing3 = time - timing2;
				}

				// SquadGameCountdownTimer check added in 1.04
				if (IsServerAdmin || OfflineMode || timing3 > 60000 || SquadGameCountdownTimer.Time() > 0) {
					IsEditing = false;
					retcode = 1;
					input = KN_NONE;
					ToRedraw = true;
					Sound_Effect(VOC_SIDEBAR_OPEN, VOL_2);
					MessageTiming[MessageTimingIndex] = time;
					MessageTimingIndex++;

					if (MessageTimingIndex > 9) {
						MessageTimingIndex = 0;
					}

				} else {
					MessageListClass::Add_Message(Text_String(TXT_DONT_FLOOD_MESSAGE_AREA), 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
					Map.Flag_To_Redraw(false);
					Sound_Effect(VOC_SCOLD, VOL_2);
				}
				break;

			/*------------------------------------------------------------------
			BACKSPACE = remove a character
			------------------------------------------------------------------*/
			case KA_BACKSPACE:
				if (EditCurPos > 0) {
					EditCurPos--;
					EditBuf[EditCurPos] = 0;
					ToRedraw = true;
					if (Options.TypingSound) {
						Sound_Effect(VOC_BUTTON, VOL_3);
					}
				}
				input = KN_NONE;
				break;

			/*------------------------------------------------------------------
			default: add a character.  Reserve the last buffer position for null.
			EditCurPos is the buffer index # of the next
			character, after the "To:" prefix.
			------------------------------------------------------------------*/
			default:
				//no idea what
		        if (input == 0x1120) {
					ascii = ' ';
				}
				//whats 0x7FFF..
				//why did <= 127 lose =.....
				if (EditCurPos < (MaxChars - 1) && input < 0x7FFF && ((input == 0x1120 || (input & WWKEY_VK_BIT) && ascii >='0' && ascii <= '9') || !(input & WWKEY_VK_BIT) && ascii >= ' ' && ascii < 127)) {
					EditBuf[EditCurPos] = ascii;
					EditCurPos++;
					ToRedraw = true;
					if (Options.TypingSound) {
						Sound_Effect(VOC_DOWN, VOL_2);
					}
				}
				input = KN_NONE;
				break;
		}
	}
	return(retcode);
}

void MessageListClass::Add_Team_Message(int index)
{
	int team = PlayerPtr->ActLike - 6;
	if ( team >= 0 && team <= 3 ) {
		Sound_Effect(VOC_RELOAD, VOL_FULL);
		Messages.Add_Edit(3, 464, MPlayerTColors[team + 2], TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, "Team Message: ");
		strcpy(EditBuf, TeamMessages[index]);
		EditCurPos += strlen(TeamMessages[index]);
		ToRedraw = true;
		Map.Flag_To_Redraw(false);
		IsTeamMessage = true;
	}
}

void MessageListClass::Make_Message_Private(void)
{
	strcpy(EditBuf, PrivateMessageBuffer);
	EditCurPos += strlen(PrivateMessageBuffer);
	ToRedraw = true;
}

void MessageListClass::Add_Private_Message(const char *message)
{
	sprintf(PrivateMessageBuffer, "#%s ", message);
}

/***************************************************************************
 * MessageListClass::Draw -- draws messages                                *
 *                                                                         *
 * INPUT:                                                                  *
 *      none                                                               *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/22/1995 BRR : Created.                                             *
 *=========================================================================*/
void MessageListClass::Draw(GraphicViewPortClass &viewport)
{
	int i;
	GraphicViewPortClass *oldpage;

	if (ToRedraw) {
		Hide_Mouse();

		oldpage = Set_Logic_Page(viewport);

		for (i = 0; i < MessageIndex; i++) {
			int y = MessageY + i * Height;
			Conquer_Clip_Text_Print(MessageBuffers[i], MessageX, y, MessageColors[i], 0, MessageStyles[i], 600, 0);
		}

		if (IsEditing) {
			//draw who's it to
			Conquer_Clip_Text_Print(ToBuf, EditCurX, EditCurY, EditCurColor, 0, EditCurStyle, 600, 0);
			//draw actual message
			Conquer_Clip_Text_Print(EditBuf, EditCurX + String_Pixel_Width(ToBuf), EditCurY, EditCurColor, 0, EditCurStyle, 600, 0);
		}

		Set_Logic_Page(oldpage);

		Show_Mouse();
		ToRedraw = false; 
	}

}
