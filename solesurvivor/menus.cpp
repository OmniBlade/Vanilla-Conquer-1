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

/* $Header:   F:\projects\c&c\vcs\code\menus.cpv   2.17   16 Oct 1995 16:50:48   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : MENUS.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Phil W. Gorrow                                               *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : May 17, 1995 [BRR]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Main_Menu -- Menu processing                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"
#include "mssleep.h"
#include "internet.h"

/*****************************
**	Function prototypes
******************************/

#ifdef SCENARIO_EDITOR

PRIVATE int Coordinates_In_Region(int x, int y, int inx1, int iny1, int inx2, int iny2);
PRIVATE int Select_To_Entry(int select, unsigned int bitfield, int index);
PRIVATE void Flash_Line(char const* text, int xpix, int ypix, unsigned nfgc, unsigned hfgc, unsigned bgc);

int UnknownKey;

PRIVATE int MenuUpdate = 1;
PRIVATE int MenuSkip;

/*=========================================================================*/
/*	SELECT_TO_ENTRY:																			*/
/*																									*/
/*		This routine converts a selection to the correct string entry. It	   */
/*	does this by search through a long bitfield starting at position index	*/
/*	until it finds the correct conversion to entries.								*/
/*																									*/
/*	INPUTS:	int selection from menu, long the bit field to search, int 	   */
/*				the starting index within the bit field.								*/
/*	RETURNS:	int the index into the table of entries								*/
/*=========================================================================*/
PRIVATE int Select_To_Entry(int select, unsigned int bitfield, int index)
{
    int placement;

    if (bitfield == 0xFFFFFFFFL) /* if all bits are set	*/
        return (select);         /*		then it as is		*/

    placement = 0;                                  /* current pos zero		*/
    while (select) {                                /* while still ones		*/
        if (bitfield & (1L << (placement + index))) /* if this flagged then	*/
            select--;                               /* decrement counter		*/
        placement++;                                /* and we moved a place	*/
    }
    while (!(bitfield & (1L << (placement + index)))) {
        placement++;
    }

    return (placement); /* return the position	*/
}

/*=========================================================================*/
/*	FLASH_LINE:																					*/
/*																									*/
/*		This routine will flash the line at the desired location for the		*/
/*	menu routine. It is way cool awesome!												*/
/*																									*/
/*	INPUTS:	char *text, int x position on line, int y position, char		   */
/*				normal foreground color, char hilight foreground color, char 	*/
/*				background color																*/
/*	RETURNS:	none																				*/
/*=========================================================================*/
PRIVATE void Flash_Line(char const* text, int xpix, int ypix, unsigned nfgc, unsigned hfgc, unsigned bgc)
{
    int loop;

    for (loop = 0; loop < 3; loop++) {
        Hide_Mouse();
        Fancy_Text_Print(text, xpix, ypix, hfgc, bgc, TPF_8POINT | TPF_DROPSHADOW);
        Delay(2);
        Fancy_Text_Print(text, xpix, ypix, nfgc, bgc, TPF_8POINT | TPF_DROPSHADOW);
        Show_Mouse();
        Delay(2);
    }
}

/*=========================================================================*/
/*	COORDINATES_IN_REGION:																	*/
/*																									*/
/*		Test to see if a given pair of coordinates are within the given 		*/
/*	rectangular region.																		*/
/*																									*/
/*	INPUTS:	int x to be tested, int y to be tested, int left x pos,			*/
/*				int top y pos, int right x pos, int bottom y pos					*/
/*	RETURNS:	none																				*/
/*=========================================================================*/
PRIVATE int Coordinates_In_Region(int x, int y, int inx1, int iny1, int inx2, int iny2)
{
    return ((x >= inx1) && (x <= inx2) && (y >= iny1) && (y <= iny2));
}

