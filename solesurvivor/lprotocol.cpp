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
#include "gprotocol.h"

ListenerProtocolClass::ListenerProtocolClass(void)
{
	ConnectionRequests = 0;
	Queue = new CommBufferClass(100, 100, MAX_PACKET_LEN, sizeof(struct DestAddress));
}

ListenerProtocolClass::~ListenerProtocolClass(void) 
{
	delete Queue;
}

void ListenerProtocolClass::Data_Received(void)
{
	//nothing
}

void ListenerProtocolClass::Connected_To_Server(int)
{
	//nothing
}

void ListenerProtocolClass::Connection_Requested(void)
{
	ConnectionRequests++;
}

void ListenerProtocolClass::Closed(void)
{
	//nothing
}

void ListenerProtocolClass::Name_Resolved(void)
{
	//nothing
}
