//
// Copyright 2020 Electronic Arts Inc.
//
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

/* $Header:   F:\projects\c&c\vcs\code\sidebar.cpv   2.13   02 Aug 1995 17:03:22   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SIDEBAR.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : October 20, 1994                                             *
 *                                                                                             *
 *                  Last Update : January 25, 1996   []                    *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SidebarClass::AI -- Handles player clicking on sidebar area.                              *
 *   SidebarClass::Abandon_Production -- Stops production of the object specified.             *
 *   SidebarClass::Activate -- Controls the sidebar activation.                                *
 *   SidebarClass::Activate_Demolish -- Controls the demolish button on the sidebar.           *
 *   SidebarClass::Activate_Repair -- Controls the repair button on the sidebar.               *
 *   SidebarClass::Activate_Upgrade -- Controls the upgrade button on the sidebar.             *
 *   SidebarClass::Add -- Adds a game object to the sidebar list.                              *
 *   SidebarClass::Draw_It -- Renders the sidebar display.                                     *
 *   SidebarClass::Factory_Link -- Links a factory to a sidebar strip.                         *
 *   SidebarClass::Init_Clear -- Sets sidebar to a known (and deactivated) state               *
 *   SidebarClass::Init_IO -- Adds buttons to the button list                                  *
 *   SidebarClass::Init_Theater -- Performs theater-specific initialization                    *
 *   SidebarClass::One_Time -- Handles the one time game initializations.                      *
 *   SidebarClass::One_Time -- Handles the one time game initializations.                      *
 *   SidebarClass::Recalc -- Examines the sidebar data and updates it as necessary.            *
 *   SidebarClass::Refresh_Cells -- Intercepts the refresh, looking for sidebar controls.      *
 *   SidebarClass::SBGadgetClass::Action -- Special function that controls the mouse over the s*
 *   SidebarClass::Scroll -- Handles scrolling the sidebar object strip.                       *
 *   SidebarClass::Set_Current -- Sets a specified object that controls the sidebar display.   *
 *   SidebarClass::SidebarClass -- Default constructor for the sidebar.                        *
 *   SidebarClass::StripClass::AI -- Input and AI processing for the side strip.               *
 *   SidebarClass::StripClass::Abandon_Produ -- Abandons production associated with sidebar.   *
 *   SidebarClass::StripClass::Activate -- Adds the strip buttons to the input system.         *
 *   SidebarClass::StripClass::Add -- Add an object to the side strip.                         *
 *   SidebarClass::StripClass::Deactivate -- Removes the side strip buttons from the input syst*
 *   SidebarClass::StripClass::Draw_It -- Render the sidebar display.                          *
 *   SidebarClass::StripClass::Factory_Link -- Links a factory to a sidebar button.            *
 *   SidebarClass::StripClass::Flag_To_Redra -- Flags the sidebar strip to be redrawn.         *
 *   SidebarClass::StripClass::Get_Special_Cameo -- Fetches the special event cameo shape.     *
 *   SidebarClass::StripClass::Init_Clear -- Sets sidebar to a known (and deactivated) state   *
 *   SidebarClass::StripClass::Init_IO -- Adds buttons to the button list                      *
 *   SidebarClass::StripClass::Init_Theater -- Performs theater-specific initialization        *
 *   SidebarClass::StripClass::One_Time -- Performs one time actions necessary for the side str*
 *   SidebarClass::StripClass::Recalc -- Revalidates the current sidebar list of objects.      *
 *   SidebarClass::StripClass::Scroll -- Causes the side strip to scroll.                      *
 *   SidebarClass::StripClass::SelectClass:: -- Action function when buildable cameo is selecte*
 *   SidebarClass::StripClass::SelectClass:: -- Assigns special values to a buildable select bu*
 *   SidebarClass::StripClass::SelectClass::SelectClass -- Default constructor.                *
 *   SidebarClass::StripClass::StripClass -- Default constructor for the side strip class.     *
 *   SidebarClass::Which_Column -- Determines which column a given type should appear.         *
 *   SidebarClass::Zoom_Mode_Control -- Handles the zoom mode toggle operation.                *
 *   sortfunc -- Utility routine that handles 'qsort' the strip buttons.                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"
#include "settings.h"
#include "dict.h"

/*
**	Define "_RETRIEVE" if the palette morphing tables are part of the loaded data. If this
**	is undefined, then the files will be created.
*/
#define _RETRIEVE

/***************************************************************************
**	This holds the translucent table for use with the construction clock
**	animation.
*/
char SidebarClass::StripClass::ClockTranslucentTable[(1 + 1) * 256];

/***************************************************************************
**	This points to the main sidebar shapes. These include the upgrade and
**	repair buttons.
*/
TheaterType SidebarClass::StripClass::LastTheater = THEATER_NONE;

typedef enum ButtonNumberType
{
    BUTTON_RADAR = 100,
    BUTTON_REPAIR,
    BUTTON_DEMOLISH,
    BUTTON_UPGRADE,
    BUTTON_SELECT,
    BUTTON_ZOOM,
	BUTTON_COLORLIST,
} ButtonNumberType;

/*
** Sidebar buttons
*/

SidebarClass::SBGadgetClass SidebarClass::Background;
ToggleClass* SidebarClass::Repair = NULL;
ToggleClass* SidebarClass::Upgrade = NULL;
ToggleClass* SidebarClass::Zoom = NULL;
ShapeButtonClass SidebarClass::StripClass::UpButton[COLUMNS];
ShapeButtonClass SidebarClass::StripClass::DownButton[COLUMNS];
SidebarClass::StripClass::SelectClass SidebarClass::StripClass::SelectButton[COLUMNS][MAX_VISIBLE];

ColorListClass *SidebarClass::ColorListInstance1;
ColorListClass *SidebarClass::ColorListInstance2;

/*
** Shape data pointers
*/
void const* SidebarClass::StripClass::LogoShapes = NULL;
void const* SidebarClass::StripClass::ClockShapes;
void const* SidebarClass::StripClass::SpecialShapes[3];

void const* SidebarClass::SidebarShape1;
void const* SidebarClass::SidebarShape2;

/***********************************************************************************************
 * SidebarClass::SidebarClass -- Default constructor for the sidebar.                          *
 *                                                                                             *
 *    Constructor for the sidebar handler. It basically sets up the sidebar to the empty       *
 *    condition.                                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
SidebarClass::SidebarClass(void)
    : IsSidebarActive(false)
    , IsToRedraw(true)
    , IsRepairActive(false)
    , IsUpgradeActive(false)
    , IsDemolishActive(false)
{
    new (&Column[0]) StripClass(InitClass());
    new (&Column[1]) StripClass(InitClass());
}

/***********************************************************************************************
 * SidebarClass::One_Time -- Handles the one time game initializations.                        *
 *                                                                                             *
 *    This routine is used to load the graphic data that is needed by the sidebar display. It  *
 *    should only be called ONCE.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this routine once when the game first starts.                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/28/94   JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::One_Time(void)
{
    PowerClass::One_Time();
    /*
    ** Set up the pixel offsets and widths and heights used to render the
    ** sidebar.  They are now variables because we need to change them for
    ** variable resolutions.
    */
    int factor = (SeenBuff.Get_Width() == 320) ? 1 : 2;
    SideBarWidth = SIDEBARWIDTH * factor;
    SideX = SeenBuff.Get_Width() - SideBarWidth;
    SideY = Map.RadY + Map.RadHeight + 1;
    SideWidth = SeenBuff.Get_Width() - SideX;
    SideHeight = SeenBuff.Get_Height() - SideY;
    MaxVisible = 4;
    ButtonHeight = 9 * factor;
    TopHeight = ButtonHeight + (4 * factor);

    Background.X = SideX + 8 * factor;
    Background.Y = SideY, Background.Width = SideWidth - 1;
    Background.Height = SideHeight - 1;
    /*
    **	This sets up the clipping window. This window is used by the shape drawing
    **	code so that as the sidebar buildable buttons scroll, they get properly
    **	clipped at the top and bottom edges.
    */
    WindowList[WINDOW_SIDEBAR][WINDOWX] = SideX + PowWidth;
    WindowList[WINDOW_SIDEBAR][WINDOWY] = SideY + TopHeight;
    WindowList[WINDOW_SIDEBAR][WINDOWWIDTH] = SideWidth;
    WindowList[WINDOW_SIDEBAR][WINDOWHEIGHT] = (MaxVisible * (StripClass::OBJECT_HEIGHT * factor));

    if (factor == 1) {
        WindowList[WINDOW_SIDEBAR][WINDOWY] += 1;
        WindowList[WINDOW_SIDEBAR][WINDOWHEIGHT] -= 1;
    }

    /*
    **	Set up the coordinates for the sidebar strips. These coordinates are for
    **	the upper left corner.
    */
    int width = (SideWidth - PowWidth) - (((StripClass::STRIP_WIDTH)*factor) << 1);
    int spacing = width / 3;

    Column[0].X = SideX + PowWidth + spacing;
    Column[0].Y = SideY + TopHeight + 1;
    Column[1].X = Column[0].X + (StripClass::STRIP_WIDTH * factor) + spacing - 1;
    Column[1].Y = SideY + TopHeight + 1;

    Column[0].One_Time(0);
    Column[1].One_Time(1);

    /*
    **	Load the sidebar shape in at this time. (Hi-Res sidebar is theater dependant)
    */
    if (SidebarShape1 == NULL) {
        SidebarShape1 = Hires_Retrieve("SIDE1.SHP");
    }
    if (SidebarShape2 == NULL) {
        SidebarShape2 = Hires_Retrieve("SIDE2.SHP");
    }

    static int tabs[] = {100};

	ColorListInstance1 = new ColorListClass(
		BUTTON_COLORLIST,
		Background.X - 16,
		Background.Y + 2,
		Background.Width + 1,
		Background.Height - 1,
		TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
		Hires_Retrieve("BTN-UP.SHP"),
		Hires_Retrieve("BTN-DN.SHP")
	);

	ColorListInstance1->Set_Tabs(tabs);
	ColorListInstance1->Set_Selected_Style(ColorListClass::SELECT_NONE);

	ColorListInstance2 = new ColorListClass(
		BUTTON_COLORLIST,
		Background.X - 16,
		Background.Y + 2,
		Background.Width + 1,
		Background.Height - 1,
		TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR,
		Hires_Retrieve("BTN-UP.SHP"),
		Hires_Retrieve("BTN-DN.SHP")
	);

	ColorListInstance1->Set_Tabs(tabs);
	ColorListInstance1->Set_Selected_Style(ColorListClass::SELECT_NONE);

	Timer1.Set(1800, 1);
}

