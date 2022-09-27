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
#include "listener.h"
#include "common/combuf.h"
#include "protocol.h"
#include "common/debugstring.h"
#include "common/internet.h"
#include "function.h"

#if defined _WIN32 && !defined SDL2_BUILD
#define SSWM_LISTENER_MSG (WM_USER + 0x65)
#endif

ListenerProtocolClass::ListenerProtocolClass() : ConnectionRequests(0)
{
    Queue = new CommBufferClass(100, 100, 422, 32);
}

ListenerProtocolClass::~ListenerProtocolClass()
{
    delete Queue;
}

void ListenerProtocolClass::Connection_Requested()
{
    // TODO This is called on FD_ACCEPT events, I believe it should also signal ReliableCommClass to accept the connection.
    ++ConnectionRequests;
}

ListenerClass::ListenerClass() :
    Socket(INVALID_SOCKET),
    Protocol(nullptr),
#if defined _WIN32 && !defined SDL2_BUILD
    Window(),
#endif
    IsListening(false)
{
    Create_Window();
}

ListenerClass::ListenerClass(ProtocolClass *protocol) :
    Socket(INVALID_SOCKET),
    Protocol(nullptr),
#if defined _WIN32 && !defined SDL2_BUILD
    Window(),
#endif
    IsListening(false)
{
    Create_Window();
    Register(protocol);
}

ListenerClass::~ListenerClass()
{
    if (IsListening) {
        Stop_Listening();
    }

    Destroy_Window();
}

void ListenerClass::Register(ProtocolClass *protocol)
{
    Protocol = protocol;
}

int ListenerClass::Start_Listening(void *address, int size, int unk)
{
    // Check we are in a state capable of listening and have what we need.
    if (Protocol == nullptr || size != sizeof(DestAddress) || IsListening || unk != 0) {
        return 0;
    }

    // Open our listening socket.
    if (!Open_Socket(static_cast<DestAddress *>(address)->Port)) {
        return 0;
    }

    if (listen(Socket, 3) == SOCKET_ERROR) {
        Close_Socket();
        return 0;
    }

#if defined _WIN32 && !defined SDL2_BUILD
    // Start listening for FD_ACCEPT events.
    if (WSAAsyncSelect(Socket, Window, SSWM_LISTENER_MSG, FD_ACCEPT) == SOCKET_ERROR) {
        Close_Socket();
        return 0;
    }
#endif
    IsListening = true;

    return 1;
}

void ListenerClass::Stop_Listening()
{
    // If we aren't listening, then there is nothing to do.
    if (IsListening) {
        // Set to not be notified of any socket events, then close socket.
#if defined _WIN32 && !defined SDL2_BUILD
        WSAAsyncSelect(Socket, Window, SSWM_LISTENER_MSG, 0);
#endif
        Close_Socket();
        IsListening = false;
    }
}

void ListenerClass::Create_Window()
{
#if defined _WIN32 && !defined SDL2_BUILD
    WNDCLASSA wndclass;

    wndclass.style = 3;
    wndclass.lpfnWndProc = Message_Handler;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = GetModuleHandleA(nullptr);
    wndclass.hIcon = LoadIconA(0, MAKEINTRESOURCEA(0x7F00));
    wndclass.hCursor = LoadCursorA(0, MAKEINTRESOURCEA(0x7F00));
    wndclass.hbrBackground = (HBRUSH)GetStockObject(0);
    wndclass.lpszMenuName = 0;
    wndclass.lpszClassName = "ListenWin";
    RegisterClassA(&wndclass);

    Window = CreateWindowExA(0,
        "ListenWin",
        "Listening for Connections",
        0x80000u,
        0x80000000,
        0x80000000,
        0x80000000,
        0x80000000,
        0,
        0,
        GetModuleHandleA(nullptr),
        0);

    ShowWindow(Window, 0);
    UpdateWindow(Window);
    SetWindowLongPtrA(Window, GWLP_USERDATA, (LONG_PTR)this);
#endif
}

void ListenerClass::Destroy_Window()
{
#if defined _WIN32 && !defined SDL2_BUILD
    if (Window != nullptr) {
        DestroyWindow(Window);
        Window = nullptr;
    }
#endif
}

int ListenerClass::Open_Socket(unsigned short port)
{
    // Create a TCP/IP socket.
    Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if (Socket == INVALID_SOCKET) {
        return 0;
    }

    // Ensure it is none blocking.
    ioctlopt_t opt = 1;
    ioctlsocket(Socket, FIONBIO, &opt);

    // Set port for this socket to listen to a bind it.
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = hton16(port);
    addr.sin_addr.s_addr = hton32(0);

    if (bind(Socket, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        Close_Socket();
        return 0;
    }

    return 1;
}

void ListenerClass::Close_Socket()
{
    if (Socket == INVALID_SOCKET) {
        struct linger optval;
        optval.l_onoff = 0;
        optval.l_linger = 0;
        setsockopt(Socket, SOL_SOCKET, SO_LINGER, (const char *)&optval, sizeof(optval));
        closesocket(Socket);
        Socket = INVALID_SOCKET;
    }
}

#if defined _WIN32 && !defined SDL2_BUILD
LRESULT __stdcall ListenerClass::Message_Handler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    ListenerClass *thisptr = reinterpret_cast<ListenerClass *>(GetWindowLongPtrA(hwnd, -21));

    switch (umsg) {
        // Fallthrough on all these events we are going to ignore.
        case WM_CREATE:
        case WM_DESTROY:
        case WM_PAINT:
        case WM_COMMAND:
            return 0;
        case SSWM_LISTENER_MSG:
            DBG_INFO("Listener accepted a connection.");
            if ((lparam >> 16) == 0) {
                thisptr->Protocol->Connection_Requested();
            }

            return 0;
        default:
            break;
    }

    // All other events get passed to the next handler.
    return DefWindowProcA(hwnd, umsg, wparam, lparam);
}
#else
int ListenerClass::Message_Handler()
{
    fd_set set;
    struct timeval tv;
    FD_ZERO(&set);
    FD_SET(Socket, &set);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    int retval = select(Socket + 1, &set, NULL, NULL, &tv);

    if (retval > 0) {
        Protocol->Connection_Requested();
    }

    return 0;
}
#endif
