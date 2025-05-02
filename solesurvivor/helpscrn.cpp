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
#include "ccfile.h"
#include <string.h>

HelpScreenClass::HelpScreenClass(void)
    : HelpFile("SSHELP.INI")
    , EntryCount(0)
    , Entries(NULL)
{
}

void HelpScreenClass::Set_File_Name(const char* filename)
{
    HelpFile = filename;
    EntryCount = 0;
    delete[] Entries;
    Entries = NULL;
}

HelpScreenClass::~HelpScreenClass()
{
    if (Entries != NULL) {
        delete[] Entries;
        Entries = NULL;
        EntryCount = 0;
    }
}

bool HelpScreenClass::Load()
{
    static const char section[] = "SSHELP";

    CCFileClass file(HelpFile);

    if (!file.Is_Available()) {
        WWMessageBox().Process("File error! (SSHELP)", TXT_OK);
        return false;
    }

    INIClass ini;
    ini.Load(file);

    EntryCount = 0;

    if (Entries != NULL) {
        delete[] Entries;
    }

    EntryCount = ini.Entry_Count(section);

    if (EntryCount > 0) {
        Entries = new SoleHelpBufferStruct[EntryCount];
        char buf[1024];

        if (Entries != NULL) {
            for (int i = 0; i < EntryCount; i++) {
                ini.Get_String(section, ini.Get_Entry(section, i), "x", buf, sizeof(buf));

                if (strcmp(buf, "x") == 0) {
                    WWMessageBox().Process("Decode error! (SSHELP)", TXT_OK);
                    return false;
                }

                Read_Entry(buf, i);
            }
            return true;
        } else {
            WWMessageBox().Process("Allocation Failed! (SSHELP)", TXT_OK);
        }
    }
    return false;
}

bool HelpScreenClass::Get_Entry(int xpos, int ypos, const char*& tooltip, const char*& description)
{
    if (Entries != NULL && EntryCount > 0) {

        for (int i = 0; i < EntryCount; ++i) {
            if (ypos < Entries[i].ClipY || Entries[i].ClipY + Entries[i].ClipHeight < ypos) {
                continue;
            }

            if (xpos < Entries[i].ClipX || Entries[i].ClipX + Entries[i].ClipWidth < xpos) {
                continue;
            }

            tooltip = Entries[i].Tooltip;
            description = Entries[i].Description;
            return true;
        }
    }
    return false;
}

void HelpScreenClass::Read_Entry(char* entry, int index)
{
    char* token;

    token = strtok(entry, ",");
    if (token) {
        Entries[index].ClipX = atoi(token);
    }

    token = strtok(0, ",");
    if (token) {
        Entries[index].ClipY = atoi(token);
    }

    token = strtok(0, ",");
    if (token) {
        Entries[index].ClipWidth = atoi(token);
    }

    token = strtok(0, ",");
    if (token) {
        Entries[index].ClipHeight = atoi(token);
    }

    Entries[index].Tooltip[0] = 0;

    token = strtok(0, "\"");
    if (token) {
        strncpy(Entries[index].Tooltip, token, sizeof(Entries[index].Tooltip));
    }

    Entries[index].Description[0] = 0;

    token = strtok(0, "\"");
    if (token) {
        token = strtok(0, "\"");

        if (token) {
            strncpy(Entries[index].Description, token, sizeof(Entries[index].Description));
        }
    }
}

const char* HelpScreenClass::Get_File_Name()
{
    return HelpFile;
}
