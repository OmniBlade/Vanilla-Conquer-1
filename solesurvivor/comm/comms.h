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
#ifndef COMMCLAS_H
#define COMMCLAS_H

#include "common/sockets.h"
#include "protocol.h"

class CommStatsClass
{
public:
    CommStatsClass(char c);
    ~CommStatsClass(void);

    int Set1(int a1);
    int Get1(void);
    int Set2(int a1);
    void Get2(int** a2, int** a3, int** a4, unsigned int** a5);

private:
    char Char1;
    int Member1;
    int Member2;
    int Member3;
    unsigned int Member4;

    static int Static11;
    static int Static01;

    static int Static12;
    static int Static02;

    static int Static13;
    static int Static03;

    static unsigned int Static14;
    static unsigned int Static04;
};

enum DestTypeTag
{
    COMMS_TCPIP,
    COMMS_IPX,
    COMMS_MODEM
};

#define MAX_COMM_HOSTNAME 30
#define MAX_COMM_PHONENUM 30

struct DestAddress
{
    union
    {
        struct
        {
            char Host[MAX_COMM_HOSTNAME];
            unsigned short Port;
        } TCPIP;
        struct
        {
            char Net[4];
            char Node[6];
        } IPX;
        struct
        {
            char Number[MAX_COMM_PHONENUM];
        } Modem;
    } Address;
};

struct TCPHost
{
    struct in_addr Addr;
    char DotAddr[16];
    char Name[255];
    unsigned short Port;
};

class ReliableCommClass;

class ListenerClass
{
    friend class ReliableCommClass;

public:
    ListenerClass(void);
    ListenerClass(ProtocolClass* protocol);
    virtual ~ListenerClass();
    virtual void Register(ProtocolClass* protocol);
    virtual int Start_Listening(void* addr, int len, int type);
    virtual void Stop_Listening(void);

private:
    void Create_Window(void);
    void Destroy_Window(void);
    int Open_Socket(unsigned short port);
    void Close_Socket(void);

#ifdef WIN32
    static long __stdcall Window_Proc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
#endif

    SOCKET Socket;
    ProtocolClass* Protocol;
    HWND Window;
    int IsListening : 1;
};

class ReliableCommClass
{
public:
    ReliableCommClass(int maxpacketsize = 500);
    ReliableCommClass(ProtocolClass* protocol, int maxpacketsize = 500);
    virtual ~ReliableCommClass();

    virtual void Register(ProtocolClass* protocol);
    virtual int Connect(void* addr, int len, int type);
    virtual int Connect(ListenerClass* listener);
    virtual void Disconnect(void);
    virtual int Send(void);

    static unsigned short Network_Short(unsigned short local_val);
    static unsigned short Local_Short(unsigned short net_val);
    static unsigned long Network_Long(unsigned long local_val);
    static unsigned long Local_Long(unsigned long net_val);

    struct TCPHost Host;

private:
    void Create_Window(void);
    void Destroy_Window(void);
    int Open_Socket(void);
    void Close_Socket(void);

#ifdef WIN32
    static long __stdcall Window_Proc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
#endif

    int MaxPacketSize;
    SOCKET Socket;
    ProtocolClass* Protocol;

#ifdef WIN32
    HWND Window;
    HANDLE Async;
#endif

    char Hbuf[MAXGETHOSTSTRUCT];
    SendQueueType* SendEntry;
    int SendLen;
    char* ReceiveBuf;
    int ReceiveLen;
    int IsConnected : 1;
};

class UnreliableCommClass
{
public:
    UnreliableCommClass(int maxpacketsize = 500, void* addr = NULL, int len = 0, int type = COMMS_TCPIP);
    UnreliableCommClass(ProtocolClass* protocol,
                        int maxpacketsize = 500,
                        void* addr = NULL,
                        int len = 0,
                        int type = COMMS_TCPIP);
    virtual ~UnreliableCommClass();

    virtual void Register(ProtocolClass* protocol);
    virtual int Send(void);
    virtual void Set_Default_Destination(void* addr, int len, int type);

private:
    void Create_Window(void);
    void Destroy_Window(void);

    int Open_Socket(unsigned short port);
    void Close_Socket(void);

#ifdef WIN32
    static long __stdcall Window_Proc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
#endif

    void Resolve_Address(void);

    SOCKET Socket;
    ProtocolClass* Protocol;

#ifdef WIN32
    HWND Window;
    HANDLE Async;
#endif
    char Hbuf[MAXGETHOSTSTRUCT];
    struct DestAddress DefaultDest;
    struct TCPHost DefaultAddr;
    char* ReceiveBuf;
    int ReceiveLen;
    struct DestAddress ReceiveAddr;
    int DestResolved : 1;
    int TryingSend : 1;
};

int Init_Comms_API(void);
void Shutdown_Comms_API(void);

#endif