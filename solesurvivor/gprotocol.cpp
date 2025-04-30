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
#include "gprotocol.h"
#include "function.h"

ReliableProtocolClass::ReliableProtocolClass(void)
{
	ConnectionState = 0;
	NameResolved = false;

	if (GameToPlay == GAME_HOST) {
		Queue = new CommBufferClass(200, 200, MAX_PACKET_LEN, sizeof(struct DestAddress));
	} else {
		Queue = new CommBufferClass(200, 600, MAX_PACKET_LEN, sizeof(struct DestAddress));
	}
}

ReliableProtocolClass::~ReliableProtocolClass(void) 
{
	delete Queue;
}

void ReliableProtocolClass::Data_Received(void)
{
	//nothing
}

void ReliableProtocolClass::Connected_To_Server(int status)
{
	if (status == 1) {
		ConnectionState = 1;
	} else {
		ConnectionState = -1;
	}
}

void ReliableProtocolClass::Connection_Requested(void)
{
	//nothing
}

void ReliableProtocolClass::Closed(void)
{
	ConnectionState = 0;
}

void ReliableProtocolClass::Name_Resolved(void)
{
	NameResolved = true;
}

UnreliableProtocolClass::UnreliableProtocolClass(void)
{
	Queue = new CommBufferClass(50, 50, MAX_PACKET_LEN, sizeof(struct DestAddress));
}

UnreliableProtocolClass::~UnreliableProtocolClass(void) 
{
	delete Queue;
}

void UnreliableProtocolClass::Data_Received(void)
{
	//nothing
}

void UnreliableProtocolClass::Connected_To_Server(int /*status*/)
{
	//nothing
}

void UnreliableProtocolClass::Connection_Requested(void)
{
	//nothing
}

void UnreliableProtocolClass::Closed(void)
{
	//nothing
}

void UnreliableProtocolClass::Name_Resolved(void)
{
	//nothing
}