#ifdef NEVER
/*=========================================================================*/
/*	FIND_MENU_ITEMS:																			*/
/*																									*/
/*		This routine finds the real total items in a menu when certain items	*/
/*	may be disabled by bit fields and the like. This is done by looping		*/
/*	through the fields, starting at the position passed in index and 			*/
/*	counting the number of bits that are set.											*/
/*																									*/
/*	INPUTS:	int the maximum number of items possible on the menu, long 		*/
/*				the bit field of enabled and disabled items, char the index		*/
/*				point to start at within the list.										*/
/*	RETURNS:	int the total number of items in the menu							*/
/*=========================================================================*/
int Find_Menu_Items(int maxitems, unsigned int field, char index)
{
    int loop, ctr;

    if (field == 0xFFFFFFFFL) /* if all bits are set	*/
        return (maxitems);    /* then maxitems set		*/

    for (loop = ctr = 0; loop < maxitems; loop++) { /* loop through items	*/
        if (field & (1L << (loop + index))) {       /* if the bit is set		*/
            ctr++;                                  /*		count the item		*/
        }
    }
    return (ctr);
}
#endif

/*=========================================================================*/
/*	SETUP_EOB_MONITOR_MENU:																	*/
/*																									*/
/*		This routine sets up the eye of the beholder monitor menu.				*/
/*																									*/
/*	INPUTS:	int the menu we are using, char *[] the array of text which		*/
/*				makes up the menu commands, long the info field, int the			*/
/*				index into the field, int the number of lines to skip.			*/
/*	RETURNS:	none																				*/
/*=========================================================================*/
void Setup_Menu(int menu, char const* text[], unsigned int field, int index, int skip)
{
    int *menuptr, lp;
    int menuy, menux, idx, item, num, drawy;

    menuptr = &MenuList[menu][0];         /* get pointer to menu	*/
    menuy = WinY + menuptr[MENUY];        /* get the absolute 		*/
    menux = (WinX + menuptr[MENUX]) << 3; /*		coords of menu		*/
    item = Select_To_Entry(menuptr[MSELECTED], field, index);
    num = menuptr[ITEMSHIGH];

    Fancy_Text_Print(0, 0, 0, TBLACK, TBLACK, TPF_8POINT | TPF_DROPSHADOW);
    Hide_Mouse();
    for (lp = 0; lp < num; lp++) {
        idx = Select_To_Entry(lp, field, index);
        drawy = menuy + (lp * FontHeight) + (lp * skip);
        Fancy_Text_Print(text[idx],
                         menux,
                         drawy,
                         menuptr[((idx == item) && (MenuUpdate)) ? HILITE : NORMCOL],
                         TBLACK,
                         TPF_8POINT | TPF_DROPSHADOW);
        //		if ((idx==item) && (MenuUpdate ))
        //			Text_Print(text[idx],menux,drawy,menuptr[HILITE],TBLACK);
    }
    MenuSkip = skip;
    Show_Mouse();
    Keyboard->Clear();
}

