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
#include <time.h>

int CommStatsClass::Static11;
int CommStatsClass::Static01;

int CommStatsClass::Static12;
int CommStatsClass::Static02;

int CommStatsClass::Static13;
int CommStatsClass::Static03;

unsigned int CommStatsClass::Static14 = 10;
unsigned int CommStatsClass::Static04 = 10;

#ifdef WIN32
WSADATA WinSockData;
HANDLE AppInstance = 0;
#endif

CommStatsClass::CommStatsClass(char c)
{
    Member1 = 0;
    Member2 = 0;
    Member3 = 0;
    Member4 = CommStatsClass::Static14;
    Char1 = c;
}

CommStatsClass::~CommStatsClass()
{
}

int CommStatsClass::Set1(int a1)
{
    int* a2;
    int* a3;
    int* a4;
    unsigned int* a5;
    int v8;

    CommStatsClass::Get2(&a2, &a3, &a4, &a5);
    v8 = time(0);
    if (*a2 > 0xFFFFFFF || v8 - *a4 >= *a5) {
        if (*a4) {
            *a3 = *a2 / (unsigned int)(v8 - *a4);
        } else {
            *a3 = 0;
        }
        *a2 = 0;
        *a4 = v8;
    }
    if (a1 < 0) {
        return 1;
    }
    *a2 += a1;
    return 0;
}

int CommStatsClass::Get1()
{
    int* a2;
    int* a3;
    int* a4;
    unsigned int* a5;
    int v6;

    CommStatsClass::Get2(&a2, &a3, &a4, &a5);
    v6 = time(0);
    if (*a2 > 0xFFFFFFF || v6 - *a4 >= *a5) {
        if (*a4) {
            *a3 = *a2 / (unsigned int)(v6 - *a4);
        } else {
            *a3 = 0;
        }
        *a2 = 0;
        *a4 = v6;
    }
    return *a3;
}

int CommStatsClass::Set2(int a1)
{
    int* a2;
    int* a3;
    int* a4;
    unsigned int* a5;

    Get2(&a2, &a3, &a4, &a5);
    if (a1 < 1) {
        return 1;
    }
    *a5 = a1;
    return 0;
}

void CommStatsClass::Get2(int** a2, int** a3, int** a4, unsigned int** a5)
{
    switch (Char1) {
    case 0:
        *a2 = &Static01;
        *a3 = &Static02;
        *a4 = &Static03;
        *a5 = &Static04;
        break;
    case 1:
        *a2 = &Static11;
        *a3 = &Static12;
        *a4 = &Static13;
        *a5 = &Static14;
        break;
    default:
        *a2 = &Member1;
        *a3 = &Member2;
        *a4 = &Member3;
        *a5 = &Member4;
        break;
    }
}

int Init_Comms_API(HINSTANCE instance)
{
#ifdef WIN32
    AppInstance = GetModuleHandleA(NULL);
#endif

    int rc = socket_startup();

    return rc;
}

void Shutdown_Comms_API(void)
{
    socket_cleanup();
}
