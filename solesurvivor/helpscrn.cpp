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
#include "helpscrn.h"
#include "conquer.h"
#include "function.h"
#include "common/ini.h"
#include "common/ccfile.h"
#include <assert.h>
#include <string.h>

bool HelpScreenClass::Load()
{
    const char* section = "SSHELP";
    char buf[256];
    CCFileClass file(HelpFile);

    if (!file.Is_Available()) {
        WWMessageBox().Process("File error! (SSHELP)", TXT_OK);
        return false;
    }

    INIClass ini;
    ini.Load(file);

    Reset();
    EntryCount = ini.Entry_Count(section);

    if (EntryCount == 0) {
        return false;
    }

    Entries = new SoleHelpBufferStruct[EntryCount];

    for (unsigned index = 0; index < EntryCount; index++) {
        char const* entry = ini.Get_Entry(section, index);
        ini.Get_String(section, ini.Get_Entry(section, index), "x", buf, sizeof(buf));

        if (strcmp(buf, "x") == 0) {
            WWMessageBox().Process("Decode error! (SSHELP)", TXT_OK);
            Reset();
            return false;
        }

        Read_Entry(buf, index);
    }

    return true;
}

bool HelpScreenClass::Get_Entry(int xpos, int ypos, const char*& tooltip, const char*& description)
{
    if (Entries == nullptr) {
        return false;
    }

    for (unsigned i = 0; i < EntryCount; ++i) {
        if (ypos >= Entries[i].ClipRect.Y && Entries[i].ClipRect.Height + Entries[i].ClipRect.Y >= ypos
            && xpos >= Entries[i].ClipRect.X && Entries[i].ClipRect.Width + Entries[i].ClipRect.X >= xpos) {
            tooltip = Entries[i].Tooltip;
            description = Entries[i].Description;
            return true;
        }
    }

    return false;
}

void HelpScreenClass::Read_Entry(char* entry, int index)
{
    assert((unsigned)index < EntryCount);
    char* token = strtok(entry, ",");
    if (token) {
        Entries[index].ClipRect.X = atoi(token);
    }
    token = strtok(0, ",");
    if (token) {
        Entries[index].ClipRect.Y = atoi(token);
    }
    token = strtok(0, ",");
    if (token) {
        Entries[index].ClipRect.Width = atoi(token);
    }
    token = strtok(0, ",");
    if (token) {
        Entries[index].ClipRect.Height = atoi(token);
    }
    Entries[index].Tooltip[0] = 0;
    token = strtok(0, "\"");
    if (token) {
        strncpy(Entries[index].Tooltip, token, sizeof(Entries[index].Tooltip));
    }
    Entries[index].Description[0] = 0;
    if (strtok(0, "\"")) {
        token = strtok(0, "\"");
        if (token) {
            strncpy(Entries[index].Description, token, sizeof(Entries[index].Description));
        }
    }
}