/*=========================================================================*/
/*	CHECK_MENU:																					*/
/*																									*/
/*																									*/
/*																									*/
/*	INPUTS:																						*/
/*	RETURNS:																						*/
/*=========================================================================*/
int Check_Menu(int menu, char const* text[], char*, int field, int index)
{
    int maxitem, select, key, menuy, menux;
    int mx1, mx2, my1, my2, tempy;
    int drawy, menuskip, halfskip;
    int normcol, litcol, item, newitem, idx;
    int* menuptr;

    // selection++;												/* get rid of warning	*/

    menuptr = &MenuList[menu][0];                        /* get pointer to menu	*/
    maxitem = menuptr[ITEMSHIGH] - 1;                    /* find max items			*/
    newitem = item = menuptr[MSELECTED] % (maxitem + 1); /* find selected 			*/
    select = -1;                                         /* no selection made		*/
    menuskip = FontHeight + MenuSkip;                    /* calc new font height	*/
    halfskip = MenuSkip >> 1;                            /* adjustment for menus	*/

    menuy = WinY + menuptr[MENUY];        /* get the absolute 		*/
    menux = (WinX + menuptr[MENUX]) << 3; /*		coords of menu		*/
    normcol = menuptr[NORMCOL];
    litcol = menuptr[HILITE];

    /*
    **	Fetch a pending keystroke from the buffer if there is a keystroke
    **	present. If no keystroke is pending then simple mouse tracking will
    **	be done.
    */
    key = 0;
    UnknownKey = 0;
    if (Keyboard->Check()) {
        key = (Keyboard->Get() & 0x18FF); /* mask off all but release bit	*/
    }

    /*
    **	if we are using the mouse and it is installed, then find the mouse
    **	coordinates of the menu and if we are not somewhere on the menu get
    **	the heck outta here. If we are somewhere on the menu, then figure
    **	out the new selected item, and continue forward.
    */
    mx1 = (WinX << 3) + (menuptr[MENUX] * FontWidth); /* get menu coords		*/
    my1 = (WinY) + (menuptr[MENUY]) - halfskip;       /*		from the menu		*/
    mx2 = mx1 + (menuptr[ITEMWIDTH] * FontWidth) - 1; /*		structure as		*/
    my2 = my1 + (menuptr[ITEMSHIGH] * menuskip) - 1;  /*		necessary			*/

    tempy = Get_Mouse_Y();
    if (Coordinates_In_Region(Get_Mouse_X(), tempy, mx1, my1, mx2, my2) && MenuUpdate) {
        newitem = (tempy - my1) / menuskip;
    }

    switch (key) {

    case KN_UP:                /* if the key moves up	*/
        newitem--;             /* 	new item up one	*/
        if (newitem < 0)       /* if invalid new item	*/
            newitem = maxitem; /* put at list bottom	*/
        break;
    case KN_DOWN:              /* if key moves down		*/
        newitem++;             /*		new item down one	*/
        if (newitem > maxitem) /* if new item past 		*/
            newitem = 0;       /*		list end, clear	*/
        break;
    case KN_HOME:    /* if top of list key 	*/
    case KN_PGUP:    /*		is selected then	*/
        newitem = 0; /*		new item = top		*/
        break;
    case KN_END:           /* if bottom of list is	*/
    case KN_PGDN:          /*		selected then		*/
        newitem = maxitem; /*		new item = bottom	*/
        break;

    /*
    **	Handle mouse button press. Set selection and then fall into the
    **	normal menu item select logic.
    */
    case KN_RMOUSE:
    case KN_LMOUSE:
        if (Coordinates_In_Region(Keyboard->MouseQX, Keyboard->MouseQY, mx1, my1, mx2, my2)) {
            newitem = (Keyboard->MouseQY - my1) / menuskip;
        } else {
            UnknownKey = key; //	Pass the unprocessed button click back.
            break;
        }

    /*
    **	Normal menu item select logic. Will flash line and exit with menu
    **	selection number.
    */
    case KN_RETURN: /* if a selection is 	*/
    case KN_SPACE:  /*		made with key		*/
    case KN_CENTER:
        select = newitem; /*		flag it made.		*/
        break;

    case 0:
        break;

    /*
    **	When no key was pressed or an unknown key was pressed, set the
    **	global record of the key and exit normally.
    **	EXCEPTION:	If the key matches the first letter of any of the
    **					menu entries, then presume it as a selection of
    **					that entry.
    */
    default:
        for (idx = 0; idx < menuptr[ITEMSHIGH]; idx++) {
            if (toupper(*(text[Select_To_Entry(idx, field, index)]))
                == toupper(Keyboard->To_ASCII((KeyNumType)(key & 0x0FF)))) {
                newitem = select = idx;
                break;
            }
        }
        UnknownKey = key;
        break;
    }

    if (newitem != item) {
        Hide_Mouse();
        idx = Select_To_Entry(item, field, index);
        drawy = menuy + (item * menuskip);
        Fancy_Text_Print(text[idx], menux, drawy, normcol, TBLACK, TPF_8POINT | TPF_DROPSHADOW);
        idx = Select_To_Entry(newitem, field, index);
        drawy = menuy + (newitem * menuskip);
        Fancy_Text_Print(text[idx], menux, drawy, litcol, TBLACK, TPF_8POINT | TPF_DROPSHADOW);
        Show_Mouse(); /* resurrect the mouse	*/
    }

    if (select != -1) {
        idx = Select_To_Entry(select, field, index);
        Hide_Mouse(); /* get rid of the mouse	*/
        drawy = menuy + (newitem * menuskip);
        Flash_Line(text[idx], menux, drawy, normcol, litcol, TBLACK);
        Show_Mouse();
        select = idx;
    }

    menuptr[MSELECTED] = newitem; /* update menu select	*/

    return (select);
}