/***********************************************************************************************
 * SidebarClass::Init_Clear -- Sets sidebar to a known (and deactivated) state                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::Init_Clear(void)
{
    PowerClass::Init_Clear();

    IsToRedraw = true;
    IsRepairActive = false;
    IsUpgradeActive = false;
    IsDemolishActive = false;

    Column[0].Init_Clear();
    Column[1].Init_Clear();

    Activate(false);
}

/***********************************************************************************************
 * SidebarClass::Init_IO -- Adds buttons to the button list                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::Init_IO(void)
{
    PowerClass::Init_IO();
}

/***********************************************************************************************
 * SidebarClass::Init_Theater -- Performs theater-specific initialization                      *
 *                                                                                             *
 * INPUT:   theater  -- The theater that is being initialized. Sometimes this has an effect on *
 *                      the data that is loaded.                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::Init_Theater(TheaterType theater)
{
    PowerClass::Init_Theater(theater);

    Column[0].Init_Theater(theater);
    Column[1].Init_Theater(theater);
}

/***********************************************************************************************
 * SidebarClass::Which_Column -- Determines which column a given type should appear.           *
 *                                                                                             *
 *    Use this function to resolve what column the specified object type should be placed      *
 *    into.                                                                                    *
 *                                                                                             *
 * INPUT:   otype -- Pointer to the object type class of the object in question.               *
 *                                                                                             *
 * OUTPUT:  Returns with the column number that the object should be placed in.                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/01/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int SidebarClass::Which_Column(RTTIType type)
{
    if (type == RTTI_BUILDINGTYPE || type == RTTI_BUILDING) {
        return (0);
    }
    return (1);
}

/***********************************************************************************************
 * SidebarClass::Factory_Link -- Links a factory to a sidebar strip.                           *
 *                                                                                             *
 *    This routine will link the specified factory to the sidebar strip. A factory must be     *
 *    linked to the sidebar so that as the factory production progresses, the sidebar will     *
 *    show the production progress.                                                            *
 *                                                                                             *
 * INPUT:   factory  -- The factory number to attach.                                          *
 *                                                                                             *
 *          type     -- The object type number.                                                *
 *                                                                                             *
 *          id       -- The object sub-type number.                                            *
 *                                                                                             *
 * OUTPUT:  Was the factory successfully attached to the sidebar strip?                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Factory_Link(int factory, RTTIType type, int id)
{
    assert((unsigned)type < RTTI_COUNT);
    assert(id >= 0);

    return (Column[Which_Column(type)].Factory_Link(factory, type, id));
}

/***********************************************************************************************
 * SidebarClass::Refresh_Cells -- Intercepts the refresh, looking for sidebar controls.        *
 *                                                                                             *
 *    This routine intercepts the Refresh_Cells call in order to see if the sidebar needs      *
 *    to be refreshed as well. If the special code to refresh the sidebar was found, it        *
 *    flags the sidebar to be redrawn and then removes the code from the list.                 *
 *                                                                                             *
 * INPUT:   cell  -- The cell to base the refresh list on.                                     *
 *                                                                                             *
 *          list  -- Pointer to the cell offset list that elaborates all the cells that        *
 *                   need to be flagged for redraw.                                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::Refresh_Cells(CELL cell, short const* list)
{
    if (*list == REFRESH_SIDEBAR) {
        IsToRedraw = true;
        Column[0].IsToRedraw = true;
        Column[1].IsToRedraw = true;
        Flag_To_Redraw(false);
    }
    PowerClass::Refresh_Cells(cell, list);
}

/***********************************************************************************************
 * SidebarClass::Activate_Repair -- Controls the repair button on the sidebar.                 *
 *                                                                                             *
 *    Use this routine to turn the repair sidebar button on and off. Typically, the button     *
 *    is enabled when the currently selected structure is friendly and damaged.                *
 *                                                                                             *
 * INPUT:   control  -- The controls how the button is to be activated or deactivated;         *
 *                      0  -- Turn button off.                                                 *
 *                      1  -- Turn button on.                                                  *
 *                      -1 -- Toggle button state.                                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the button previously activated?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Activate_Repair(int control)
{
    bool old = IsRepairActive;

    if (control == -1) {
        control = IsRepairActive ? 0 : 1;
    }
    switch (control) {
    case 1:
        IsRepairActive = true;
        break;

    default:
    case 0:
        IsRepairActive = false;
        break;
    }
    if (old != IsRepairActive) {
        Flag_To_Redraw(false);
        IsToRedraw = true;

        if (!IsRepairActive) {
            Help_Text(TXT_NONE);
            Set_Default_Mouse(MOUSE_NORMAL, false);
        }
    }
    return (old);
}

/***********************************************************************************************
 * SidebarClass::Activate_Upgrade -- Controls the upgrade button on the sidebar.               *
 *                                                                                             *
 *    Use this routine to turn the upgrade sidebar button on and off. Typically, the button    *
 *    is enabled when the currently selected structure can be upgraded and disabled otherwise. *
 *                                                                                             *
 * INPUT:   control  -- The controls how the button is to be activated or deactivated;         *
 *                      0  -- Turn button off.                                                 *
 *                      1  -- Turn button on.                                                  *
 *                      -1 -- Toggle button state.                                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the button previously activated?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Activate_Upgrade(int control)
{
    bool old = IsUpgradeActive;
    if (control == -1) {
        control = IsUpgradeActive ? 0 : 1;
    }
    switch (control) {
    case 1:
        IsUpgradeActive = true;
        break;

    default:
    case 0:
        IsUpgradeActive = false;
        break;
    }
    if (old != IsUpgradeActive) {
        Flag_To_Redraw(false);
        IsToRedraw = true;
        if (!IsUpgradeActive) {
            Set_Default_Mouse(MOUSE_NORMAL, false);
        }
    }
    return (old);
}

/***********************************************************************************************
 * SidebarClass::Activate_Demolish -- Controls the demolish button on the sidebar.             *
 *                                                                                             *
 *    Use this routine to turn the demolish/dismantle sidebar button on and off. Typically,    *
 *    the button is enabled when a friendly building is selected and disabled otherwise.       *
 *                                                                                             *
 * INPUT:   control  -- The controls how the button is to be activated or deactivated;         *
 *                      0  -- Turn button off.                                                 *
 *                      1  -- Turn button on.                                                  *
 *                      -1 -- Toggle button state.                                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the button previously activated?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Activate_Demolish(int control)
{
    bool old = IsDemolishActive;

    if (control == -1) {
        control = IsDemolishActive ? 0 : 1;
    }
    switch (control) {
    case 1:
        IsDemolishActive = true;
        break;

    default:
    case 0:
        IsDemolishActive = false;
        break;
    }
    if (old != IsDemolishActive) {
        Flag_To_Redraw(false);
        IsToRedraw = true;
        if (!IsDemolishActive) {
            Set_Default_Mouse(MOUSE_NORMAL, false);
        }
    }
    return (old);
}

/***********************************************************************************************
 * SidebarClass::Add -- Adds a game object to the sidebar list.                                *
 *                                                                                             *
 *    This routine is used to add a game object to the sidebar. Call this routine when a       *
 *    factory type building is created. It handles the case of adding an item that has already *
 *    been added -- it just ignores it.                                                        *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object that is being added.                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the object added to the sidebar?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/17/1994 JLB : Created.                                                                 *
 *   9/24/2019 3:17PM : Added via capture parameter for new sidebar functionality              *
 *=============================================================================================*/
bool SidebarClass::Add(RTTIType type, int id, bool via_capture)
{
    assert((unsigned)type < RTTI_COUNT);

    /*
    ** Add the sidebar only if we're not in editor mode.
    */
    if (!Debug_Map) {
        int column = Which_Column(type);

        if (Column[column].Add(type, id, via_capture)) {
            Activate(1);
            IsToRedraw = true;
            Flag_To_Redraw(false);
            return (true);
        }
        return (false);
    }

    return (false);
}

/***********************************************************************************************
 * SidebarClass::Scroll -- Handles scrolling the sidebar object strip.                         *
 *                                                                                             *
 *    This routine is used to scroll the sidebar strip of objects. The strip appears whenever  *
 *    a building is selected that can produce units. If the number of units to produce is      *
 *    greater than what the sidebar can hold, this routine is used to scroll the other object  *
 *    into view so they can be selected.                                                       *
 *                                                                                             *
 * INPUT:   up -- Should the scroll be upwards? Upward scrolling reveals object that are       *
 *                later in the list of objects.                                                *
 *                                                                                             *
 * OUTPUT:  bool; Did scrolling occur?                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/28/94   JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Scroll(bool up, int column)
{
    if (column == -1) {
        bool scr = false;
        scr |= Column[0].Scroll(up);
        scr |= Column[1].Scroll(up);
        if (!scr) {
            Sound_Effect(VOC_SCOLD);
        }
        if (scr) {
            IsToRedraw = true;
            Flag_To_Redraw(false);
            return (true);
        }
        return (false);
    }

    if (Column[column].Scroll(up)) {
        // No need to redraw the whole sidebar juts because we scrolled a strip is there? ST - 10/15/96 7:29PM
        // IsToRedraw = true;
        Flag_To_Redraw(false);
        return (true);
    }
    return (false);
}

/***********************************************************************************************
 * SidebarClass::Draw_It -- Renders the sidebar display.                                       *
 *                                                                                             *
 *    This routine performs the actual drawing of the sidebar display.                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the sidebar imagery changed at all?                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/28/94   JLB : Created.                                                                 *
 *   12/31/1994 JLB : Split rendering off into the sidebar strip class.                        *
 *=============================================================================================*/
void SidebarClass::Draw_It(bool complete)
{
    PowerClass::Draw_It(complete);

    if (IsSidebarActive) {
		if (SpecialDialog == SDLG_NONE) {
			ColorListInstance1->Draw_Me(true);
		}
	}

    IsToRedraw = false;
}

/***********************************************************************************************
 * SidebarClass::AI -- Handles player clicking on sidebar area.                                *
 *                                                                                             *
 *    This routine handles the processing necessary when the player clicks on the sidebar.     *
 *    Typically, this is selection of the item to build.                                       *
 *                                                                                             *
 * INPUT:   input -- Reference to the keyboard input value.                                    *
 *                                                                                             *
 *          x,y   -- Mouse coordinates at time of input.                                       *
 *                                                                                             *
 * OUTPUT:  bool; Was the click handled?                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/28/94   JLB : Created.                                                                 *
 *   11/11/1994 JLB : Processes input directly.                                                *
 *   12/26/1994 JLB : Uses factory manager class for construction handling.                    *
 *   12/31/1994 JLB : Simplified to use the sidebar strip class handlers.                      *
 *   12/31/1994 JLB : Uses mouse coordinate parameters.                                        *
 *   06/27/1995 JLB : <TAB> key toggles sidebar.                                               *
 *=============================================================================================*/
