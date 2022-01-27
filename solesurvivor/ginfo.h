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
#ifndef GINFO_H
#define GINFO_H

#include <stdint.h>

class GameInfoClass
{
    enum
    {
        MAX_TEAMS = 4, /* This probably belongs somewhere more global, max SS teams. */
    };

public:
    GameInfoClass()
        : Show(false)
    {
    }

    bool Get_Show()
    {
        return Show;
    }

    void Set_Show(bool show)
    {
        Show = show;
    }

    void Render(int w, int h);

private:
    bool Show;
    static int16_t TeamCounters[MAX_TEAMS];
};

#endif /* GINFO_H */
