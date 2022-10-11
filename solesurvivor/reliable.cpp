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
#include "reliable.h"
#include "listener.h"
#include "common/debugstring.h"
#include "common/endianness.h"
#include <algorithm>

#if defined _WIN32 && !defined SDL2_BUILD
#define SSWM_RELIABLE_COMM_MSG (WM_USER + 0x67)
#define SSWM_HOSTBYADDR_MSG    (WM_USER + 0x65)
#define SSWM_HOSTBYNAME_MSG    (WM_USER + 0x66)
#else
fd_set ReliableCommClass::MonitoringSet;
fd_set ReliableCommClass::ReadSet;
fd_set ReliableCommClass::WriteSet;
fd_set ReliableCommClass::ErrorSet;
SOCKET ReliableCommClass::HighSocket = INVALID_SOCKET;
#endif

ReliableProtocolClass::ReliableProtocolClass()
    : Connected(0)
    , NameResolved(0)
{
    if (GameToPlay == GAME_HOST) {
        Queue = new CommBufferClass(200, 200, 422, 32);
    } else {
        Queue = new CommBufferClass(200, 600, 422, 32);
    }
}

ReliableCommClass::ReliableCommClass(int maxpacketsize)
{
    Socket = -1;
    Protocol = 0;
#if defined _WIN32 && !defined SDL2_BUILD
    Window = 0;
    Async = 0;
#endif
    Host.Addr = -1;
    Host.DotAddr[0] = 0;
    Host.Name[0] = 0;
    Host.Port = 0;
    SendEntry = 0;
    SendLen = 0;
    ReceiveBuf = 0;
    ReceiveLen = 0;
    IsConnected = false;
    MaxPacketSize = maxpacketsize;
    ReceiveBuf = new char[maxpacketsize];
#if defined _WIN32 && !defined SDL2_BUILD
    Create_Window();
#endif
}

ReliableCommClass::ReliableCommClass(ProtocolClass* protocol, int maxpacketsize)
{
    Socket = -1;
    Protocol = 0;
#if defined _WIN32 && !defined SDL2_BUILD
    Window = 0;
    Async = 0;
#endif
    Host.Addr = -1;
    Host.DotAddr[0] = 0;
    Host.Name[0] = 0;
    Host.Port = 0;
    SendEntry = 0;
    SendLen = 0;
    ReceiveBuf = 0;
    ReceiveLen = 0;
    IsConnected = false;
    MaxPacketSize = maxpacketsize;
    ReceiveBuf = new char[maxpacketsize];
#if defined _WIN32 && !defined SDL2_BUILD
    Create_Window();
#endif
    Register(protocol);
}

ReliableCommClass::~ReliableCommClass()
{
    if (IsConnected) {
        Disconnect();
    }
#if defined _WIN32 && !defined SDL2_BUILD
    Destroy_Window();
#endif
    delete[] ReceiveBuf;
}

void ReliableCommClass::Register(ProtocolClass* protocol)
{
    Protocol = protocol;
}