void SidebarClass::AI(KeyNumType& input, int x, int y)
{
    bool redraw = false;

    //
    // We need to process the sidebar differently in multiplayer. ST - 8/7/2019 10:48AM
    //
    if (GameToPlay == GAME_GLYPHX_MULTIPLAYER) {
        PowerClass::AI(input, x, y);
        return;
    }

    /*
    **	Toggle the sidebar in and out with the <TAB> key.
    */
    if (input == KN_TAB) {
        Activate(-1);
    }

    if (!Debug_Map) {
        Column[0].AI(input, x, y);
        Column[1].AI(input, x, y);
    }

    if (IsSidebarActive) {

        /*
        **	If there are any buildings in the payer's inventory, then allow the repair
        **	option.
        */
        if (PlayerPtr->BScan) {
            Activate_Repair(true);
        } else {
            Activate_Repair(false);
        }

        if (input == (BUTTON_REPAIR | KN_BUTTON)) {
            Repair_Mode_Control(-1);
        }

        if (input == (BUTTON_ZOOM | KN_BUTTON)) {
            Zoom_Mode_Control();
        }

        if (input == (BUTTON_UPGRADE | KN_BUTTON)) {
            Sell_Mode_Control(-1);
        }

#ifdef NEVER
        //		int index = -1;
        if (index != -1) {
            /*
            **	Display help text if the mouse is over a sidebar button.
            */
            switch (index) {
            default:
            case 2:
                Map.Help_Text(TXT_UPGRADE, -1, -1, PlayerPtr->Class->Color);
                break;

            case 1:
                Map.Help_Text(
                    PlayerPtr->Class->House == HOUSE_GOOD ? TXT_SELL : TXT_DEMOLISH, x, y, PlayerPtr->Class->Color);
                break;

            case 0:
                Map.Help_Text(TXT_REPAIR, x, y, PlayerPtr->Class->Color);
                break;
            }
        }
#endif

        if (redraw) {
            // IsToRedraw = true;
            Column[0].Flag_To_Redraw();
            Column[1].Flag_To_Redraw();

            Flag_To_Redraw(false);
        }

        if (input == (BUTTON_COLORLIST|KN_BUTTON)) {
			if (SpecialDialog == SDLG_NONE) {
				ColorListInstance1->Draw_Me(true);
			}
		}

		if(!Timer1.Time()) {
			Color_List_Reset();
			ColorListInstance1->Draw_Me(true);
			Timer1.Set(900, true);
		}
    }

    if ((!IsRepairMode) && Repair && Repair->IsOn) {
        Repair->Turn_Off();
    }

    if ((!IsSellMode) && Upgrade && Upgrade->IsOn) {
        Upgrade->Turn_Off();
    }

    PowerClass::AI(input, x, y);
}

/***********************************************************************************************
 * SidebarClass::Recalc -- Examines the sidebar data and updates it as necessary.              *
 *                                                                                             *
 *    Occasionally a factory gets destroyed. This routine must be called in such a case        *
 *    because it might be possible that sidebar object need to be removed. This routine will   *
 *    examine all existing objects in the sidebar class and if no possible factory can         *
 *    produce it, then it will be removed.                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine is exhaustive and thus time consuming. Only call it when really    *
 *             necessary. Such as when a factory is destroyed rather than when a non-factory   *
 *             is destroyed.                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/30/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::Recalc(void)
{
    bool redraw = false;

    // Done elsewhere for new multiplayer. ST - 8/7/2019 10:49AM
    if (GameToPlay == GAME_GLYPHX_MULTIPLAYER) {
        return;
    }

    redraw |= Column[0].Recalc();
    redraw |= Column[1].Recalc();

    if (redraw) {
        IsToRedraw = true;
        Flag_To_Redraw(false);
    }
}

/***********************************************************************************************
 * SidebarClass::Activate -- Controls the sidebar activation.                                  *
 *                                                                                             *
 *    Use this routine to turn the sidebar on or off. This routine handles updating the        *
 *    necessary flags.                                                                         *
 *                                                                                             *
 * INPUT:   control  -- Tells what to do with the sidebar according to the following:          *
 *                         0 = Turn sidebar off.                                               *
 *                         1 = Turn sidebar on.                                                *
 *                         -1= Toggle sidebar on or off.                                       *
 *                                                                                             *
 * OUTPUT:  bool; Was the sidebar already on?                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/09/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Activate(int control)
{
#ifdef REMASTER_BUILD
    //
    // We don't want the original sidebar to be visible. ST - 1/31/2019 11:28AM
    //
    if (control < 100) {
        return IsSidebarActive;
    }
#endif

    bool old = IsSidebarActive;

    int sidex = SeenBuff.Get_Width() - SideBarWidth;
    int sidey = Map.RadY + Map.RadHeight;
    int topheight = 13;
    int sidewidth = SeenBuff.Get_Width() - sidex;
    int sideheight = SeenBuff.Get_Height() - sidey;

    /*
    **	Determine the new state of the sidebar.
    */
    switch (control) {
    case -1:
        IsSidebarActive = IsSidebarActive == false;
        break;

    case 1:
        IsSidebarActive = true;
        break;

    default:
    case 0:
        IsSidebarActive = false;
        break;
    }

    /*
    **	Only if there is a change in the state of the sidebar will anything
    **	be done to change it.
    */
    if (IsSidebarActive != old) {

        /*
        **	If the sidebar is activated but was on the right side of the screen, then
        **	activate it on the left side of the screen.
        */
        if (IsSidebarActive /*&& X*/) {
            if (WDTRadarAdded) {
				RadarButton.Zap();
				Add_A_Button(RadarButton);
			}
			Help_Text(TXT_NONE);
			Set_View_Dimensions(0, Map.Get_Tab_Height(), SeenBuff.Get_Width() - sidewidth);
			ColorListInstance1->Zap();
			Add_A_Button(*ColorListInstance1);
		} else  {
			if (WDTRadarAdded) {
				Remove_A_Button(RadarButton);
			}			
			Help_Text(TXT_NONE);
			Set_View_Dimensions(0, Map.Get_Tab_Height());
			Remove_A_Button(*ColorListInstance1);
        }

        /*
        **	Since the sidebar status has changed, update the map so that the graphics
        **	will be rendered correctly.
        */
        Flag_To_Redraw(true);
        Map.IsToDrawUnknown = true;
    }

    return (old);
}

/***********************************************************************************************
 * SidebarClass::StripClass::StripClass -- Default constructor for the side strip class.       *
 *                                                                                             *
 *    This constructor is used to reset the side strip to default empty state.                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
SidebarClass::StripClass::StripClass(InitClass const&)
    : X(0)
    , Y(0)
    , ID(0)
    , IsToRedraw(true)
    , IsBuilding(false)
    , IsScrollingDown(false)
    , IsScrolling(false)
    , Flasher(-1)
    , TopIndex(0)
    , Scroller(0)
    , Slid(0)
    , BuildableCount(0)
{
    for (int index = 0; index < MAX_BUILDABLES; index++) {
        Buildables[index].BuildableID = 0;
        Buildables[index].BuildableType = RTTI_NONE;
        Buildables[index].Factory = -1;
        Buildables[index].BuildableViaCapture = false; // Added for new sidebar functionality. ST - 9/24/2019 3:10PM
    }
}

/***********************************************************************************************
 * SidebarClass::StripClass::One_Time -- Performs one time actions necessary for the side stri *
 *                                                                                             *
 *    Call this routine ONCE at the beginning of the game. It handles retrieving pointers to   *
 *    the shape files it needs for rendering.                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::One_Time(int)
{
    static const char* _file[3] = {"ION", "ATOM", "BOMB"};
    int factor = Get_Resolution_Factor();

    ObjectWidth = OBJECT_WIDTH << factor;
    ObjectHeight = OBJECT_HEIGHT << factor;
    StripWidth = STRIP_WIDTH << factor;
    LeftEdgeOffset = (StripWidth - ObjectWidth) >> 1;
    ButtonSpacingOffset = (StripWidth - ((BUTTON_WIDTH << factor) << 1)) / 3;

    /*
    ** Sidebar is player team specific in Hires
    */
    LogoShapes = Hires_Retrieve("STRIP.SHP");
    ClockShapes = Hires_Retrieve("CLOCK.SHP");

    char fullname[_MAX_FNAME + _MAX_EXT];
    char buffer[_MAX_FNAME];

    for (int lp = 0; lp < 3; lp++) {
        if (Get_Resolution_Factor()) {
            sprintf(buffer, "%sICNH", _file[lp]);
        } else {
            sprintf(buffer, "%sICON", _file[lp]);
        }
        _makepath(fullname, NULL, NULL, buffer, ".SHP");
        SpecialShapes[lp] = MFCD::Retrieve(fullname);
    }
}

/***********************************************************************************************
 * SidebarClass::StripClass::Get_Special_Cameo -- Fetches the special event cameo shape.       *
 *                                                                                             *
 *    This routine will return with a pointer to the cameo data for the special objects that   *
 *    can appear on the sidebar (e.g., nuclear bomb).                                          *
 *                                                                                             *
 * INPUT:   type  -- The special type to fetch the cameo imagery for.                          *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the cameo imagery for the specified special object.      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/1995 JLB : commented                                                                *
 *=============================================================================================*/
void const* SidebarClass::StripClass::Get_Special_Cameo(int type)
{
    return (SpecialShapes[type]);
}

/***********************************************************************************************
 * SidebarClass::StripClass::Init_Clear -- Sets sidebar to a known (and deactivated) state     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::Init_Clear(void)
{
    IsScrollingDown = false;
    IsScrolling = false;
    IsBuilding = false;
    Flasher = -1;
    TopIndex = 0;
    Slid = 0;
    BuildableCount = 0;

    /*
    ** Since we're resetting the strips, clear out all the buildables & factory pointers.
    */
    for (int index = 0; index < MAX_BUILDABLES; index++) {
        Buildables[index].BuildableID = 0;
        Buildables[index].BuildableType = RTTI_NONE;
        Buildables[index].Factory = -1;
        Buildables[index].BuildableViaCapture = false; // Added for new sidebar functionality. ST - 9/24/2019 3:10PM
    }
}

