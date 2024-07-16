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
#include <windows.h>
#define WM_ACCEPT (WM_USER + 101)
extern HANDLE AppInstance;
#endif

ListenerClass::ListenerClass(void)
{
    Socket = INVALID_SOCKET;
    Protocol = NULL;
    Window = 0;
    IsListening = 0;

    Create_Window();
}

ListenerClass::ListenerClass(ProtocolClass* protocol)
{
    Socket = INVALID_SOCKET;
    Protocol = NULL;
    Window = 0;
    IsListening = 0;

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

void ListenerClass::Register(ProtocolClass* protocol)
{
    Protocol = protocol;
}

int ListenerClass::Start_Listening(void* addr, int len, int type)
{
    struct DestAddress* dest;

    if (Protocol == NULL) {
        return (0);
    }

    if (len != sizeof(struct DestAddress)) {
        return (0);
    }

    if (IsListening) {
        return (0);
    }

    if (type != COMMS_TCPIP) {
        return (0);
    }

    dest = (struct DestAddress*)addr;

    if (!Open_Socket(dest->Address.TCPIP.Port)) {
        return (0);
    }

    if (listen(Socket, 3) == SOCKET_ERROR) {
        Close_Socket();
        return (0);
    }

#if 0
    if (WSAAsyncSelect(Socket, Window, WM_ACCEPT, FD_ACCEPT) == SOCKET_ERROR) {
        Close_Socket();
        return (0);
    }
#endif

    IsListening = 1;

    return (1);
}

void ListenerClass::Stop_Listening(void)
{
    if (IsListening == 0) {
        return;
    }

#if 0
    WSAAsyncSelect(Socket, Window, WM_ACCEPT, 0);
#endif

    Close_Socket();

    IsListening = 0;
}

// Creates an invisible window to process the winapi async select calls.
void ListenerClass::Create_Window(void)
{
#if 0
    WNDCLASS wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = Window_Proc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = AppInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "ListenWin";

    RegisterClass(&wndclass);

    Window = CreateWindow("ListenWin",
                          "Listening for Connections",
                          (WS_OVERLAPPED | WS_SYSMENU) & (~WS_VISIBLE),
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          NULL,
                          NULL,
                          AppInstance,
                          NULL);


    ShowWindow(Window, SW_HIDE);
    UpdateWindow(Window);
    SetWindowLong(Window, GWL_USERDATA, (LONG)this);
#endif
}

void ListenerClass::Destroy_Window(void)
{
#if 0
    if (Window) {
        DestroyWindow(Window);
        Window = 0;
    }
#endif
}

int ListenerClass::Open_Socket(unsigned short port)
{
    struct sockaddr_in addr;

    Socket = socket(AF_INET, SOCK_STREAM, 0);
    if (Socket == INVALID_SOCKET) {
        return (0);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(Socket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        Close_Socket();
        return (0);
    }

    return (1);
}

void ListenerClass::Close_Socket(void)
{
    LINGER ling;

    if (Socket == INVALID_SOCKET) {
        return;
    }

    ling.l_onoff = 0;
    ling.l_linger = 0;
    setsockopt(Socket, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));

    if (closesocket(Socket) == SOCKET_ERROR) {
    }

    Socket = INVALID_SOCKET;

}

// WIN32 event handler.
#ifdef WIN32
long __stdcall ListenerClass::Window_Proc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
    int rc;
    ListenerClass* obj;

    obj = (ListenerClass*)GetWindowLongA(hwnd, GWL_USERDATA);

    switch (message) {
    case WM_CREATE:
        return (0);

    case WM_PAINT:
        return (0);

    case WM_DESTROY:
        return (0);

    case WM_COMMAND:
        return (0);

    case WM_ACCEPT:
        rc = WSAGETSELECTERROR(lParam);
        if (rc == 0) {
            obj->Protocol->Connection_Requested();
        }
        return (0);

    default:
        return (DefWindowProcA(hwnd, message, wParam, lParam));
    }

}
#endif