bool ReliableCommClass::Connect(void* addr, int len, int type)
{
    DestAddress* _addr = (DestAddress*)addr;

    if (Protocol != nullptr) {
        return false;
    }

    if (len != sizeof(DestAddress)) {
        return false;
    }

    if (IsConnected) {
        Disconnect();
    }

    if (!Open_Socket()) {
        return false;
    }

#if defined _WIN32 && !defined SDL2_BUILD
    // Handle most network events.
    if (WSAAsyncSelect(Socket, Window, SSWM_RELIABLE_COMM_MSG, FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE)
        == SOCKET_ERROR) {
        Close_Socket();
        return false;
    }
#else
    FD_SET(Socket, &MonitoringSet);
    HighSocket = MAX(HighSocket, Socket + 1);
#endif

    if (type == 0) // Sole 1.0 has this as UnreliableCommClass::Resolve_Address
    {
        Host.Port = _addr->Port;
        Host.Addr = inet_addr(_addr->Host);
        if (Host.Addr != -1) {
            strcpy(Host.DotAddr, _addr->Host);
            Host.Name[0] = 0;
#if defined _WIN32 && !defined SDL2_BUILD
            Async = WSAAsyncGetHostByAddr(
                Window, SSWM_HOSTBYADDR_MSG, (char*)&Host.Addr, sizeof(Host.Addr), PF_INET, Hbuf, sizeof(Hbuf));
            if (!Async) {
                WSAAsyncSelect(Socket, Window, SSWM_RELIABLE_COMM_MSG, 0);
                Close_Socket();
                return false;
            }
#endif
            sockaddr_in name;
            name.sin_family = 2;
            name.sin_port = hton16(Host.Port);
            name.sin_addr.s_addr = Host.Addr;
            if (connect(Socket, (sockaddr*)&name, sizeof(name)) == SOCKET_ERROR
                && (LastSocketError != WSAEWOULDBLOCK && LastSocketError != WSAEINPROGRESS)) {
#if defined _WIN32 && !defined SDL2_BUILD
                WSACancelAsyncRequest(Async);
                Async = 0;
                WSAAsyncSelect(Socket, Window, SSWM_RELIABLE_COMM_MSG, 0);
#else
                FD_CLR(Socket, &MonitoringSet);
#endif
                Close_Socket();
                return false;
            }
        } else {
            strcpy(Host.Name, _addr->Host);
#if defined _WIN32 && !defined SDL2_BUILD
            Async = WSAAsyncGetHostByName(Window, SSWM_HOSTBYNAME_MSG, Host.Name, Hbuf, sizeof(Hbuf));

            if (!Async) {
                WSAAsyncSelect(Socket, Window, SSWM_RELIABLE_COMM_MSG, 0);
                Close_Socket();
                return false;
            }
#endif
        }

        IsConnected = true;
        return true;
    }

#if defined _WIN32 && !defined SDL2_BUILD
    WSAAsyncSelect(Socket, Window, SSWM_RELIABLE_COMM_MSG, 0);
#else
    FD_CLR(Socket, &MonitoringSet);
#endif
    Close_Socket();
    return false;
}

int ReliableCommClass::Connect(ListenerClass* listener)
{
    if (Protocol == nullptr) {
        return false;
    }

    if (IsConnected) {
        Disconnect();
    }

    socklen_t addrlen = 16;
    sockaddr_in addr;
    Socket = accept(listener->Socket, (sockaddr*)&addr, &addrlen);

    if (Socket == INVALID_SOCKET) {
        return false;
    }

    // Ensure it is none blocking.
    ioctlopt_t opt = 1;
    ioctlsocket(Socket, FIONBIO, &opt);

#if defined _WIN32 && !defined SDL2_BUILD
    if (WSAAsyncSelect(Socket, Window, SSWM_RELIABLE_COMM_MSG, FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE)
        == SOCKET_ERROR) {
        Close_Socket();
        return false;
    }
#else
    FD_SET(Socket, &MonitoringSet);
    HighSocket = MAX(HighSocket, Socket + 1);
#endif
    Host.Addr = addr.sin_addr.s_addr;
    strcpy(Host.DotAddr, inet_ntoa(addr.sin_addr));
    Host.Port = ntoh16(addr.sin_port);
    Host.Name[0] = 0;

#if defined _WIN32 && !defined SDL2_BUILD
    Async =
        WSAAsyncGetHostByAddr(Window, SSWM_HOSTBYADDR_MSG, (char*)&Host.Addr, sizeof(Host.Addr), PF_INET, Hbuf, 1024);

    if (!Async) {
        WSAAsyncSelect(Socket, Window, SSWM_RELIABLE_COMM_MSG, 0);
        Close_Socket();
        return false;
    }

    IsConnected = true;
#endif
    return true;
}

void ReliableCommClass::Disconnect()
{

    if (IsConnected) {
#if defined _WIN32 && !defined SDL2_BUILD
        if (Async) {
            WSACancelAsyncRequest(Async);
            Async = 0;
        }

        WSAAsyncSelect(Socket, Window, SSWM_RELIABLE_COMM_MSG, 0);
#else
        FD_CLR(Socket, &MonitoringSet);
#endif
        Close_Socket();
        Host.Name[0] = 0;
        Host.DotAddr[0] = 0;
        Host.Addr = -1;
        Host.Port = 0;
        SendEntry = 0;
        SendLen = 0;
        IsConnected = false;
    }
}

