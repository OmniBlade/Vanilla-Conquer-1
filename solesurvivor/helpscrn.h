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
#ifndef HELPSCRN_H
#define HELPSCRN_H

#include <wwstd.h>

class HelpScreenClass
{
public:
    struct SoleHelpBufferStruct
    {
        int ClipX;
        int ClipY;
        int ClipWidth;
        int ClipHeight;
        char Tooltip[64];
        char Description[512];
    };

    HelpScreenClass(void);
    ~HelpScreenClass();

    void Set_File_Name(const char* filename);
    const char* Get_File_Name();

    bool Load();
    bool Get_Entry(int xpos, int ypos, const char*& tooltip, const char*& description);

private:
    void Read_Entry(char* entry, int index);

    const char* HelpFile;
    int EntryCount;
    SoleHelpBufferStruct* Entries;
};

#endif