/***************************************************************************
 * Do_Menu -- Generic menu processor.                                      *
 *                                                                         *
 *    This helper function displays a menu of specified entries and waits  *
 *    for the player to make a selection. If a selection is made, then     *
 *    a whole number (starting at 0) is returned matching the entry        *
 *    selected. If ESC is pressed, then -1 is returned.                    *
 *                                                                         *
 * INPUT:   strings  -- A pointer to an array of pointers to text strings. *
 *                      Each entry in the list will be a menu entry that   *
 *                      can be selected.                                   *
 *                                                                         *
 *          blue     -- Should the special blue color be used to display   *
 *                      the menu?                                          *
 *                                                                         *
 * OUTPUT:  Returns with the cardinal number of the selected menu entry.   *
 *          If ESC was pressed, then -1 is returned.                       *
 *                                                                         *
 * WARNINGS:   none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/16/1994 JLB : Created.                                             *
 *=========================================================================*/
int Do_Menu(char const** strings, bool blue)
{
    int count;        // Number of entries in this menu.
    int length;       // The width of the menu (in pixels).
    char const** ptr; // Working menu text pointer.
    int selection;    // Selection from user.

    if (!strings)
        return (-1);
    Set_Logic_Page(SeenBuff);
    Keyboard->Clear();

    /*
    **	Determine the number of entries in this string.
    */
    ptr = strings;
    count = 0;
    while (*ptr++) {
        count++;
    }
    MenuList[0][ITEMSHIGH] = count;

    /*
    **	Determine the width of the menu by finding the length of the
    **	longest menu entry.
    */
    Fancy_Text_Print(TXT_NONE, 0, 0, 0, 0, TPF_8POINT | TPF_DROPSHADOW);
    length = 0;
    ptr = strings;
    while (*ptr) {
        length = MAX(length, (int)String_Pixel_Width(*ptr));
        ptr++;
    }
    length += 7;
    MenuList[0][ITEMWIDTH] = length >> 3;

    /*
    **	Adjust the window values to match the size of the
    **	specified menu.
    */
    WindowList[WINDOW_MENU][WINDOWWIDTH] = MenuList[0][ITEMWIDTH] + 2;
    WindowList[WINDOW_MENU][WINDOWX] = 19 - (length >> 4);
    WindowList[WINDOW_MENU][WINDOWY] = 174 - (unsigned)(MenuList[0][ITEMSHIGH] * (FontHeight + FontYSpacing));
    WindowList[WINDOW_MENU][WINDOWHEIGHT] = MenuList[0][ITEMSHIGH] * FontHeight + 5 /*11*/;

    /*
    **	Display the menu.
    */
    Change_Window((int)WINDOW_MENU);
    Show_Mouse();
    Window_Box(WINDOW_MENU, blue ? BOXSTYLE_BLUE_UP : BOXSTYLE_RAISED);
    Setup_Menu(0, strings, 0xFFFFL, 0, 0);

    Keyboard->Clear();
    selection = -1;
    UnknownKey = 0;
    while (selection == -1) {
        Call_Back();
        selection = Check_Menu(0, strings, NULL, 0xFFL, 0);
        if (UnknownKey != 0 || UnknownKey == KN_ESC || UnknownKey == KN_LMOUSE || UnknownKey == KN_RMOUSE)
            break;

        Frame_Limiter();
    }
    Keyboard->Clear();
    Hide_Mouse();

    Blit_Hid_Page_To_Seen_Buff();
    Change_Window((int)WINDOW_MAIN);
    Map.Flag_To_Redraw(true);
    return (selection);
}
#endif

