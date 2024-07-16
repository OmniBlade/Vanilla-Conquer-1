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
#include "comms.h"
#include "sockets.h"

#ifdef WIN32
#define WM_HOSTBYADDRESS (WM_USER + 101)
#define WM_HOSTBYNAME    (WM_USER + 102)
#define WM_ASYNCEVENT    (WM_USER + 103)
extern HANDLE AppInstance;
#endif

UnreliableCommClass::UnreliableCommClass(int maxpacketsize, void* addr, int len, int type)
{
    unsigned short port;

    Socket = INVALID_SOCKET;
    Protocol = NULL;
    Window = 0;
    Async = 0;
    DefaultDest.Address.TCPIP.Host[0] = 0;
    DefaultDest.Address.TCPIP.Port = 0;
    DefaultAddr.Addr.s_addr = INADDR_NONE;
    DefaultAddr.DotAddr[0] = 0;
    DefaultAddr.Name[0] = 0;
    DefaultAddr.Port = 0;
    ReceiveBuf = NULL;
    ReceiveLen = 0;
    ReceiveAddr.Address.TCPIP.Host[0] = 0;
    ReceiveAddr.Address.TCPIP.Port = 0;
    DestResolved = 0;
    TryingSend = 0;
    ReceiveBuf = new char[maxpacketsize];
    ReceiveLen = maxpacketsize;

    Create_Window();

    if (addr != NULL && len == sizeof(struct DestAddress) && type == COMMS_TCPIP) {
        port = ((struct DestAddress*)addr)->Address.TCPIP.Port;
    } else {
        port = 0;
    }

    Open_Socket(port);
#if 0
    if (WSAAsyncSelect(Socket, Window, WM_ASYNCEVENT, FD_READ | FD_WRITE) == SOCKET_ERROR) {
        Close_Socket();
    }
#endif
}

UnreliableCommClass::UnreliableCommClass(ProtocolClass* protocol, int maxpacketsize, void* addr, int len, int type)
{
    unsigned short port;

    Socket = INVALID_SOCKET;
    Protocol = NULL;
    Window = 0;
    Async = 0;
    DefaultDest.Address.TCPIP.Host[0] = 0;
    DefaultDest.Address.TCPIP.Port = 0;
    DefaultAddr.Addr.s_addr = INADDR_NONE;
    DefaultAddr.DotAddr[0] = 0;
    DefaultAddr.Name[0] = 0;
    DefaultAddr.Port = 0;
    ReceiveBuf = NULL;
    ReceiveLen = 0;
    ReceiveAddr.Address.TCPIP.Host[0] = 0;
    ReceiveAddr.Address.TCPIP.Port = 0;
    DestResolved = 0;
    TryingSend = 0;
    ReceiveBuf = new char[maxpacketsize];
    ReceiveLen = maxpacketsize;
    Create_Window();

    if (addr != NULL && len == sizeof(struct DestAddress) && type == COMMS_TCPIP) {
        port = ((struct DestAddress*)addr)->Address.TCPIP.Port;
    } else {
        port = 0;
    }

    Open_Socket(port);

#if 0
    if (WSAAsyncSelect(Socket, Window, WM_ASYNCEVENT, FD_READ | FD_WRITE) == SOCKET_ERROR) {
        Close_Socket();
    }
#endif

    Register(protocol);
}

UnreliableCommClass::~UnreliableCommClass()
{
#if 0
    if (Async) {
        WSACancelAsyncRequest(Async);
        Async = 0;
    }

    WSAAsyncSelect(Socket, Window, WM_ASYNCEVENT, 0);
#endif

    Close_Socket();
    Destroy_Window();

    delete[] ReceiveBuf;
}

void UnreliableCommClass::Register(ProtocolClass* protocol)
{
    Protocol = protocol;
}

int UnreliableCommClass::Send(void)
{
    if (Protocol == NULL || Socket == INVALID_SOCKET) {
        return (0);
    }

    PostMessageA(Window, WM_ASYNCEVENT, 0, (LONG)FD_WRITE);

    return (1);
}

void UnreliableCommClass::Set_Default_Destination(void* addr, int len, int type)
{
    struct DestAddress* dest;

    if (type != COMMS_TCPIP || len != sizeof(struct DestAddress)) {
        return;
    }

    dest = (struct DestAddress*)addr;

    if (memcmp(dest, &DefaultDest, sizeof(struct DestAddress))) {
#if 0
        if (Async) {
            WSACancelAsyncRequest(Async);
            Async = 0;
        }
#endif
        memcpy(&DefaultDest, dest, sizeof(struct DestAddress));
        DestResolved = 0;
    }

    if (!DestResolved && !Async) {
        Resolve_Address();
    }
}

