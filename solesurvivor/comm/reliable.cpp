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

ReliableCommClass::ReliableCommClass(int maxpacketsize)
{
    Socket = INVALID_SOCKET;
    Protocol = NULL;
    Window = 0;
    Async = 0;
    Host.Addr.s_addr = INADDR_NONE;
    Host.DotAddr[0] = 0;
    Host.Name[0] = 0;
    Host.Port = 0;
    SendEntry = NULL;
    SendLen = 0;
    ReceiveBuf = NULL;
    ReceiveLen = 0;
    IsConnected = 0;
    MaxPacketSize = maxpacketsize;
    ReceiveBuf = new char[maxpacketsize];

    Create_Window();
}

ReliableCommClass::ReliableCommClass(ProtocolClass* protocol, int maxpacketsize)
{
    Socket = INVALID_SOCKET;
    Protocol = NULL;
    Window = 0;
    Async = 0;
    Host.Addr.s_addr = INADDR_NONE;
    Host.DotAddr[0] = 0;
    Host.Name[0] = 0;
    Host.Port = 0;
    SendEntry = NULL;
    SendLen = 0;
    ReceiveBuf = NULL;
    ReceiveLen = 0;
    IsConnected = 0;
    MaxPacketSize = maxpacketsize;

    ReceiveBuf = new char[maxpacketsize];

    Create_Window();
    Register(protocol);
}

ReliableCommClass::~ReliableCommClass()
{
    if (IsConnected) {
        Disconnect();
    }

    Destroy_Window();

    delete[] ReceiveBuf;
}

void ReliableCommClass::Register(ProtocolClass* protocol)
{
    Protocol = protocol;
}