int ReliableCommClass::Send()
{
    uint16_t* buff;

    if (Protocol == nullptr || !IsConnected) {
        return false;
    }

    if (SendEntry != nullptr) {
        return true;
    }

    SendEntry = Protocol->Queue->Get_Send(0);

    if (SendEntry) {
        SendLen = SendEntry->BufLen;
        buff = (uint16_t*)SendEntry->Buffer;
        *buff = hton16(SendEntry->BufLen);
#if defined _WIN32 && !defined SDL2_BUILD
        PostMessageA(Window, SSWM_RELIABLE_COMM_MSG, 0, 2);
#endif
        return true;
    }

    return false;
}

unsigned short ReliableCommClass::Network_Short(unsigned short local_val)
{
    return hton16(local_val);
}

unsigned short ReliableCommClass::Local_Short(unsigned short net_val)
{
    return ntoh16(net_val);
}

unsigned int ReliableCommClass::Network_Long(unsigned int local_val)
{
    return hton32(local_val);
}

unsigned int ReliableCommClass::Local_Long(unsigned int net_val)
{
    return ntoh32(net_val);
}

#if defined _WIN32 && !defined SDL2_BUILD
void ReliableCommClass::Create_Window()
{
    WNDCLASSA WndClass;

    WndClass.style = 3;
    WndClass.lpfnWndProc = Message_Handler;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hWSockInstance;
    WndClass.hIcon = LoadIconA(NULL, (LPCSTR)0x7F00);
    WndClass.hCursor = LoadCursorA(NULL, (LPCSTR)0x7F00);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(0);
    WndClass.lpszMenuName = 0;
    WndClass.lpszClassName = "RCommWin";
    RegisterClassA(&WndClass);

    Window = CreateWindowExA(0,
                             "RCommWin",
                             "Data Transmission",
                             0x80000,
                             0x80000000,
                             0x80000000,
                             0x80000000,
                             0x80000000,
                             NULL,
                             NULL,
                             hWSockInstance,
                             NULL);

    ShowWindow(Window, 0);
    UpdateWindow(Window);
    SetWindowLongPtrA(Window, GWLP_USERDATA, (LONG_PTR)this);
}

void ReliableCommClass::Destroy_Window()
{
    if (Window) {
        DestroyWindow(Window);
        Window = 0;
    }
}
#endif