/***********************************************************************************************
 * SidebarClass::StripClass::Init_IO -- Initializes the strip's buttons                        *
 *                                                                                             *
 * This routine doesn't actually add any buttons to the list.                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::Init_IO(int id)
{
    ID = id;

    UpButton[ID].IsSticky = true;
    UpButton[ID].ID = BUTTON_UP + id;
    UpButton[ID].X = X + ButtonSpacingOffset + 1;
    UpButton[ID].Y = Y + MAX_VISIBLE * ObjectHeight - 1;

    UpButton[ID].Set_Shape(Hires_Retrieve("STRIPUP.SHP"));

    DownButton[ID].IsSticky = true;
    DownButton[ID].ID = BUTTON_DOWN + id;
    DownButton[ID].X = UpButton[ID].X + UpButton[ID].Width + ButtonSpacingOffset - 2;
    DownButton[ID].Y = Y + MAX_VISIBLE * ObjectHeight - 1;

    DownButton[ID].Set_Shape(Hires_Retrieve("STRIPDN.SHP"));

    for (int index = 0; index < MAX_VISIBLE; index++) {
        SelectClass& g = SelectButton[ID][index];
        g.ID = BUTTON_SELECT;
        g.X = X;
        g.Y = Y + (ObjectHeight * index);
        g.Width = ObjectWidth;
        g.Height = ObjectHeight;
        g.Set_Owner(*this, index);
    }
}

/***********************************************************************************************
 * SidebarClass::StripClass::Init_Theater -- Performs theater-specific initialization          *
 *                                                                                             *
 * INPUT:   theater                                                                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::Init_Theater(TheaterType theater)
{
    // if (theater != LastTheater) {

    static const char* _file[3] = {"ION", "ATOM", "BOMB"};
    int factor = Get_Resolution_Factor();
    char fullname[_MAX_FNAME + _MAX_EXT];
    char buffer[_MAX_FNAME];
    void const* cameo_ptr;

    for (int lp = 0; lp < 3; lp++) {
        if (Get_Resolution_Factor()) {
            sprintf(buffer, "%sICNH", _file[lp]);
        } else {
            sprintf(buffer, "%sICON", _file[lp]);
        }
        _makepath(fullname, NULL, NULL, buffer, Theaters[theater].Suffix);
        cameo_ptr = MFCD::Retrieve(fullname);
        if (cameo_ptr) {
            SpecialShapes[lp] = cameo_ptr;
        }
    }

#ifndef _RETRIEVE
    static TLucentType const ClockCols[1] = {//			{LTGREEN, BLACK, 0, 0},
                                             {GREEN, LTGREY, 180, 0}};

    /*
    **	Make sure that remapping doesn't occur on the colors that cycle.
    */
    Mem_Copy(GamePalette, OriginalPalette, 768);
    memset(&GamePalette[CYCLE_COLOR_START * 3], 0x3f, CYCLE_COLOR_COUNT * 3);

    /*
    **	Create the translucent table used for the sidebar.
    */
    Build_Translucent_Table(GamePalette, &ClockCols[0], 1, (void*)ClockTranslucentTable);
    CCFileClass(Fading_Table_Name("CLOCK", theater)).Write(ClockTranslucentTable, sizeof(ClockTranslucentTable));
    Mem_Copy(OriginalPalette, GamePalette, 768);
#else
    CCFileClass(Fading_Table_Name("CLOCK", theater)).Read(ClockTranslucentTable, sizeof(ClockTranslucentTable));
#endif
    LastTheater = theater;
    //}
}

/***********************************************************************************************
 * SidebarClass::StripClass::Activate -- Adds the strip buttons to the input system.           *
 *                                                                                             *
 *    This routine will add the side strip buttons to the map's input system. This routine     *
 *    should be called once when the sidebar activates.                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Never call this routine a second time without first calling Deactivate().       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::Activate(void)
{
    UpButton[ID].Zap();
    Map.Add_A_Button(UpButton[ID]);

    DownButton[ID].Zap();
    Map.Add_A_Button(DownButton[ID]);

    for (int index = 0; index < MAX_VISIBLE; index++) {
        SelectButton[ID][index].Zap();
        Map.Add_A_Button(SelectButton[ID][index]);
    }
}

/***********************************************************************************************
 * SidebarClass::StripClass::Deactivate -- Removes the side strip buttons from the input syste *
 *                                                                                             *
 *    Call this routine to remove all the buttons on the side strip from the map's input       *
 *    system.                                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Never call this routine unless the Activate() function was previously called.   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::Deactivate(void)
{
    Map.Remove_A_Button(UpButton[ID]);
    Map.Remove_A_Button(DownButton[ID]);
    for (int index = 0; index < MAX_VISIBLE; index++) {
        Map.Remove_A_Button(SelectButton[ID][index]);
    }
}

#ifdef NEVER
/***********************************************************************************************
 * sortfunc -- Utility routine that handles 'qsort' the strip buttons.                         *
 *                                                                                             *
 *    This routine is called by qsort() in order to sort the sidebar buttons. This sorting     *
 *    forces the sidebar buttons to always occur in the order that they can be built in,       *
 *    rather than the order that they were added to the sidebar list.                          *
 *                                                                                             *
 * INPUT:   ptr1  -- Pointer to the first sidebar class object.                                *
 *                                                                                             *
 *          ptr2  -- Pointer to the second sidebar class object.                               *
 *                                                                                             *
 * OUTPUT:  Returns <0 if the first object can be produced before the second. It returns       *
 *          >0 if the reverse is true. It returns exactly 0 if the production scneario for     *
 *          both objects is the same.                                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
static int sortfunc(void const* ptr1, void const* ptr2)
{
    SidebarClass::StripClass::BuildType* b1 = (SidebarClass::StripClass::BuildType*)ptr1;
    SidebarClass::StripClass::BuildType* b2 = (SidebarClass::StripClass::BuildType*)ptr2;

    TechnoTypeClass const* p1 = Fetch_Techno_Type(b1->BuildableType, b1->BuildableID);
    TechnoTypeClass const* p2 = Fetch_Techno_Type(b2->BuildableType, b2->BuildableID);

    int i1 = 0;
    int i2 = 0;

    if (p1)
        i1 = p1->What_Am_I() * 2;
    if (p2)
        i2 = p2->What_Am_I() * 2;

    /*
    **	Walls should be sorted after the regular buildings.
    */
    if (p1 && p1->What_Am_I() == RTTI_BUILDINGTYPE && ((BuildingTypeClass* const)p1)->IsWall) {
        i1++;
    }
    if (p2 && p2->What_Am_I() == RTTI_BUILDINGTYPE && ((BuildingTypeClass* const)p2)->IsWall) {
        i2++;
    }

    /*
    **	If the object types are identical, then sort by scenario available.
    */
    if (i1 == i2) {

        /*
        **	In the case of walls (can tell if there is an odd value), then sort
        **	by cost.
        */
        if (i1 & 0x01) {
            i1 = p1->Cost;
            i2 = p2->Cost;
        } else {
            i1 = p1->Scenario;
            i2 = p2->Scenario;
        }
    }

    return (i1 - i2);
}
#endif

/***********************************************************************************************
 * SidebarClass::StripClass::Add -- Add an object to the side strip.                           *
 *                                                                                             *
 *    Use this routine to add a buildable object to the side strip.                            *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object type that can be built and is to be added to     *
 *                      the side strip.                                                        *
 *                                                                                             *
 * OUTPUT:  bool; Was the object successfully added to the side strip? Failure could be the    *
 *                result of running out of room in the side strip array or the object might    *
 *                already be in the list.                                                      *
 *                                                                                             *
 * WARNINGS:   none.                                                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *   9/24/2019 3:17PM : Added via capture parameter for new sidebar functionality              *
 *=============================================================================================*/
bool SidebarClass::StripClass::Add(RTTIType type, int id, bool via_capture)
{
    if (BuildableCount <= MAX_BUILDABLES) {
        for (int index = 0; index < BuildableCount; index++) {
            if (Buildables[index].BuildableType == type && Buildables[index].BuildableID == id) {
                return (false);
            }
        }
        if (!ScenarioInit && type != RTTI_SPECIAL) {
            Speak(VOX_NEW_CONSTRUCT);
        }
        Buildables[BuildableCount].BuildableType = type;
        Buildables[BuildableCount].BuildableID = id;
        Buildables[BuildableCount].BuildableViaCapture = via_capture;
        BuildableCount++;
        IsToRedraw = true;
#ifdef OBSOLETE
        if (GameToPlay == GAME_NORMAL) {
            qsort(&Buildables[0], BuildableCount, sizeof(Buildables[0]), sortfunc);
        }
#endif
        return (true);
    }
    return (false);
}

/***********************************************************************************************
 * SidebarClass::StripClass::Scroll -- Causes the side strip to scroll.                        *
 *                                                                                             *
 *    Use this routine to flag the side strip to scroll. The direction scrolled is controlled  *
 *    by the parameter. Scrolling is merely initiated by this routine. Subsequent calls to     *
 *    the AI function and the Draw_It function are required to properly give the appearance    *
 *    of scrolling.                                                                            *
 *                                                                                             *
 * INPUT:   bool; Should the side strip scroll UP? If it is to scroll down then pass false.    *
 *                                                                                             *
 * OUTPUT:  bool; Was the side strip started to scroll in the desired direction?               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *   07/29/1995 JLB : Simplified scrolling logic.                                              *
 *=============================================================================================*/
bool SidebarClass::StripClass::Scroll(bool up)
{
    if (up) {
        Scroller--;
    } else {
        Scroller++;
    }
#ifdef NEVER
    if (BuildableCount <= MAX_VISIBLE)
        return (false);

    /*
    **	Top of list is moving toward lower ordered entries in the object list. It looks like
    **	the "window" to the object list is moving up even though the actual object images are
    **	scrolling downward.
    */
    if (up) {
        if (!TopIndex)
            return (false);

        TopIndex--;
        Slid = 0;
    } else {
        if (TopIndex + MAX_VISIBLE >= BuildableCount)
            return (false);

        Slid = ObjectHeight;
    }
    IsScrollingDown = !up;
    IsScrolling = true;
#endif
    return (true);
}

/***********************************************************************************************
 * SidebarClass::StripClass::Flag_To_Redra -- Flags the sidebar strip to be redrawn.           *
 *                                                                                             *
 *    This utility routine is called when something changes on the sidebar and it must be      *
 *    reflected the next time drawing is performed.                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::Flag_To_Redraw(void)
{
    IsToRedraw = true;
    // Map.SidebarClass::IsToRedraw = true;
    Map.Flag_To_Redraw(false);
}

/***********************************************************************************************
 * SidebarClass::StripClass::AI -- Input and AI processing for the side strip.                 *
 *                                                                                             *
 *    The side strip AI processing is performed by this function. This function not only       *
 *    checks for player input, but also handles any graphic logic updating necessary as a      *
 *    result of flashing or construction animation.                                            *
 *                                                                                             *
 * INPUT:   input -- The player input code.                                                    *
 *                                                                                             *
 *          x,y   -- Mouse coordinate to use.                                                  *
 *                                                                                             *
 * OUTPUT:  bool; Did the AI detect that it will need a rendering change? If this routine      *
 *                returns true, then the Draw_It function should be called at the              *
 *                earliest opportunity.                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *   12/31/1994 JLB : Uses mouse coordinate parameters.                                        *
 *=============================================================================================*/