int ReliableCommClass::Connect(void* addr, int len, int type)
{
    struct DestAddress* dest;
    struct sockaddr_in inaddr;

    if (Protocol == NULL) {
        return (0);
    }

    if (len != sizeof(struct DestAddress)) {
        return (0);
    }

    if (IsConnected) {
        Disconnect();
    }

    if (!Open_Socket()) {
        return (0);
    }

#if 0
    if (WSAAsyncSelect(Socket, Window, WM_ASYNCEVENT, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
        Close_Socket();
        return (0);
    }

    if (type != COMMS_TCPIP) {
        WSAAsyncSelect(Socket, Window, WM_ASYNCEVENT, 0);
        Close_Socket();
        return (0);
    }
#endif

    dest = (struct DestAddress*)addr;
    Host.Port = dest->Address.TCPIP.Port;
    Host.Addr.s_addr = inet_addr(dest->Address.TCPIP.Host);

    if (Host.Addr.s_addr != INADDR_NONE) {
        strcpy(Host.DotAddr, dest->Address.TCPIP.Host);
        Host.Name[0] = 0;
        #if 0
        Async = WSAAsyncGetHostByAddr(
            Window, WM_HOSTBYADDRESS, (char const*)&Host.Addr.s_addr, 4, PF_INET, Hbuf, MAXGETHOSTSTRUCT);
        if (Async == 0) {
            WSAAsyncSelect(Socket, Window, WM_ASYNCEVENT, 0);
            Close_Socket();
            return (0);
        }
        #endif

        inaddr.sin_family = AF_INET;
        inaddr.sin_port = htons(Host.Port);
        inaddr.sin_addr.s_addr = Host.Addr.s_addr;
        if (connect(Socket, (LPSOCKADDR)&inaddr, sizeof(inaddr)) == SOCKET_ERROR) {
            if (LastSocketError != WSAEWOULDBLOCK) {
#if 0
                WSACancelAsyncRequest(Async);
                Async = 0;
                WSAAsyncSelect(Socket, Window, WM_ASYNCEVENT, 0);
#endif
                Close_Socket();
                return (0);
            }
        }
    } else {
        strcpy(Host.Name, dest->Address.TCPIP.Host);
#if 0        
        Async = WSAAsyncGetHostByName(Window, WM_HOSTBYNAME, Host.Name, Hbuf, MAXGETHOSTSTRUCT);
        if (Async == 0) {
            WSAAsyncSelect(Socket, Window, WM_ASYNCEVENT, 0);
            Close_Socket();
            return (0);
        }
#endif
    }

    IsConnected = 1;

    return (1);
}

int ReliableCommClass::Connect(ListenerClass* listener)
{
    struct sockaddr_in addr;
    int addrsize;

    if (Protocol == NULL) {
        return (0);
    }

    if (IsConnected) {
        Disconnect();
    }

    addrsize = sizeof(addr);
    Socket = accept(listener->Socket, (LPSOCKADDR)&addr, &addrsize);
    if (Socket == INVALID_SOCKET) {
        return (0);
    }

#if 0
    if (WSAAsyncSelect(Socket, Window, WM_ASYNCEVENT, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
        Close_Socket();
        return (0);
    }
#endif

    Host.Addr.s_addr = addr.sin_addr.s_addr;
    strcpy(Host.DotAddr, inet_ntoa(addr.sin_addr));
    Host.Port = ntohs(addr.sin_port);
    Host.Name[0] = 0;

#if 0
    Async = WSAAsyncGetHostByAddr(
        Window, WM_HOSTBYADDRESS, (char const*)&Host.Addr.s_addr, 4, PF_INET, Hbuf, MAXGETHOSTSTRUCT);
    if (Async == 0) {
        WSAAsyncSelect(Socket, Window, WM_ASYNCEVENT, 0);
        Close_Socket();
        return (0);
    }
#endif

    IsConnected = 1;

    return (1);
}

void ReliableCommClass::Disconnect(void)
{
    if (IsConnected == 0) {
        return;
    }

#if 0
    if (Async) {
        WSACancelAsyncRequest(Async);
        Async = 0;
    }

    WSAAsyncSelect(Socket, Window, WM_ASYNCEVENT, 0);
#endif

    Close_Socket();

    Host.Name[0] = 0;
    Host.DotAddr[0] = 0;
    Host.Addr.s_addr = INADDR_NONE;
    Host.Port = 0;

    SendEntry = NULL;
    SendLen = 0;

    IsConnected = 0;
}

int ReliableCommClass::Send(void)
{
    unsigned short* len_ptr;

    if (Protocol == NULL || IsConnected == 0) {
        return (0);
    }

    if (SendEntry != NULL) {
        return (1);
    }

    SendEntry = Protocol->Queue->Get_Send(0);

    if (!SendEntry) {
        return (0);
    }

    SendLen = SendEntry->BufLen;

    len_ptr = (unsigned short*)SendEntry->Buffer;
    (*len_ptr) = htons((short)SendEntry->BufLen);

    PostMessageA(Window, WM_ASYNCEVENT, 0, (LONG)FD_WRITE);

    return (1);
}

unsigned short ReliableCommClass::Network_Short(unsigned short local_val)
{
    return (htons(local_val));
}

unsigned short ReliableCommClass::Local_Short(unsigned short net_val)
{
    return (ntohs(net_val));
}

unsigned long ReliableCommClass::Network_Long(unsigned long local_val)
{
    return (htonl(local_val));
}

unsigned long ReliableCommClass::Local_Long(unsigned long net_val)
{
    return (ntohl(net_val));
}

void ReliableCommClass::Create_Window(void)
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
    wndclass.lpszClassName = "RCommWin";

    RegisterClassA(&wndclass);

    Window = CreateWindowA("RCommWin",
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

void ReliableCommClass::Destroy_Window(void)
{
#if 0
    if (Window) {
        DestroyWindow(Window);
        Window = 0;
    }
#endif
}

int ReliableCommClass::Open_Socket(void)
{
    struct sockaddr_in addr;

    Socket = socket(AF_INET, SOCK_STREAM, 0);
    if (Socket == INVALID_SOCKET) {
        return (0);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(Socket, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR) {
        Close_Socket();
        return (0);
    }

    return (1);
}

void ReliableCommClass::Close_Socket(void)
{
    LINGER ling;

    if (Socket == INVALID_SOCKET) {
        return;
    }

    ling.l_onoff = 0;
    ling.l_linger = 0;
    setsockopt(Socket, SOL_SOCKET, SO_LINGER, (LPSTR)&ling, sizeof(ling));

    if (closesocket(Socket) == SOCKET_ERROR) {
        // TODO BUG? Nothing
    }

    Socket = INVALID_SOCKET;
}

#ifdef WIN32
long __stdcall ReliableCommClass::Window_Proc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
    static char const* _http_response = "HTTP/1.0 200 OK\nServer: NealScape/1.0\nContent-Type: "
                                        "text/html\n\n<title>Sole Survivor</title><h1>Sole Survivor "
                                        "is "
                                        "alive.</h1>HTTP Responder.";

    int rc;
    int get_request = 0;
    char* recbufptr2;
    CommStatsClass send_stats(0);
    CommStatsClass recv_stats(1);

    ReliableCommClass* obj;
    struct hostent* hentry;
    int event;
    unsigned short* len_ptr;
    unsigned short len;
    struct sockaddr_in addr;
    char* buf;

    obj = (ReliableCommClass*)GetWindowLongA(hwnd, GWL_USERDATA);

    switch (message) {
    case WM_CREATE:
        return (0);

    case WM_PAINT:
        return (0);

    case WM_DESTROY:
        if (obj->IsConnected) {
            obj->Disconnect();
        }
        return (0);

    case WM_COMMAND:
        return (0);

    case WM_HOSTBYADDRESS:
        if (WSAGETASYNCERROR(lParam) == 0) {
            hentry = (struct hostent*)obj->Hbuf;
            strcpy(obj->Host.Name, hentry->h_name);

            if (obj->Protocol) {
                obj->Protocol->Name_Resolved();
            }
        } else {
            obj->Host.Name[0] = 0;
        }
        obj->Async = 0;
        return (0);

    case WM_HOSTBYNAME:
        if (WSAGETASYNCERROR(lParam) == 0) {
            hentry = (struct hostent*)obj->Hbuf;
            memcpy(&(obj->Host.Addr.s_addr), hentry->h_addr, 4);
            strcpy(obj->Host.DotAddr, inet_ntoa(obj->Host.Addr));

            addr.sin_family = AF_INET;
            addr.sin_port = htons(obj->Host.Port);
            addr.sin_addr.s_addr = obj->Host.Addr.s_addr;
            if (connect(obj->Socket, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR) {
                if (LastSocketError != WSAEWOULDBLOCK) {
                    if (obj->Protocol) {
                        obj->Protocol->Connected_To_Server(0);
                    }
                    obj->Disconnect();
                    return (0);
                }
            }
        } else {
            if (obj->Protocol) {
                obj->Protocol->Connected_To_Server(0);
            }
            obj->Disconnect();
        }
        obj->Async = 0;
        return (0);

    case WM_ASYNCEVENT:
        event = WSAGETSELECTEVENT(lParam);
        switch (event) {
        case FD_CLOSE:
            if (obj->Protocol) {
                obj->Protocol->Closed();
            }
            obj->Disconnect();
            break;

        case FD_CONNECT:
            rc = WSAGETSELECTERROR(lParam);
            if (rc != 0) {
                if (obj->Protocol) {
                    obj->Protocol->Connected_To_Server(0);
                }
                obj->Disconnect();
                return (0);
            }

            if (obj->Protocol) {
                obj->Protocol->Connected_To_Server(1);
                if (strlen(obj->Host.Name) > 0) {
                    obj->Protocol->Name_Resolved();
                }
            }
            return (0);

        case FD_READ:
            rc = WSAGETSELECTERROR(lParam);
            if (rc != 0 && rc != WSAEWOULDBLOCK) {
                if (obj->Protocol) {
                    obj->Protocol->Closed();
                }
                obj->Disconnect();
                return (0);
            }

            while (1) {
                if (obj->ReceiveLen < 2) {
                    rc = recv(obj->Socket, obj->ReceiveBuf + obj->ReceiveLen, 2 - obj->ReceiveLen, 0);
                    if (rc == SOCKET_ERROR) {
                        if (LastSocketError != WSAEWOULDBLOCK) {
                            if (obj->Protocol) {
                                obj->Protocol->Closed();
                            }

                            obj->Disconnect();
                            return (0);
                        }
                        rc = 0;
                    }

                    obj->ReceiveLen += rc;
                    if (obj->ReceiveLen < 2) {
                        return (0);
                    }

                    len_ptr = (unsigned short*)obj->ReceiveBuf;
                    recbufptr2 = (char*)obj->ReceiveBuf;
                    if (strncmp(recbufptr2, "GE", sizeof(unsigned short)) != 0) {
                        len = ntohs(*len_ptr);
                        if (len > obj->MaxPacketSize) {
                            len = (unsigned short)obj->MaxPacketSize;
                        }
                        *len_ptr = len;
                        get_request = 0;
                    } else {
                        get_request = 1;
                    }
                }

                if (obj->ReceiveLen >= 2) {
                    len_ptr = (unsigned short*)obj->ReceiveBuf;
                    len = (*len_ptr);

                    if (len > obj->ReceiveLen) {

                        if (!get_request) {
                            rc = recv(obj->Socket, obj->ReceiveBuf + obj->ReceiveLen, len - obj->ReceiveLen, 0);

                            if (rc == SOCKET_ERROR) {
                                if (LastSocketError != WSAEWOULDBLOCK) {
                                    if (obj->Protocol) {
                                        obj->Protocol->Closed();
                                    }

                                    obj->Disconnect();
                                    return (0);
                                }
                                rc = 0;
                            }
                        } else if (get_request == 1) {
                            do {
                                rc = recv(obj->Socket, obj->ReceiveBuf, 10, 0);
                            } while (rc > 0);

                            rc = send(obj->Socket, _http_response, strlen(_http_response) + 1, 0);
                            obj->Disconnect();
                            return (0);
                        }
                        obj->ReceiveLen += rc;
                        recv_stats.Set1(rc);
                    }
                }

                if (obj->ReceiveLen == len) {
                    if (obj->Protocol) {
                        if (obj->Protocol->Queue->Num_Receive() == obj->Protocol->Queue->Max_Receive()) {
                            obj->Protocol->Queue->Grow_Receive(50);
                        }

                        rc = obj->Protocol->Queue->Queue_Receive(obj->ReceiveBuf, obj->ReceiveLen, NULL, 0);
                        if (rc) {
                            obj->Protocol->Data_Received();
                        }
                    }
                    obj->ReceiveLen = 0;

                    if (obj->Protocol == NULL) {
                        return (0);
                    }
                } else {
                    return (0);
                }
            }

        case FD_WRITE:
            if (!obj->SendEntry) {
                return (0);
            }
            rc = WSAGETSELECTERROR(lParam);
            if (rc != 0) {
                if (obj->Protocol) {
                    obj->Protocol->Closed();
                }
                obj->Disconnect();
                return (0);
            }

            buf = obj->SendEntry->Buffer;
            len_ptr = (unsigned short*)buf;
            len = ntohs(*len_ptr);
            while (obj->SendLen > 0) {
                rc = send(obj->Socket, buf + (len - obj->SendLen), obj->SendLen, 0);
                if (rc == SOCKET_ERROR) {
                    if (LastSocketError != WSAEWOULDBLOCK) {
                        if (obj->Protocol) {
                            obj->Protocol->Closed();
                        }
                        obj->Disconnect();
                        return (0);
                    }
                    break;
                } else {
                    send_stats.Set1(rc);
                    obj->SendLen -= rc;
                }
            }

            if (obj->SendLen == 0) {
                if (obj->Protocol) {
                    obj->Protocol->Queue->UnQueue_Send(NULL, NULL, 0, NULL, NULL);
                }
                obj->SendEntry = NULL;
                obj->Send();
            }
            break;
        }
        return (0);

    default:
        return DefWindowProcA(hwnd, message, wParam, lParam);
    }
}
#endif
