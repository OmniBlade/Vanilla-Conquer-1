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
#ifndef GPROTOCOL_H
#define GPROTOCOL_H

#include <comm/comms.h>

class ListenerProtocolClass : public ProtocolClass
{
public:
	ListenerProtocolClass(void);

	virtual void Data_Received(void);
	virtual void Connected_To_Server(int status);
	virtual void Connection_Requested(void);
	virtual void Closed(void);
	virtual void Name_Resolved(void);
	virtual ~ListenerProtocolClass(void);

public:
	int ConnectionRequests;
};

class ReliableProtocolClass : public ProtocolClass
{
public:
	ReliableProtocolClass(void);

	virtual void Data_Received(void);
	virtual void Connected_To_Server(int status);
	virtual void Connection_Requested(void);
	virtual void Closed(void);
	virtual void Name_Resolved(void);
	virtual ~ReliableProtocolClass(void);

public:
	int ConnectionState;
	int NameResolved;
};

class UnreliableProtocolClass : public ProtocolClass
{
public:
	UnreliableProtocolClass(void);

	virtual void Data_Received(void);
	virtual void Connected_To_Server(int status);
	virtual void Connection_Requested(void);
	virtual void Closed(void);
	virtual void Name_Resolved(void);
	virtual ~UnreliableProtocolClass(void);
};

#endif