void Color_Cycle_Button_Text(CountDownTimerClass &ftimer, unsigned char *palette)
{
	static bool _up = false;

	if ( !ftimer.Time() ) {
		ftimer.Set(TIMER_SECOND/4);

		/*
		**	Pulse the pulsing text color.
		*/
		#define	MIN_BTN_CYCLE_COLOR	62
		#define	MAX_BTN_CYCLE_COLOR	63

		if (_up) {
			palette[13]++;
			if (palette[13] > MAX_BTN_CYCLE_COLOR) {
				palette[13] = MAX_BTN_CYCLE_COLOR;
				_up = false;
			}
		} else {
			palette[13]--;
			if (palette[13] < MIN_BTN_CYCLE_COLOR) {
				palette[13] = MIN_BTN_CYCLE_COLOR;
				_up = true;
			}
		}
		Wait_Vert_Blank();
		Set_Palette(palette);
	}
}

/***************************************************************************
 * Main_Menu -- Menu processing                                            *
 *                                                                         *
 * INPUT:                                                                  *
 *		none.																						*
 *                                                                         *
 * OUTPUT:                                                                 *
 *		index of item selected, -1 if time out											*
 *                                                                         *
 * WARNINGS:                                                               *
 *		none.																						*
 *                                                                         *
 * HISTORY:                                                                *
 *   05/17/1995 BRR : Created.                                             *
 *=========================================================================*/