bool SidebarClass::StripClass::AI(KeyNumType& input, int, int)
{
    bool redraw = false;

    /*
    **	If this is scroll button for this side strip, then scroll the strip as
    **	indicated.
    */
    if (input == (UpButton[ID].ID | KN_BUTTON)) { // && !IsScrolling
        UpButton[ID].IsPressed = false;
        Scroll(true);
    }
    if (input == (DownButton[ID].ID | KN_BUTTON)) { // && !IsScrolling
        DownButton[ID].IsPressed = false;
        Scroll(false);
    }

    /*
    **	Reflect the scroll desired direction/value into the scroll
    **	logic handler. This might result in up or down scrolling.
    */
    if (!IsScrolling && Scroller) {
        if (BuildableCount <= MAX_VISIBLE) {
            Scroller = 0;
        } else {

            /*
            **	Top of list is moving toward lower ordered entries in the object list. It looks like
            **	the "window" to the object list is moving up even though the actual object images are
            **	scrolling downward.
            */
            if (Scroller < 0) {
                if (!TopIndex) {
                    Scroller = 0;
                } else {
                    Scroller++;
                    IsScrollingDown = false;
                    IsScrolling = true;
                    TopIndex--;
                    Slid = 0;
                }

            } else {
                if (TopIndex + MAX_VISIBLE >= BuildableCount) {
                    Scroller = 0;
                } else {
                    Scroller--;
                    Slid = ObjectHeight;
                    IsScrollingDown = true;
                    IsScrolling = true;
                }
            }
        }
    }

    /*
    **	Scroll logic is handled here.
    */
    if (IsScrolling) {
        if (IsScrollingDown) {
            Slid -= SCROLL_RATE;
            if (Slid <= 0) {
                IsScrolling = false;
                Slid = 0;
                TopIndex++;
            }
        } else {
            Slid += SCROLL_RATE;
            if (Slid >= ObjectHeight) {
                IsScrolling = false;
                Slid = 0;
            }
        }
        redraw = true;
    }

    /*
    **	Handle any flashing logic. Flashing occurs when the player selects an object
    **	and provides the visual feedback of a recognized and legal selection.
    */
    if (Flasher != -1) {
        if (Graphic_Logic()) {
            redraw = true;
            if (Fetch_Stage() >= 7) {
                Set_Rate(0);
                Set_Stage(0);
                Flasher = -1;
            }
        }
    }

    /*
    **	Handle any building clock animation logic.
    */
    if (IsBuilding) {
        for (int index = 0; index < BuildableCount; index++) {
            int factoryid = Buildables[index].Factory;

            if (factoryid != -1) {
                FactoryClass* factory = Factories.Raw_Ptr(factoryid);

                if (factory && (factory->Has_Changed() || factory->Is_Blocked())) {
                    redraw = true;
                    if (factory->Has_Completed()) {

                        /*
                        **	Construction has been completed. Announce this fact to the player and
                        **	try to get the object to automatically leave the factory. Buildings are
                        **	the main exception to the ability to leave the factory under their own
                        **	power.
                        */
                        TechnoClass* pending = factory->Get_Object();
                        if (pending != NULL) {
                            switch (pending->What_Am_I()) {
                            case RTTI_UNIT:
                            case RTTI_AIRCRAFT:
                                OutList.Add(EventClass(EventClass::PLACE, pending->What_Am_I(), -1));
                                // Fall into next case.

                            case RTTI_BUILDING:
                                if (!factory->Is_Blocked()) {
                                    Speak(VOX_CONSTRUCTION);
                                }
                                break;

                            case RTTI_INFANTRY:
                                OutList.Add(EventClass(EventClass::PLACE, pending->What_Am_I(), -1));
                                if (!factory->Is_Blocked()) {
                                    Speak(VOX_UNIT_READY);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    /*
    **	If any of the logic determined that this side strip needs to be redrawn, then
    **	set the redraw flag for this side strip.
    */
    if (redraw) {
        Flag_To_Redraw();
    }
    return (redraw);
}

/***********************************************************************************************
 * SidebarClass::StripClass::Draw_It -- Render the sidebar display.                            *
 *                                                                                             *
 *    Use this routine to render the sidebar display. It checks to see if it needs to be       *
 *    redrawn and only redraw if necessary. If the "complete" parameter is true, then it       *
 *    will force redraw the entire strip.                                                      *
 *                                                                                             *
 * INPUT:   complete -- Should the redraw be forced? A force redraw will ignore the redraw     *
 *                      flag.                                                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *   08/06/1995 JLB : Handles multi factory tracking in same strip.                            *
 *=============================================================================================*/
void SidebarClass::StripClass::Draw_It(bool complete)
{
    if (IsToRedraw || complete) {
        IsToRedraw = false;
        int factor = Get_Resolution_Factor();

        if (RunningAsDLL) {
            return;
        }
        /*
        **	Fills the background to the side strip. We shouldnt need to do this if the strip
        ** has a full complement of icons.
        */
        /*
        ** New sidebar needs to be drawn not filled
        */
        CC_Draw_Shape(LogoShapes, ID, X + 3, Y - 1, WINDOW_MAIN, SHAPE_WIN_REL | SHAPE_NORMAL, 0);

        /*
        **	Redraw the scroll buttons.
        */
        UpButton[ID].Draw_Me(true);
        DownButton[ID].Draw_Me(true);

        /*
        **	Loop through all the buildable objects that are visible in the strip and render
        **	them. Their Y offset may be adjusted if the strip is in the process of scrolling.
        */
        for (int i = 0; i < MAX_VISIBLE + (IsScrolling ? 1 : 0); i++) {
            bool production;
            bool completed;
            int stage;
            bool darken = false;
            void const* shapefile = 0;
            int shapenum = 0;
            void const* remapper = 0;
            FactoryClass* factory = 0;
            int index = i + TopIndex;
            int x = X;
            int y = Y + i * ObjectHeight;
            y--;

            /*
            **	If the strip is scrolling, then the offset is adjusted accordingly.
            */
            if (IsScrolling) {
                y -= ObjectHeight - Slid;
            }

            /*
            **	Fetch the shape number for the object type located at this current working
            **	slot. This shape pointer is used to draw the underlying graphic there.
            */
            if (index < BuildableCount) {
                ObjectTypeClass const* obj = NULL;
                int spc = 0;

                if (Buildables[index].BuildableType != RTTI_SPECIAL) {

                    obj = Fetch_Techno_Type(Buildables[index].BuildableType, Buildables[index].BuildableID);
                    if (obj != NULL) {
                        bool isbusy = false;
                        switch (Buildables[index].BuildableType) {
                        case RTTI_INFANTRYTYPE:
                            isbusy = (PlayerPtr->InfantryFactory != -1);
                            break;

                        case RTTI_BUILDINGTYPE:
                            isbusy = (PlayerPtr->BuildingFactory != -1);
                            if (!BuildingTypeClass::As_Reference((StructType)Buildables[index].BuildableID).IsWall) {
                                remapper = PlayerPtr->Remap_Table(false, false);
                            }
                            break;

                        case RTTI_UNITTYPE:
                            isbusy = (PlayerPtr->UnitFactory != -1);
                            switch (Buildables[index].BuildableID) {
                            case UNIT_MCV:
                            case UNIT_HARVESTER:
                                remapper = PlayerPtr->Remap_Table(false, false);
                                break;

                            default:
                                remapper = PlayerPtr->Remap_Table(false, true);
                                break;
                            }
                            break;

                        case RTTI_AIRCRAFTTYPE:
                            isbusy = (PlayerPtr->AircraftFactory != -1);
                            remapper = PlayerPtr->Remap_Table(false, true);
                            break;
                        }
                        shapefile = obj->Get_Cameo_Data();
                        shapenum = 0;
                        if (Buildables[index].Factory != -1) {
                            factory = Factories.Raw_Ptr(Buildables[index].Factory);
                            production = true;
                            completed = factory->Has_Completed();
                            stage = factory->Completion();
                            darken = false;
                        } else {
                            production = false;
                            //							darken      = IsBuilding;

                            /*
                            **	Darken the imagery if a factory of a matching type is
                            **	already busy.
                            */
                            darken = isbusy;
                        }
                    }

                } else {

                    spc = Buildables[index].BuildableID;
                    shapefile = Get_Special_Cameo(spc - 1);
                    shapenum = 0;

                    switch (spc) {
                    case SPC_ION_CANNON:
                        production = true;
                        completed = PlayerPtr->IonCannon.Is_Ready();
                        stage = PlayerPtr->IonCannon.Anim_Stage();
                        darken = false;
                        break;

                    case SPC_AIR_STRIKE:
                        production = true;
                        completed = PlayerPtr->AirStrike.Is_Ready();
                        stage = PlayerPtr->AirStrike.Anim_Stage();
                        darken = false;
                        break;

                    case SPC_NUCLEAR_BOMB:
                        production = true;
                        completed = PlayerPtr->NukeStrike.Is_Ready();
                        stage = PlayerPtr->NukeStrike.Anim_Stage();
                        darken = false;
                        break;
                    }
                }

                if (obj != NULL || spc) {
                    /*
                    ** If this item is flashing then take care of it.
                    **
                    */
                    if (Flasher == index && (Fetch_Stage() & 0x01)) {
                        remapper = Map.FadingLight;
                    }

                } else {
                    shapefile = LogoShapes;
                    shapenum = SB_BLANK;
                }
            } else {
                shapefile = LogoShapes;
                shapenum = SB_BLANK;
                production = false;
            }

            if (Get_Resolution_Factor()) {
                remapper = 0;
            }
            /*
            **	Now that the shape of the object at the current working slot has been found,
            **	draw it and any graphic overlays as necessary.
            **
            ** Don't draw blank shapes over the new 640x400 sidebar art - ST 5/1/96 6:01PM
            */
            if (factor == 0 || shapenum != SB_BLANK || shapefile != LogoShapes) {
                IsTheaterShape = (bool)factor; // This shape is theater specific
                CC_Draw_Shape(shapefile,
                              shapenum,
                              x - WindowList[WINDOW_SIDEBAR][WINDOWX] + LeftEdgeOffset,
                              y - WindowList[WINDOW_SIDEBAR][WINDOWY],
                              WINDOW_SIDEBAR,
                              SHAPE_NORMAL | SHAPE_WIN_REL | (remapper ? SHAPE_FADING : SHAPE_NORMAL),
                              remapper);
                IsTheaterShape = false;

                /*
                **	Darken this object because it cannot be produced or is otherwise
                **	unavailable.
                */
                if (darken) {
                    CC_Draw_Shape(ClockShapes,
                                  0,
                                  x - WindowList[WINDOW_SIDEBAR][WINDOWX] + LeftEdgeOffset,
                                  y - WindowList[WINDOW_SIDEBAR][WINDOWY],
                                  WINDOW_SIDEBAR,
                                  SHAPE_NORMAL | SHAPE_WIN_REL | SHAPE_GHOST,
                                  NULL,
                                  ClockTranslucentTable);
                }
            }

            /*
            **	Draw the overlapping clock shape if this is object is being constructed.
            **	If the object is completed, then display "Ready" with no clock shape.
            */
            if (production) {
                if (completed) {

                    /*
                    **	Display text showing that the object is ready to place.
                    */
                    CC_Draw_Shape(ObjectTypeClass::PipShapes,
                                  PIP_READY,
                                  (x - WindowList[WINDOW_SIDEBAR][WINDOWX]) + LeftEdgeOffset + (ObjectWidth >> 1),
                                  (y - WindowList[WINDOW_SIDEBAR][WINDOWY]) + ObjectHeight
                                      - Get_Build_Frame_Height(ObjectTypeClass::PipShapes) - 8,
                                  WINDOW_SIDEBAR,
                                  SHAPE_CENTER);
                    //					Fancy_Text_Print(TXT_READY, x+TEXT_X_OFFSET, y+TEXT_Y_OFFSET, TEXT_COLOR, TBLACK,
                    //TPF_6POINT|TPF_CENTER|TPF_NOSHADOW);
                } else {
                    CC_Draw_Shape(ClockShapes,
                                  stage + 1,
                                  x - WindowList[WINDOW_SIDEBAR][WINDOWX] + LeftEdgeOffset,
                                  y - WindowList[WINDOW_SIDEBAR][WINDOWY],
                                  WINDOW_SIDEBAR,
                                  SHAPE_NORMAL | SHAPE_WIN_REL | SHAPE_GHOST,
                                  NULL,
                                  ClockTranslucentTable);

                    /*
                    **	Display text showing that the construction is temporarily on hold.
                    */
                    if (factory && !factory->Is_Building()) {
                        CC_Draw_Shape(ObjectTypeClass::PipShapes,
                                      PIP_HOLDING,
                                      (x - WindowList[WINDOW_SIDEBAR][WINDOWX]) + LeftEdgeOffset + (ObjectWidth >> 1),
                                      (y - WindowList[WINDOW_SIDEBAR][WINDOWY]) + ObjectHeight
                                          - Get_Build_Frame_Height(ObjectTypeClass::PipShapes)
                                          - 8, // Moved up now that icons have names on them
                                      WINDOW_SIDEBAR,
                                      SHAPE_CENTER);
                        //						Fancy_Text_Print(TXT_HOLDING, x+TEXT_X_OFFSET, y+TEXT_Y_OFFSET, TEXT_COLOR,
                        //TBLACK, TPF_6POINT|TPF_CENTER|TPF_NOSHADOW);
                    }
                }
            }
        }
    }
}

/***********************************************************************************************
 * SidebarClass::StripClass::Recalc -- Revalidates the current sidebar list of objects.        *
 *                                                                                             *
 *    This routine will revalidate all the buildable objects in the sidebar. This routine      *
 *    comes in handy when a factory has been destroyed, and the sidebar needs to reflect any   *
 *    change that this requires. It checks every object to see if there is a factory available *
 *    that could produce it. If none can be found, then the object is removed from the         *
 *    sidebar.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; The sidebar has changed as a result of this call?                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *   06/26/1995 JLB : Doesn't collapse sidebar when buildables removed.                        *
 *=============================================================================================*/
bool SidebarClass::StripClass::Recalc(void)
{
    int ok;

    if (Debug_Map || !BuildableCount) {
        return (false);
    }

    /*
    **	Sweep through all objects listed in the sidebar. If any of those object can
    **	not be created -- even in theory -- then they must be removed form the sidebar and
    **	any current production must be abandoned.
    */
    bool redraw = false;
    for (int index = 0; index < BuildableCount; index++) {
        TechnoTypeClass const* tech = Fetch_Techno_Type(Buildables[index].BuildableType, Buildables[index].BuildableID);
        if (tech != NULL) {
            ok = tech->Who_Can_Build_Me(true, false, PlayerPtr->Class->House) != NULL;
        } else {
            switch (Buildables[index].BuildableID) {
            case SPC_ION_CANNON:
                ok = PlayerPtr->IonCannon.Is_Present();
                break;

            case SPC_NUCLEAR_BOMB:
                ok = PlayerPtr->NukeStrike.Is_Present();
                break;

            case SPC_AIR_STRIKE:
                ok = PlayerPtr->AirStrike.Is_Present();
                break;

            default:
                ok = false;
                break;
            }

#ifdef OBSOLETE
        }
        else
        {
            switch (Buildables[index].BuildableID) {
            case SPC_ION_CANNON:
                ok = (PlayerPtr->BScan & STRUCTF_EYE) != 0 || PlayerPtr->IonOneTimeFlag;
                if (!ok) {
                    PlayerPtr->Remove_Ion_Cannon();
                }
                break;

            case SPC_NUCLEAR_BOMB:
                ok = (PlayerPtr->BScan & STRUCTF_TEMPLE) != 0 && PlayerPtr->Has_Nuke_Device();
                ok = ok || PlayerPtr->NukeOneTimeFlag;
                if (!ok) {
                    PlayerPtr->Remove_Nuke_Bomb();
                }
                break;

            case SPC_AIR_STRIKE:
                //					ok = (PlayerPtr->BScan & STRUCTF_SAM) == 0;
                //					ok = !PlayerPtr->Does_Enemy_Building_Exist(STRUCT_SAM);
                ok = (PlayerPtr->AirPresent /*&& !PlayerPtr->Does_Enemy_Building_Exist(STRUCT_SAM)*/)
                     || PlayerPtr->AirOneTimeFlag;
                if (!ok) {
                    PlayerPtr->Remove_Air_Strike();
                }
                break;

            default:
                ok = false;
                break;
            }
#endif
        }

        if (!ok) {

            /*
            **	If there was something in production, then abandon it before deleting the
            **	factory manager.
            */
            // if (Buildables[index].Factory != -1) {
            // FactoryClass * factory = Factories.Raw_Ptr(Buildables[index].Factory);
            // factory->Abandon();
            // delete factory;
            // Buildables[index].Factory = -1;
            //}
            //			Buildables[index].Factory = -1;

            /*
            **	Removes this entry from the list.
            */
            if (BuildableCount > 1 && index < BuildableCount - 1) {
                memmove(
                    &Buildables[index], &Buildables[index + 1], sizeof(Buildables[0]) * ((BuildableCount - index) - 1));
            }
            TopIndex = 0;
            IsToRedraw = true;
            redraw = true;
            BuildableCount--;
            index--;
        }
    }

#ifdef NEVER
    /*
    **	If there are no more buildable objects to display, make the sidebar go away.
    */
    if (!BuildableCount) {
        Map.SidebarClass::Activate(0);
    }
#endif
    return (redraw);
}

/***********************************************************************************************
 * SidebarClass::StripClass::SelectClass::SelectClass -- Default constructor.                  *
 *                                                                                             *
 *    This is the default constructor for the button that controls the buildable cameos on     *
 *    the sidebar strip.                                                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The coordinates are set to zero by this routine. They must be set to the        *
 *             correct values before this button will function.                                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
SidebarClass::StripClass::SelectClass::SelectClass(void)
    : ControlClass(0, 0, 0, 0, 0, LEFTPRESS | RIGHTPRESS | LEFTUP)
    , Strip(0)
    , Index(0)
{
    int factor = Get_Resolution_Factor();
    Width = StripClass::OBJECT_WIDTH << factor;
    Height = StripClass::OBJECT_HEIGHT << factor;
}

/***********************************************************************************************
 * SidebarClass::StripClass::SelectClass:: -- Assigns special values to a buildable select but *
 *                                                                                             *
 *    Use this routine to set custom buildable vars for this particular select button. It      *
 *    uses this information to properly know what buildable object to start or stop production *
 *    on.                                                                                      *
 *                                                                                             *
 * INPUT:   strip    -- Reference to the strip that owns this buildable button.                *
 *                                                                                             *
 *          index    -- The index (0 .. MAX_VISIBLE-1) of this button. This is used to let     *
 *                      the owning strip know what index this button refers to.                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::SelectClass::Set_Owner(StripClass& strip, int index)
{
    int factor = Get_Resolution_Factor();
    Strip = &strip;
    Index = index;
    X = strip.X;
    Y = strip.Y + (index * (OBJECT_HEIGHT << factor));
}

/***********************************************************************************************
 * SidebarClass::StripClass::SelectClass:: -- Action function when buildable cameo is selected *
 *                                                                                             *
 *    This function is called when the buildable icon (cameo) is clicked on. It handles        *
 *    starting and stopping production as indicated.                                           *
 *                                                                                             *
 * INPUT:   flags -- The input event that triggered the call.                                  *
 *                                                                                             *
 *          key   -- The keyboard value at the time of the input.                              *
 *                                                                                             *
 * OUTPUT:  Returns with whether the input list should be scanned further.                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int SidebarClass::StripClass::SelectClass::Action(unsigned flags, KeyNumType& key)
{
    int index = Strip->TopIndex + Index;
    RTTIType otype = Strip->Buildables[index].BuildableType;
    int oid = Strip->Buildables[index].BuildableID;
    int fnumber = Strip->Buildables[index].Factory;

    FactoryClass* factory = NULL;
    ObjectTypeClass const* choice = NULL;
    int spc = 0;

    /*
    **	Determine the factory number that would apply to objects of the type
    **	the mouse is currently addressing. This doesn't mean that the factory number
    **	fetched is actually producing the indicated object, merely that that particular
    **	kind of factory is specified by the "genfactory" value. This can be used to see
    **	if the factory type is currently busy or not.
    */
    int genfactory = -1;
    switch (otype) {
    case RTTI_INFANTRYTYPE:
        genfactory = PlayerPtr->InfantryFactory;
        break;

    case RTTI_UNITTYPE:
        genfactory = PlayerPtr->UnitFactory;
        break;

    case RTTI_AIRCRAFTTYPE:
        genfactory = PlayerPtr->AircraftFactory;
        break;

    case RTTI_BUILDINGTYPE:
        genfactory = PlayerPtr->BuildingFactory;
        break;

    default:
        genfactory = -1;
        break;
    }

    Map.Override_Mouse_Shape(MOUSE_NORMAL);

    if (index < Strip->BuildableCount) {
        if (otype != RTTI_SPECIAL) {
            choice = Fetch_Techno_Type(otype, oid);
        } else {
            spc = oid;
        }

        if (fnumber != -1) {
            factory = Factories.Raw_Ptr(fnumber);
        }

    } else {
        Map.Help_Text(TXT_NONE);
    }

    if (spc) {
        /*
        **	Display the help text if the mouse is over the button.
        */
        if (flags & LEFTUP) {
            switch (spc) {
            case SPC_ION_CANNON:
                Map.Help_Text(TXT_ION_CANNON, X, Y, CC_GREEN, true);
                break;

            case SPC_NUCLEAR_BOMB:
                Map.Help_Text(TXT_NUKE_STRIKE, X, Y, CC_GREEN, true);
                break;

            case SPC_AIR_STRIKE:
                Map.Help_Text(TXT_AIR_STRIKE, X, Y, CC_GREEN, true);
                break;
            }
            flags &= ~LEFTUP;
        }

        /*
        **	A right mouse button signals "cancel".  If we are in targeting
        ** mode then we don't want to be any more.
        */
        if (flags & RIGHTPRESS) {
            Map.IsTargettingMode = false;
        }
        /*
        **	A left mouse press signal "activate".  If our weapon type is
        ** available then we should activate it.
        */
        if (flags & LEFTPRESS) {
            switch (spc) {
            case SPC_ION_CANNON:
                if (PlayerPtr->IonCannon.Is_Ready()) {
                    Map.IsTargettingMode = spc;
                    Unselect_All();
                    Speak(VOX_SELECT_TARGET);
                } else {
                    PlayerPtr->IonCannon.Impatient_Click();
                }
                break;

            case SPC_AIR_STRIKE:
                if (PlayerPtr->AirStrike.Is_Ready()) {
                    Map.IsTargettingMode = spc;
                    Unselect_All();
                    Speak(VOX_SELECT_TARGET);
                } else {
                    PlayerPtr->AirStrike.Impatient_Click();
                }
                break;

            case SPC_NUCLEAR_BOMB:
                if (PlayerPtr->NukeStrike.Is_Ready()) {
                    Map.IsTargettingMode = spc;
                    Unselect_All();
                    Speak(VOX_SELECT_TARGET);
                } else {
                    PlayerPtr->NukeStrike.Impatient_Click();
                }
                break;
            }
        }

    } else {

        if (choice != NULL) {

            /*
            **	Display the help text if the mouse is over the button.
            */
            if (flags & LEFTUP) {
                Map.Help_Text(choice->Full_Name(), X, Y, CC_GREEN, true, choice->Cost_Of() * PlayerPtr->CostBias);
                flags &= ~LEFTUP;
            }

            /*
            **	A right mouse button signals "cancel".
            */
            if (flags & RIGHTPRESS) {

                /*
                **	If production is in progress, put it on hold. If production is already
                **	on hold, then abandon it. Money will be refunded, the factory
                **	manager deleted, and the object under construction is returned to
                **	the free pool.
                */
                if (factory != NULL) {

                    /*
                    **	Cancels placement mode if the sidebar factory is abandoned or
                    **	suspended.
                    */
                    if (Map.PendingObjectPtr && Map.PendingObjectPtr->Is_Techno()) {
                        Map.PendingObjectPtr = 0;
                        Map.PendingObject = 0;
                        Map.PendingHouse = HOUSE_NONE;
                        Map.Set_Cursor_Shape(0);
                    }

                    if (!factory->Is_Building()) {
                        Speak(VOX_CANCELED);
                        OutList.Add(EventClass(EventClass::ABANDON, otype, oid));
                    } else {
                        Speak(VOX_SUSPENDED);
                        OutList.Add(EventClass(EventClass::SUSPEND, otype, oid));
                    }
                }
            }

            if (flags & LEFTPRESS) {

                /*
                **	If there is already a factory attached to this strip but the player didn't click
                **	on the icon that has the attached factory, then say that the factory is busy and
                **	ignore the click.
                */
                if (fnumber == -1 && genfactory != -1) {
                    Speak(VOX_NO_FACTORY);
                    ControlClass::Action(flags, key);
                    return (true);
                }

                if (factory != NULL) {

                    /*
                    **	If this object is currently being built, then give a scold sound and text and then
                    **	bail.
                    */
                    if (factory->Is_Building()) {
                        Speak(VOX_NO_FACTORY);
                    } else {

                        /*
                        **	If production has completed, then attempt to have the object exit
                        **	the factory or go into placement mode.
                        */
                        if (factory->Has_Completed()) {

                            TechnoClass* pending = factory->Get_Object();
                            if (!pending && factory->Get_Special_Item()) {
                                Map.IsTargettingMode = true;
                            } else {
                                BuildingClass* builder = pending->Who_Can_Build_Me(false, false);
                                if (!builder) {
                                    OutList.Add(EventClass(EventClass::ABANDON, otype, oid));
                                    Speak(VOX_NO_FACTORY);
                                } else {

                                    /*
                                    **	If the completed object is a building, then change the
                                    **	game state into building placement mode. This fact is
                                    **	not transmitted to any linked computers until the moment
                                    **	the building is actually placed down.
                                    */
                                    if (pending->What_Am_I() == RTTI_BUILDING) {
                                        PlayerPtr->Manual_Place(builder, (BuildingClass*)pending);
                                    } else {

                                        /*
                                        **	For objects that can leave the factory under their own
                                        **	power, queue this event and process through normal house
                                        **	production channels.
                                        */
                                        OutList.Add(EventClass(EventClass::PLACE, otype, -1));
                                    }
                                }
                            }
                        } else {

                            /*
                            **	The factory must have been in a suspended state. Resume construction
                            **	normally.
                            */
                            Speak(VOX_BUILDING);
                            OutList.Add(EventClass(EventClass::PRODUCE,
                                                   Strip->Buildables[index].BuildableType,
                                                   Strip->Buildables[index].BuildableID));
                        }
                    }

                } else {

                    /*
                    **	If this side strip is already busy with production, then ignore the
                    **	input and announce this fact.
                    */
                    //					if (Strip->IsBuilding) {
                    //						Speak(VOX_NO_FACTORY);
                    //					} else {
                    Speak(VOX_BUILDING);
                    OutList.Add(EventClass(EventClass::PRODUCE,
                                           Strip->Buildables[index].BuildableType,
                                           Strip->Buildables[index].BuildableID));
                    //					}
                }
            }
        } else {
            flags = 0;
        }
    }

    ControlClass::Action(flags, key);
    return (true);
}

/***********************************************************************************************
 * SidebarClass::SBGadgetClass::Action -- Special function that controls the mouse over the si *
 *                                                                                             *
 *    This routine is called whenever the mouse is over the sidebar. It makes sure that the    *
 *    mouse is always the normal shape while over the sidebar.                                 *
 *                                                                                             *
 * INPUT:   flags -- The event flags that resulted in this routine being called.               *
 *                                                                                             *
 *          key   -- Reference the keyboard code that may be present.                          *
 *                                                                                             *
 * OUTPUT:  Returns that no further keyboard processing is necessary.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int SidebarClass::SBGadgetClass::Action(unsigned, KeyNumType&)
{
    Map.Help_Text(TXT_NONE);
    Map.Override_Mouse_Shape(MOUSE_NORMAL, false);
    return (true);
}

/***********************************************************************************************
 * SidebarClass::StripClass::Factory_Link -- Links a factory to a sidebar button.              *
 *                                                                                             *
 *    This routine will link the specified factory to this sidebar strip. The exact button to  *
 *    link to is determined from the object type and id specified. A linked button is one that *
 *    will show appropriate construction animation (clock shape) that matches the state of     *
 *    the factory.                                                                             *
 *                                                                                             *
 * INPUT:   factory  -- The factory number to link to the sidebar.                             *
 *                                                                                             *
 *          type     -- The object type that this factory refers to.                           *
 *                                                                                             *
 *          id       -- The object sub-type that this factory refers to.                       *
 *                                                                                             *
 * OUTPUT:  Was the factory successfully attached? Failure would indicate that there is no     *
 *          object of the specified type and sub-type in the sidebar list.                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::StripClass::Factory_Link(int factory, RTTIType type, int id)
{
    for (int index = 0; index < BuildableCount; index++) {
        if (Buildables[index].BuildableType == type && Buildables[index].BuildableID == id) {
            Buildables[index].Factory = factory;
            IsBuilding = true;
            /*
            ** Flag that all the icons on this strip need to be redrawn
            */
            Flag_To_Redraw();
            return (true);
        }
    }
    return (false);
}

/***********************************************************************************************
 * SidebarClass::Abandon_Production -- Stops production of the object specified.               *
 *                                                                                             *
 *    This routine is used to abandon production of the object specified. The factory will     *
 *    be completely disabled by this call.                                                     *
 *                                                                                             *
 * INPUT:   type     -- The object type that is to be abandoned. The sub-type is not needed    *
 *                      since it is presumed there can be only one type in production at any   *
 *                      one time.                                                              *
 *                                                                                             *
 *          factory  -- The factory number that is doing the production.                       *
 *                                                                                             *
 * OUTPUT:  Was the factory successfully abandoned?                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Abandon_Production(RTTIType type, int factory)
{
    return (Column[Which_Column(type)].Abandon_Production(factory));
}

/***********************************************************************************************
 * SidebarClass::StripClass::Abandon_Produ -- Abandons production associated with sidebar.     *
 *                                                                                             *
 *    Production of the object associated with this sidebar is abandoned when this routine is  *
 *    called.                                                                                  *
 *                                                                                             *
 * INPUT:   factory  -- The factory index that is to be suspended.                             *
 *                                                                                             *
 * OUTPUT:  Was the production abandonment successful?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *   08/06/1995 JLB : More intelligent abandon logic for multiple factories.                   *
 *=============================================================================================*/
bool SidebarClass::StripClass::Abandon_Production(int factory)
{
    bool noprod = true;
    bool abandon = false;
    for (int index = 0; index < BuildableCount; index++) {
        if (Buildables[index].Factory == factory) {
            Factories.Raw_Ptr(factory)->Abandon();
            Buildables[index].Factory = -1;
            abandon = true;
        } else {
            if (Buildables[index].Factory != -1) {
                noprod = false;
            }
        }
    }

    /*
    **	If there was a change to the strip, then flag the strip to be redrawn.
    */
    if (abandon) {
        Flag_To_Redraw();
    }

    /*
    **	If there is no production whatsoever on this strip, then flag it so.
    */
    if (noprod) {
        IsBuilding = false;
    }
    return (abandon);
}

// Matching
void SidebarClass::Color_List_Add_Player(const char *name, HousesType house)
{
	char str[80];
	HouseClass *hptr = HouseClass::As_Pointer(house);
	int index = Color_List_Find_Entry(name);

	if (index != -1) {
		ColorListInstance1->Colors[index] = MPlayerTColors[hptr->RemapColor];

		if (!OfflineMode && Options.IsVerbose && Frame > 60) {
			sprintf(str, Text_String(TXT_HAS_JOINED_GAME), name);
			Messages.Add_Message(str, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
			Flag_To_Redraw(false);
		}

		Color_List_Toggle_Spectator(name, false);
	} else {
		char *item;
		if (ColorListInstance1->Count() > 60) {
			for (index = 0; index < ColorListInstance1->Count(); index++) {
				if (ColorListInstance1->Colors[index] == 13) {
					item = (char *)ColorListInstance1->Get_Item(index);
					ColorListInstance1->Remove_Item(item);
					delete[] item;
					break;
				}
			}
		}

		item = new char[80];
		sprintf(item, " %s \t0", name);
		ColorListInstance1->Add_Item(item, MPlayerTColors[hptr->RemapColor]);

		if (IsSidebarActive) {
			if (SpecialDialog == SDLG_NONE) {
				ColorListInstance1->Draw_Me(true);
			}
		}

		if (!OfflineMode && Options.IsVerbose && Frame > 60) {
			sprintf(str, Text_String(TXT_HAS_JOINED_GAME), name);
			Messages.Add_Message(str, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
			Flag_To_Redraw(false);
		}
	}
}

// Matching
void SidebarClass::Color_List_Remove_Player(const char *name)
{
	char str[80];
	int index = Color_List_Find_Entry(name);

	if (index != -1) {
		ColorListInstance1->Colors[index] = 13;

		if (Options.IsVerbose && Frame > 60) {
			sprintf(str, Text_String(TXT_HAS_LEFT_GAME), name);
			Messages.Add_Message(str, 15, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 0);
			Flag_To_Redraw(false);
		}

		Color_List_Toggle_Spectator(name, false);

		if (IsSidebarActive) {
			if (SpecialDialog == SDLG_NONE) {
				ColorListInstance1->Draw_Me(true);
			}
		}
	}
}

// Matching
void SidebarClass::Color_List_Add_Teams(void)
{
	if (GameParams.NumTeams >= 2) {
		if (DebugLogTeams) {
			char msg[300];
			sprintf(msg, "*SidebarClass::Add_Teams\n");
			CCDebugString(msg);
		}

		char *team1 = new char[80];

		if (GameParams.NumTeams > 0) {
			sprintf(team1, " * %s \t%d", Text_String(TXT_BLUE), TeamScores[0]);
			ColorListInstance1->Add_Item(team1, MPlayerTColors[2]);
		}

		char *team2 = new char[80];

		if (GameParams.NumTeams > 1) {
			sprintf(team2, " * %s \t%d", Text_String(TXT_ORANGE), TeamScores[1]);
			ColorListInstance1->Add_Item(team2, MPlayerTColors[3]);
		}

		char *team3 = new char[80];

		if (GameParams.NumTeams > 2) {
			sprintf(team3, " * %s \t%d", Text_String(TXT_GREEN), TeamScores[2]);
			ColorListInstance1->Add_Item(team3, MPlayerTColors[4]);
		}

		char *team4 = new char[80];

		if (GameParams.NumTeams > 3) {
			sprintf(team4, " * %s \t%d", Text_String(TXT_GREY), TeamScores[3]);
			ColorListInstance1->Add_Item(team4, MPlayerTColors[5]);
		}

		if (IsSidebarActive) {
			if (SpecialDialog == SDLG_NONE) {
				ColorListInstance1->Draw_Me(true);
			}
		}
	}
}

// Matching
void SidebarClass::Color_List_Draw_Points(int index)
{
	char str[100];
	const char *item;
	char *score_str;
	int entry;
	int score;

	if (GameParams.NumTeams < 2) {
		return;
	}

	switch (index) {
		case 0:
			sprintf(str, "* %s", Text_String(TXT_BLUE));
			break;
		case 1:
			sprintf(str, "* %s", Text_String(TXT_ORANGE));
			break;
		case 2:
			sprintf(str, "* %s", Text_String(TXT_GREEN));
			break;
		case 3:
			sprintf(str, "* %s", Text_String(TXT_GREY));
			break;
	}

	score = TeamScores[index];
	entry = Color_List_Find_Entry(str);

	if (entry == -1) {
		return;
	}

	/*
	** Retrieve list string and update the score text.
	*/
	item = ColorListInstance1->Get_Item(entry);
	score_str = strchr((char *)item, '\t');
	score_str++;
	sprintf(score_str, "%d", score);

	if (OfflineMode) {
		sprintf(score_str, "%d", OfflinePoints);
	}

	if (IsSidebarActive) {
		if (SpecialDialog == SDLG_NONE) {
			ColorListInstance1->Draw_Me(true);
		}
	}
}

// Matching
void SidebarClass::Color_List_Update_Points(const char *name, int points)
{
	int entry = Color_List_Find_Entry(name);
	const char *item;
	char *score_str;

	if (entry == -1) {
		return;
	}

	item = ColorListInstance1->Get_Item(entry);
	score_str = strchr((char *)item, '\t');
	score_str++;
	sprintf(score_str, "%d", points);

	if (OfflineMode) {
		sprintf(score_str, "%d", OfflinePoints);
	}

	if (IsSidebarActive) {
		if (SpecialDialog == SDLG_NONE) {
			ColorListInstance1->Draw_Me(true);
		}
	}
}

// Matching
void SidebarClass::Color_List_Clear(void)
{
	const char *entry;

	while (ColorListInstance1->Count()) {
		entry = ColorListInstance1->Get_Item(0);
		ColorListInstance1->Remove_Item(entry);
		delete[] (char *)entry;
	}
}

// Matching
void SidebarClass::Color_List_Flag_To_Redraw(void)
{
	// TODO Rename as this looks like it clears redrawn rather than flags to redraw.
	const char *entry;
	int index;

	for (index = 0; index < ColorListInstance1->Count(); index++) {
		if (ColorListInstance1->Colors[index] == 13) {
			entry = ColorListInstance1->Get_Item(index);
			ColorListInstance1->Remove_Item(entry);
			delete[] (char *)entry;
			index--;
		}
	}
}

// Matching
void SidebarClass::Color_List_Reset(void)
{
	int max_score;
	int max_index;
	int index;
	char *entry;
	char *score_str;
	int score;
	int count;

	max_score = 0;

	for (index = 0; index < ColorListInstance2->Count(); index++) {
		entry = (char *)ColorListInstance1->Get_Item(0);
		ColorListInstance1->Remove_Item(entry);
		delete[] entry;
	}

	count = 0;

	for (index = 0; index < ColorListInstance1->Count(); index++) {
		entry = (char *)ColorListInstance1->Get_Item(index);
		if (*entry != '(' && ColorListInstance1->Colors[index] != 13) {
			count++;
		}
	}

	while (count > 0) {
		max_score = -1;
		max_index = 0;

		for (index = 0; index < ColorListInstance1->Count(); index++) {
			entry = (char *)ColorListInstance1->Get_Item(index);

			if (*entry != '(' && ColorListInstance1->Colors[index] != 13) {
				score_str = strchr(entry, '\t');
				score_str++;
				score = atoi(score_str);

				if (score > max_score) {
					max_score = score;
					max_index = index;
				}
			}
		}

		entry = (char *)ColorListInstance1->Get_Item(max_index);
		ColorListInstance2->Add_Item(entry, ColorListInstance1->Colors[max_index]);
		ColorListInstance1->Remove_Item(entry);
		count--;
	}

	count = 0;

	for (index = 0; index < ColorListInstance1->Count(); index++) {
		entry = (char *)ColorListInstance1->Get_Item(index); // TODO, copy/paste error?

		if (ColorListInstance1->Colors[index] != 13) {
			count++;
		}
	}

	while (count > 0) {
		max_score = -1;
		max_index = 0;

		for (index = 0; index < ColorListInstance1->Count(); index++) {
			entry = (char *)ColorListInstance1->Get_Item(index);

			if (ColorListInstance1->Colors[index] != 13) {
				score_str = strchr(entry, '\t');
				score_str++;
				score = atoi(score_str);

				if (score > max_score) {
					max_score = score;
					max_index = index;
				}
			}
		}

		entry = (char *)ColorListInstance1->Get_Item(max_index);
		ColorListInstance2->Add_Item(entry, ColorListInstance1->Colors[max_index]);
		ColorListInstance1->Remove_Item(entry);
		count--;
	}

	while (ColorListInstance1->Count() > 0) {
		max_score = -1;
		max_index = 0;

		for (index = 0; index < ColorListInstance1->Count(); index++) {
			entry = (char *)ColorListInstance1->Get_Item(index);
			score_str = strchr(entry, '\t');
			score_str++;
			score = atoi(score_str);

			if (score > max_score) {
				max_score = score;
				max_index = index;
			}
		}

		entry = (char *)ColorListInstance1->Get_Item(max_index);
		ColorListInstance2->Add_Item(entry, ColorListInstance1->Colors[max_index]);
		ColorListInstance1->Remove_Item(entry);
		count--; // TODO Copy/Paste error in original?
	}

	while (ColorListInstance2->Count() > 0) {
		entry = (char *)ColorListInstance2->Get_Item(0);
		ColorListInstance1->Add_Item(entry, ColorListInstance2->Colors[0]);
		ColorListInstance2->Remove_Item(entry);
	}
}

// Matching
int SidebarClass::Color_List_Find_Entry(const char *name)
{
	int index;
	const char *entry;
	char delim_char;
	char tab_char;

	for (index = 0; index < ColorListInstance1->Count(); index++) {
		entry = ColorListInstance1->Get_Item(index);
		delim_char = entry[strlen(name) + 1];
		tab_char = entry[strlen(name) + 2];

		if (strnicmp(entry + 1, name, strlen(name)) == 0 &&
			(delim_char == ')' || delim_char == ' ') &&
			tab_char == '\t') {
			return index;
		}
	}

	return -1;
}

// Matching
void SidebarClass::Color_List_Toggle_Spectator(const char *name, bool state)
{
	int index;
	char *name_start;
	char *name_end;

	index = Color_List_Find_Entry(name);

	if (index == -1) {
		return;
	}

	name_start = (char *)ColorListInstance1->Get_Item(index);
	name_end = strchr(name_start, '\t');
	name_end--;

	if (state) {
		*name_start = '(';
		*name_end = ')';
	} else {
		*name_start = ' ';
		*name_end = ' ';
	}
}

/***********************************************************************************************
 * SidebarClass::Zoom_Mode_Control -- Handles the zoom mode toggle operation.                  *
 *                                                                                             *
 *    This is the function that is called when the map button is pressed. It will toggle       *
 *    between the different modes that the radar map can assume.                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/31/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::Zoom_Mode_Control(void)
{
    /*
    ** If radar is active, cycle as follows:
    ** Zoomed => not zoomed
    ** not zoomed => player status (multiplayer only)
    ** player status => zoomed
    */
    if (IsRadarActive) {
        if (Is_Zoomed() || GameToPlay == GAME_NORMAL) {
            Zoom_Mode(Coord_Cell(TacticalCoord));
        } else {
            if (!Is_Player_Names()) {
                Player_Names(true);
            } else {
                Player_Names(false);
                Zoom_Mode(Coord_Cell(TacticalCoord));
            }
        }
    } else {
        if (GameToPlay != GAME_NORMAL) {
            Player_Names(Is_Player_Names() == 0);
        }
    }
}

SidebarClass::~SidebarClass()
{
    if (Repair) {
        delete Repair;
        Repair = NULL;
    }
    if (Upgrade) {
        delete Upgrade;
        Upgrade = NULL;
    }
    if (Zoom) {
        delete Zoom;
        Zoom = NULL;
    }
}