void UnreliableCommClass::Create_Window(void)
{
#if 0
    WNDCLASS wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = Window_Proc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = INSTANCE;
    wndclass.hIcon = LoadIconA(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "URCommWin";

    RegisterClassA(&wndclass);

    Window = CreateWindowA("URCommWin",
                          "Data Transmission",
                          (WS_OVERLAPPED | WS_SYSMENU) & (~WS_VISIBLE),
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          NULL,
                          NULL,
                          INSTANCE,
                          NULL);

    ShowWindow(Window, SW_HIDE);
    UpdateWindow(Window);
    SetWindowLongA(Window, GWL_USERDATA, (LONG)this);
#endif
}

void UnreliableCommClass::Destroy_Window(void)
{
#if 0
    if (Window) {
        DestroyWindow(Window);
        Window = 0;
    }
#endif
}

int UnreliableCommClass::Open_Socket(unsigned short port)
{
    struct sockaddr_in addr;

    Socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (Socket == INVALID_SOCKET) {
        return (0);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(Socket, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR) {
        Close_Socket();
        return (0);
    }

    return (1);
}

void UnreliableCommClass::Close_Socket(void)
{
    LINGER ling;

    if (Socket == INVALID_SOCKET) {
        return;
    }

    ling.l_onoff = 0;
    ling.l_linger = 0;
    setsockopt(Socket, SOL_SOCKET, SO_LINGER, (LPSTR)&ling, sizeof(ling));

    if (closesocket(Socket) == SOCKET_ERROR) {
        // TODO BUG, empty if?
    }

    Socket = INVALID_SOCKET;
}

#ifdef WIN32
long __stdcall UnreliableCommClass::Window_Proc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
    UnreliableCommClass* obj;
    int event;
    struct sockaddr_in addr;
    int addrlen;
    int rc;
    int len;
    struct DestAddress src_addr;
    SendQueueType* entry;
    struct hostent* hentry;

    obj = (UnreliableCommClass*)GetWindowLongA(hwnd, GWL_USERDATA);

    switch (message) {
    case WM_CREATE:
        return (0);

    case WM_PAINT:
        return (0);

    case WM_DESTROY:
        return (0);

    case WM_COMMAND:
        return (0);

    case WM_HOSTBYNAME:
        if (WSAGETASYNCERROR(lParam) != 0) {
            return (0);
        }
        hentry = (struct hostent*)obj->Hbuf;
        memcpy(&(obj->DefaultAddr.Addr.s_addr), hentry->h_addr, 4);
        strcpy(obj->DefaultAddr.DotAddr, inet_ntoa(obj->DefaultAddr.Addr));
        obj->DestResolved = 1;
        if (obj->TryingSend) {
            obj->TryingSend = 0;
            PostMessageA(hwnd, WM_ASYNCEVENT, 0, (LONG)FD_WRITE);
        }
        return (0);

    case WM_ASYNCEVENT:
        event = WSAGETSELECTEVENT(lParam);
        switch (event) {
        case FD_READ:
            addrlen = sizeof(addr);
            len = recvfrom(obj->Socket, obj->ReceiveBuf, obj->ReceiveLen, 0, (LPSOCKADDR)&addr, &addrlen);
            if (len == SOCKET_ERROR) {
                return (0);
            }

            if (obj->Protocol) {
                strcpy(src_addr.Address.TCPIP.Host, inet_ntoa(addr.sin_addr));
                src_addr.Address.TCPIP.Port = ntohs(addr.sin_port);
                rc = obj->Protocol->Queue->Queue_Receive(obj->ReceiveBuf, len, &src_addr, sizeof(src_addr));
                if (rc) {
                    obj->Protocol->Data_Received();
                }
            }
            return (0);

        case FD_WRITE:
            if (!obj->Protocol) {
                return (0);
            }

            entry = obj->Protocol->Queue->Get_Send(0);
            if (!entry) {
                return (0);
            }

            if (entry->ExtraLen > 0) {
                if (memcmp(&obj->DefaultDest, entry->ExtraBuffer, sizeof(struct DestAddress))) {
                    memcpy(&obj->DefaultDest, entry->ExtraBuffer, sizeof(struct DestAddress));
                    obj->Resolve_Address();
                }
            }

            if (!obj->DestResolved) {
                obj->TryingSend = 1;
                return (0);
            }

            addr.sin_family = AF_INET;
            addr.sin_port = htons(obj->DefaultAddr.Port);
            addr.sin_addr = obj->DefaultAddr.Addr;
            rc = sendto(obj->Socket, entry->Buffer, entry->BufLen, 0, (LPSOCKADDR)&addr, sizeof(addr));

            if (rc == SOCKET_ERROR) {
                if (WSAGetLastError() != WSAEWOULDBLOCK) {
                    return (0);
                }
            }

            obj->Protocol->Queue->UnQueue_Send(NULL, NULL, 0);
            PostMessageA(hwnd, WM_ASYNCEVENT, 0, (LONG)FD_WRITE);
            return (0);

        default:
            break;
        }

    default:
        return (DefWindowProcA(hwnd, message, wParam, lParam));
    }
}
#endif

void UnreliableCommClass::Resolve_Address(void)
{
    if (Async) {
        return;
    }

    DestResolved = 0;
    DefaultAddr.Addr.s_addr = inet_addr(DefaultDest.Address.TCPIP.Host);

    if (DefaultAddr.Addr.s_addr != INADDR_NONE) {
        strcpy(DefaultAddr.DotAddr, DefaultDest.Address.TCPIP.Host);
        DefaultAddr.Name[0] = 0;
        DefaultAddr.Port = DefaultDest.Address.TCPIP.Port;
        DestResolved = 1;
    } else {
        DefaultAddr.DotAddr[0] = 0;
        strcpy(DefaultAddr.Name, DefaultDest.Address.TCPIP.Host);
        DefaultAddr.Port = DefaultDest.Address.TCPIP.Port;
#if 0
        Async = WSAAsyncGetHostByName(Window, WM_HOSTBYNAME, DefaultAddr.Name, Hbuf, MAXGETHOSTSTRUCT);
#endif
    }
}