int ReliableCommClass::Open_Socket()
{
    Socket = socket(AF_INET, SOCK_STREAM, 0);

    if (Socket == INVALID_SOCKET) {
        return false;
    }

    // Ensure it is none blocking.
    ioctlopt_t opt = 1;
    ioctlsocket(Socket, FIONBIO, &opt);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = hton32(0);

    if (bind(Socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        Close_Socket();
        return false;
    }

    return true;
}

void ReliableCommClass::Close_Socket()
{
    struct linger optval;
    if (Socket != INVALID_SOCKET) {
        optval.l_linger = 0;
        optval.l_onoff = 0;
        setsockopt(Socket, SOL_SOCKET, SO_LINGER, (const char*)&optval, sizeof(optval));
        closesocket(Socket);
        Socket = -1;
    }
}

#if defined _WIN32 && !defined SDL2_BUILD
LRESULT __stdcall ReliableCommClass::Message_Handler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    static const char _http_response[] = "HTTP/1.0 200 OK\nServer: NealScape/1.0\nContent-Type: "
                                         "text/html\n\n<title>Sole Survivor</title><h1>Sole Survivor "
                                         "is "
                                         "alive.</h1>HTTP Responder.";

    ReliableCommClass* thisptr = reinterpret_cast<ReliableCommClass*>(GetWindowLongPtrA(hwnd, -21));

    switch (message) {
    // Fallthrough on all these events we are going to ignore.
    case WM_CREATE:
    case WM_PAINT:
    case WM_COMMAND:
        return 0;
    case WM_DESTROY:
        if (thisptr->IsConnected) {
            thisptr->Disconnect();
        }
        return 0;
    case SSWM_HOSTBYADDR_MSG:
        DBG_INFO("ReliableComms resolved a host by address.");
        if (WSAGETSELECTERROR(lparam) != 0) {
            thisptr->Host.Name[0] = '\0';
        } else {
            strcpy(thisptr->Host.Name, thisptr->Hbuf);

            if (thisptr->Protocol != nullptr) {
                thisptr->Protocol->Name_Resolved();
            }
        }

        return 0;
    case SSWM_HOSTBYNAME_MSG:
        DBG_INFO("ReliableComms resolved a host by name.");
        if (WSAGETSELECTERROR(lparam) != 0) {
            if (thisptr->Protocol != nullptr) {
                thisptr->Protocol->Connected_To_Server(0);
            }

            thisptr->Disconnect();
        } else {
            struct hostent* host = reinterpret_cast<struct hostent*>(thisptr->Hbuf);
            // Copy first address out of the list.
            memcpy(&thisptr->Host.Addr, *host->h_addr_list, sizeof(thisptr->Host.Addr));
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = hton16(thisptr->Host.Port);
            addr.sin_addr.s_addr = thisptr->Host.Addr;
            strcpy(thisptr->Host.DotAddr, inet_ntoa(addr.sin_addr));

            if (connect(thisptr->Socket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR
                && (LastSocketError != WSAEWOULDBLOCK)) {
                if (thisptr->Protocol != nullptr) {
                    thisptr->Protocol->Connected_To_Server(0);
                }

                thisptr->Disconnect();
            } else {
                thisptr->Async = 0;
            }
        }

        return 0;

    case SSWM_RELIABLE_COMM_MSG:
        DBG_INFO("ReliableComms handling a packet.");
        switch (WSAGETSELECTEVENT(lparam)) {
        case FD_READ:
            if (WSAGETSELECTERROR(lparam) != 0 && WSAGETSELECTERROR(lparam) != WSAEWOULDBLOCK) {
                if (thisptr->Protocol != nullptr) {
                    thisptr->Protocol->Closed();
                }

                thisptr->Disconnect();

                return 0;
            }

            while (true) {
                bool get_request = false;
                int received = 0;
                unsigned short packet_size = 0;

                // If we are yet to receive at least 2 bytes, then we need to try and get the first 2 bytes.
                if (thisptr->ReceiveLen < 2) {
                    received =
                        recv(thisptr->Socket, &thisptr->ReceiveBuf[thisptr->ReceiveLen], 2 - thisptr->ReceiveLen, 0);

                    if (received == SOCKET_ERROR) {
                        if (LastSocketError != WSAEWOULDBLOCK) {
                            if (thisptr->Protocol != nullptr) {
                                thisptr->Protocol->Connected_To_Server(0);
                            }

                            thisptr->Disconnect();
                            break;
                        }

                        received = 0;
                    }

                    thisptr->ReceiveLen += received;

                    // If we still haven't received at least 2 bytes just return as we are done for now.
                    if (thisptr->ReceiveLen < 2) {
                        break;
                    }

                    // If we get GE in the buffer assume this is a get http request.
                    if (strncmp(thisptr->ReceiveBuf, "GE", sizeof(unsigned short)) == 0) {
                        get_request = true;
                    } else {
                        packet_size = ntohs(*((unsigned short*)thisptr->ReceiveBuf));

                        if (packet_size > thisptr->MaxPacketSize) {
                            packet_size = thisptr->MaxPacketSize;
                        }

                        *((unsigned short*)thisptr->ReceiveBuf) = min((int)packet_size, thisptr->MaxPacketSize);
                        get_request = false;
                    }
                }

                if (thisptr->ReceiveLen >= 2) {
                    // Already converted from network byte order in last section.
                    packet_size = *((unsigned short*)thisptr->ReceiveBuf);

                    // Check if we have all the packet yet.
                    if (packet_size > thisptr->ReceiveLen) {
                        // If its a get request, make a response. Otherwise handle as game packet.
                        if (get_request) {
                            // Discard the rest of the packet.
                            while (recv(thisptr->Socket, thisptr->ReceiveBuf, 10, 0) > 0) {
                            }

                            send(thisptr->Socket, _http_response, strlen(_http_response) + 1, 0);
                            thisptr->Disconnect();
                            break;
                        } else {
                            received = recv(thisptr->Socket,
                                            &thisptr->ReceiveBuf[thisptr->ReceiveLen],
                                            packet_size - thisptr->ReceiveLen,
                                            0);

                            if (received == SOCKET_ERROR) {
                                if (LastSocketError != WSAEWOULDBLOCK) {
                                    if (thisptr->Protocol != nullptr) {
                                        thisptr->Protocol->Connected_To_Server(0);
                                    }

                                    thisptr->Disconnect();
                                    break;
                                }

                                received = 0;
                            }
                        }

                        thisptr->ReceiveLen += received;
                    }
                }

                // Have we got all of the packet yet? If not return 0.
                if (packet_size != thisptr->ReceiveLen) {
                    break;
                }

                if (thisptr->Protocol != nullptr) {
                    if (thisptr->Protocol->Queue->Num_Receive() == thisptr->Protocol->Queue->Max_Receive()) {
                        thisptr->Protocol->Queue->Grow_Receive(50);
                    }

                    received =
                        thisptr->Protocol->Queue->Queue_Receive(thisptr->ReceiveBuf, thisptr->ReceiveLen, nullptr, 0);

                    if (received != 0) {
                        thisptr->Protocol->Data_Received();
                    }
                }

                thisptr->ReceiveLen = 0;

                if (thisptr->Protocol == nullptr) {
                    break;
                }
            }

            return 0;
        case FD_WRITE: {
            // If we don't have a send entry, then assume nothing to send?
            if (thisptr->SendEntry == nullptr) {
                return 0;
            }

            if (WSAGETSELECTERROR(lparam) != 0) {
                if (thisptr->Protocol != nullptr) {
                    thisptr->Protocol->Closed();
                }

                thisptr->Disconnect();

                return 0;
            }

            unsigned short packet_size = ntoh16(*((unsigned short*)thisptr->SendEntry->Buffer));

            while (thisptr->SendLen > 0) {
                int sent = send(thisptr->Socket,
                                (const char*)&thisptr->SendEntry->Buffer[packet_size - thisptr->SendLen],
                                thisptr->SendLen,
                                0);

                if (sent == SOCKET_ERROR) {
                    if (LastSocketError != WSAEWOULDBLOCK) {
                        if (thisptr->Protocol != nullptr) {
                            thisptr->Protocol->Closed();
                        }

                        thisptr->Disconnect();
                        return 0;
                    }

                    break;
                }

                thisptr->SendLen -= sent;
            }

            if (thisptr->SendLen == 0) {
                if (thisptr->Protocol != nullptr) {
                    thisptr->Protocol->Queue->UnQueue_Send(nullptr, nullptr, 0, nullptr, nullptr);
                }

                thisptr->SendEntry = nullptr;
            }
        }

            return 0;
        case FD_CONNECT:
            if (WSAGETSELECTERROR(lparam) != 0) {
                if (thisptr->Protocol != nullptr) {
                    thisptr->Protocol->Connected_To_Server(0);
                }

                thisptr->Disconnect();
            } else {
                if (thisptr->Protocol != nullptr) {
                    thisptr->Protocol->Connected_To_Server(1);

                    if (strlen(thisptr->Host.Name) != 0) {
                        thisptr->Protocol->Name_Resolved();
                    }
                }
            }

            return 0;
        case FD_CLOSE:
            if (thisptr->Protocol != nullptr) {
                thisptr->Protocol->Closed();
            }

            thisptr->Disconnect();
            return 0;

        default:
            break;
        }

        return 0;
    default:
        break;
    }

    // All other events get passed to the next handler.
    return DefWindowProcA(hwnd, message, wparam, lparam);
}
#else
int ReliableCommClass::Message_Handler()
{
    static const char _http_response[] = "HTTP/1.0 200 OK\nServer: NealScape/1.0\nContent-Type: "
                                         "text/html\n\n<title>Sole Survivor</title><h1>Sole Survivor "
                                         "is "
                                         "alive.</h1>HTTP Responder.";

    if (!IsConnected) {
        return 0;
    }

    // This function assumes that the caller has called select on the read and write sets copied from the monitor set.
    if (FD_ISSET(Socket, &WriteSet)) {
        // Check if connect has succeeded.
        if (Protocol != nullptr && !Protocol->Is_Connected()) {
            struct sockaddr_in tmp;
            socklen_t tmp_size = sizeof(tmp);
            memset(&tmp, 0, sizeof(tmp));
            int retval = getpeername(Socket, (struct sockaddr*)&tmp, &tmp_size);

            if (retval == 0) {
                Protocol->Connected_To_Server(1);
            } else {
                int optval;
                socklen_t optval_size = sizeof(optval);
                retval = getsockopt(Socket, SOL_SOCKET, SO_ERROR, (char*)&optval, &optval_size);

                // Assume connection failed for some reason and bail.
                if (retval != 0 || optval != 0) {
                    if (Protocol != nullptr) {
                        Protocol->Connected_To_Server(0);
                    }

                    Disconnect();
                }
            }
        }

        // If we don't have a send entry, then assume nothing to send?
        if (SendEntry != nullptr) {
            unsigned short packet_size = ntoh16(*((unsigned short*)SendEntry->Buffer));

            while (SendLen > 0) {
                int sent = send(Socket, (const char*)&SendEntry->Buffer[packet_size - SendLen], SendLen, 0);

                if (sent == SOCKET_ERROR) {
                    if (LastSocketError != WSAEWOULDBLOCK && LastSocketError != WSAEAGAIN) {
                        if (Protocol != nullptr) {
                            Protocol->Closed();
                        }

                        Disconnect();
                        return 0;
                    }

                    break;
                }

                SendLen -= sent;
            }

            if (SendLen == 0) {
                if (Protocol != nullptr) {
                    Protocol->Queue->UnQueue_Send(nullptr, nullptr, 0, nullptr, nullptr);
                }

                SendEntry = nullptr;
            }
        }
    }

    if (FD_ISSET(Socket, &ReadSet)) {
        while (true) {
            bool get_request = false;
            int received = 0;
            unsigned short packet_size = 0;

            // If we are yet to receive at least 2 bytes, then we need to try and get the first 2 bytes.
            if (ReceiveLen < 2) {
                received = recv(Socket, &ReceiveBuf[ReceiveLen], 2 - ReceiveLen, 0);

                // 0 data means disconnect for TCP/IP
                if (received == 0) {
                    if (Protocol != nullptr) {
                        Protocol->Closed();
                    }

                    Disconnect();
                }

                if (received == SOCKET_ERROR) {
                    if (LastSocketError != WSAEWOULDBLOCK && LastSocketError != WSAEAGAIN) {
                        if (Protocol != nullptr) {
                            Protocol->Connected_To_Server(0);
                        }

                        Disconnect();
                        break;
                    }

                    received = 0;
                }

                ReceiveLen += received;

                // If we still haven't received at least 2 bytes just return as we are done for now.
                if (ReceiveLen < 2) {
                    break;
                }

                // If we get GE in the buffer assume this is a get http request.
                if (strncmp(ReceiveBuf, "GE", sizeof(unsigned short)) == 0) {
                    get_request = true;
                } else {
                    packet_size = ntoh16(*((unsigned short*)ReceiveBuf));

                    if (packet_size > MaxPacketSize) {
                        packet_size = MaxPacketSize;
                    }

                    *((unsigned short*)ReceiveBuf) = min((int)packet_size, MaxPacketSize);
                    get_request = false;
                }
            }

            if (ReceiveLen >= 2) {
                // Already converted from network byte order in last section.
                packet_size = *((unsigned short*)ReceiveBuf);

                // Check if we have all the packet yet.
                if (packet_size > ReceiveLen) {
                    // If its a get request, make a response. Otherwise handle as game packet.
                    if (get_request) {
                        // Discard the rest of the packet.
                        while (recv(Socket, ReceiveBuf, 10, 0) > 0) {
                        }

                        send(Socket, _http_response, strlen(_http_response) + 1, 0);
                        Disconnect();
                        break;
                    } else {
                        received = recv(Socket, &ReceiveBuf[ReceiveLen], packet_size - ReceiveLen, 0);

                        if (received == SOCKET_ERROR) {
                            if (LastSocketError != WSAEWOULDBLOCK && LastSocketError != WSAEAGAIN) {
                                if (Protocol != nullptr) {
                                    Protocol->Connected_To_Server(0);
                                }

                                Disconnect();
                                break;
                            }

                            received = 0;
                        }
                    }

                    ReceiveLen += received;
                }
            }

            // Have we got all of the packet yet? If not return 0.
            if (packet_size != ReceiveLen) {
                break;
            }

            if (Protocol != nullptr) {
                if (Protocol->Queue->Num_Receive() == Protocol->Queue->Max_Receive()) {
                    Protocol->Queue->Grow_Receive(50);
                }

                received = Protocol->Queue->Queue_Receive(ReceiveBuf, ReceiveLen, nullptr, 0);

                if (received != 0) {
                    Protocol->Data_Received();
                }
            }

            ReceiveLen = 0;

            if (Protocol == nullptr) {
                break;
            }
        }
    }

    return 0;
}
#endif