int Main_Menu(unsigned int timeout)
{
	enum {
		D_DIALOG_W = 152*2,
		D_DIALOG_H = 136*2,
		D_DIALOG_X = 85*2,
		D_DIALOG_Y = 0,
		D_DIALOG_CX = D_DIALOG_X + (D_DIALOG_W / 2),

		D_OFFLINE_W = 125*2,
		D_OFFLINE_H = 9*2,
		D_OFFLINE_X = 195,
		D_OFFLINE_Y = 167,

		D_ONLINE_W = 125*2,
		D_ONLINE_H = 9*2,
		D_ONLINE_X = 195,
		D_ONLINE_Y = 195,
		
		D_HELP_W = 125*2,
		D_HELP_H = 9*2,
		D_HELP_X = 195,
		D_HELP_Y = 223,

		D_SNEAK_W = 125*2,
		D_SNEAK_H = 9*2,
		D_SNEAK_X = 195,
		D_SNEAK_Y = 251,

		D_FIVE_W = 125*2,
		D_FIVE_H = 9*2,
		D_FIVE_X = 195,
		D_FIVE_Y = 279,

		D_SIX_W = 125*2,
		D_SIX_H = 9*2,
		D_SIX_X = 195,
		D_SIX_Y = 307,

		D_NEWS_W = 125*2,
		D_NEWS_H = 9*2,
		D_NEWS_X = 195,
		D_NEWS_Y = 335,	

		D_EXIT_W = 83*2,
		D_EXIT_H = 9*2,
		D_EXIT_X = 237,
		D_EXIT_Y = 363,

	};
	enum {
		BUTTON_OFFLINE=100*2,
		BUTTON_ONLINE,
		BUTTON_HELP,
		BUTTON_SNEAK,
		BUTTON_FIVE,
		BUTTON_SIX,
		BUTTON_NEWS,
		BUTTON_EXIT,
	};

	//needed for matching bss, not sure if this was this function's static, but it's a static
	static char unknown[256];

	KeyNumType input;								// input from user
	static int retval = SEL_NONE;					// return value
	static int curbutton;
	TextButtonClass *buttons[8];
	unsigned long starttime;

	int numbuttons = 8;
	ControlClass *commands = NULL;				// the button list
	int butt = 0;

	TextButtonClass offlinebtn (BUTTON_OFFLINE, TXT_PRACTICE,
		TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW,
		D_OFFLINE_X, D_OFFLINE_Y, D_OFFLINE_W, D_OFFLINE_H);

	TextButtonClass onlinebtn (BUTTON_ONLINE, TXT_PLAY_ONLINE,
		TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW,
		D_ONLINE_X, D_ONLINE_Y, D_ONLINE_W, D_ONLINE_H);

	TextButtonClass helpbtn (BUTTON_HELP, TXT_HELP,
		TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW,
		D_HELP_X, D_HELP_Y, D_HELP_W, D_HELP_H);

	TextButtonClass sneakbtn (BUTTON_SNEAK, TXT_SNEAK_PEEK,
		TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW,
		D_SNEAK_X, D_SNEAK_Y, D_SNEAK_W, D_SNEAK_H);

	TextButtonClass fivebtn (BUTTON_FIVE, ButtonFiveText,
		TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW,
		D_FIVE_X, D_FIVE_Y, D_FIVE_W, D_FIVE_H);

	TextButtonClass sixbtn (BUTTON_SIX, ButtonSixText,
		TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW,
		D_SIX_X, D_SIX_Y, D_SIX_W, D_SIX_H);

	TextButtonClass newsbtn (BUTTON_NEWS, TXT_NEWS,
		TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW,
		D_NEWS_X, D_NEWS_Y, D_NEWS_W, D_NEWS_H);

	TextButtonClass exitbtn (BUTTON_EXIT, TXT_EXIT_GAME,
		TPF_CENTER | TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW,
		D_EXIT_X, D_EXIT_Y, D_EXIT_W, D_EXIT_H);

	/*
	**	Initialize
	*/
	Set_Logic_Page(UnknownViewport1);
	Keyboard->Clear();
	starttime = WinTickCount.Time();
	/*
	**	Create the list
	*/
	commands = &offlinebtn;

	onlinebtn.Add_Tail(*commands);
	helpbtn.Add_Tail(*commands);
	sneakbtn.Add_Tail(*commands);
	fivebtn.Add_Tail(*commands);
	sixbtn.Add_Tail(*commands);
	newsbtn.Add_Tail(*commands);
	exitbtn.Add_Tail(*commands);

	/*
	**	Fill array of button ptrs
	*/
	butt = 0;

	buttons[butt++] = &offlinebtn;
	buttons[butt++] = &onlinebtn;
	buttons[butt++] = &helpbtn;
	buttons[butt++] = &sneakbtn;
	buttons[butt++] = &fivebtn;
	buttons[butt++] = &sixbtn;
	buttons[butt++] = &newsbtn;
	buttons[butt++] = &exitbtn;
	
	buttons[curbutton]->Turn_On();
	buttons[curbutton]->Flag_To_Redraw();

	Keyboard->Clear();

	Fancy_Text_Print(TXT_NONE, 0, 0, CC_GREEN, TBLACK, TPF_CENTER|TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW);

	/*
	**	Main Processing Loop.
	*/
	bool display = true;
	bool process = true;
	while (process) {

		CountDownTimerClass timer(BT_SYSTEM, 0);
		/*
		** If we have just received input focus again after running in the background then
		** we need to redraw.
		*/
		if (AllSurfaces.SurfacesRestored){
			AllSurfaces.SurfacesRestored=false;
			display=true;
		}

		/*
		**	If timeout expires, bail
		*/
		if (timeout && WinTickCount.Time() - starttime > timeout) {
			retval = SEL_TIMEOUT;
			process = false;
		}

		/*
		**	Invoke game callback.
		*/
		Call_Back();

		/*
		**	Refresh display if needed.
		*/
		if (display) {

			Hide_Mouse();
			/*
			**	Load the background picture.
			*/
			Load_Title_Screen("HTITLE.PCX", &UnknownViewport2, Palette);
			Set_Logic_Page(UnknownViewport2);
			Set_Palette(Palette);
			UnknownViewport2.Blit(UnknownViewport1);

			/*
			**	Display the title and text overlay for the menu.
			*/
			Set_Logic_Page(UnknownViewport1);
			offlinebtn.Draw_All();
			Show_Mouse();
			display = false;
		}

		Color_Cycle_Button_Text(timer, Palette);

		/*
		**	Get and process player input.
		*/
		input = commands->Input();
		switch (input) {

			case (BUTTON_ONLINE | KN_BUTTON):
				buttons[curbutton]->Turn_Off();
				buttons[curbutton]->Draw_Me(true);
				retval = SEL_NONE;
				curbutton = (input & 0x7FFF) - BUTTON_OFFLINE;
				buttons[curbutton]->IsPressed = true;
				Hide_Mouse();
				buttons[curbutton]->Draw_Me(true);
				Show_Mouse();
				process = false;
				if (SetMenuChoiceTo1) {
					retval = SEL_ONLINE;
					process = false;
				}
				// else if (!Spawn_WChat()) {
				//	WWMessageBox().Process(Text_String(TXT_CANNOT_FIND_WOL), TXT_OK);
				//}
				break;

			case (BUTTON_OFFLINE | KN_BUTTON):
			case (BUTTON_HELP | KN_BUTTON):
			case (BUTTON_SNEAK | KN_BUTTON):
			case (BUTTON_FIVE | KN_BUTTON):
			case (BUTTON_SIX | KN_BUTTON):
			case (BUTTON_NEWS | KN_BUTTON):
			case (BUTTON_EXIT | KN_BUTTON):
				buttons[curbutton]->Turn_Off();
				buttons[curbutton]->Draw_Me(true);
				retval = (input & 0x7FFF) - BUTTON_OFFLINE;
				curbutton = retval;
				buttons[curbutton]->IsPressed = true;
				Hide_Mouse();
				buttons[curbutton]->Draw_Me(true);
				Show_Mouse();
				process = false;
				break;

			case KN_UP:
				Hide_Mouse();
				buttons[curbutton]->Turn_Off();
				buttons[curbutton]->Draw_Me(true);
				curbutton--;

				if (curbutton < 0) {
					curbutton = numbuttons - 1;
				}
				buttons[curbutton]->Turn_On();
				buttons[curbutton]->Draw_Me(true);
				Show_Mouse();
				break;

			case KN_DOWN:
				Hide_Mouse();
				buttons[curbutton]->Turn_Off();
				buttons[curbutton]->Draw_Me(true);
				curbutton++;

				if (curbutton > numbuttons - 1) {
					curbutton = 0;
				}

				buttons[curbutton]->Turn_On();
				buttons[curbutton]->Draw_Me(true);
				Show_Mouse();
				break;

			case KN_RETURN:
				if (curbutton == SEL_ONLINE) {
					retval = SEL_NONE;
					buttons[curbutton]->IsPressed = true;
					Hide_Mouse();
					buttons[curbutton]->Draw_Me(true);
					Show_Mouse();
					process = false;
					//if (!Spawn_WChat()) {
					//	WWMessageBox().Process(Text_String(TXT_CANNOT_FIND_WOL), TXT_OK);
					//}
				} else {
					buttons[curbutton]->IsPressed = true;
					Hide_Mouse();
					buttons[curbutton]->Draw_Me(true);
					Show_Mouse();
					retval = curbutton;
					process = false;
				}
				break;

			default:
				break;
		}
		
		ms_sleep(50);
		//if (DDEShutdown) {
		//	process = false;
		//	retval = SEL_EXIT;
		//}
		
		//if (Read_Game_Options(NULL)) {
		//	ThemeType theme = Theme.What_Is_Playing();
		//	if (theme != THEME_MAP1) {
		//		Theme.Play_Song(THEME_NONE);
		//		Theme.AI();
		//	}
		//	
		//	//if (!SpawnedFromWChat) {
		//	//	Fade_Palette_To(BlackPalette, 0xF, 0);
		//	//	VisiblePage.Clear();
		//	//	ShowWindow(Get_WChat_Handle(), 6);
		//	//	ShowWindow(MainWindow, 9);
		//	//}
		//	retval = SEL_ONLINE;
		//	process = false;
		//}
	}
	

	//SpawnedFromWChat = false;
	return(retval);
}
