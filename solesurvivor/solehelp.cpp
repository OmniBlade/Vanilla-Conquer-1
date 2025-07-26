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
#include "helpscrn.h"
#include "common/buffer.h"
#include "mssleep.h"

struct VPRect
{
	int XPos;
	int YPos;
	int Width;
	int Height;
};

bool Clip_Rect(VPRect &rect1, VPRect &rect2);

// Matching
void Help_Menu(void)
{
	HelpScreenClass help;
	const char *last_desc;
	bool breakout;
	int xpos;
	int ypos;
	int last_xpos;
	int last_ypos;
	const char* tool_tip;
	const char* description;
	BufferClass buff(0x2000);

	last_xpos = -1;
	last_ypos = -1;
	last_desc = NULL;


	if (buff.Get_Buffer() == NULL) {
		WWMessageBox().Process("Buffer allocation failed! (SSHELP)", TXT_OK);
		return;
	}

	if (!help.Load()) {
		WWMessageBox().Process("Configuration load failed! (SSHELP)", TXT_OK);
		return;
	}
	
	VPRect clip_rect;
	VPRect txt_clip;
	
	Fade_Palette_To(BlackPalette, 15, Call_Back);
	Hide_Mouse();
	Load_Title_Screen("SSHELP.PCX", &UnknownViewport2, Palette);
	memcpy(GamePalette, Palette, 768);
	UnknownViewport2.Blit(UnknownViewport1);
	Fade_Palette_To(Palette, 30, Call_Back);
	Show_Mouse();

	Set_Logic_Page(UnknownViewport2);
	Fancy_Text_Print(TXT_NONE, 0, 0, TBLACK, TBLACK, TPF_MAP | TPF_NOSHADOW);

	clip_rect.XPos = 0;
	clip_rect.YPos = 0;
	clip_rect.Width = 16;
	clip_rect.Height = 1;

	Keyboard->Clear();

	breakout = false;

	while (!breakout) {
		xpos = Get_Mouse_X();
		ypos = Get_Mouse_Y();
		if (xpos != last_xpos || ypos != last_ypos) {
			last_xpos = xpos;
			last_ypos = ypos;

			/*
			** Retrieve help text and prepare to display if an entry exists for where cursor is.
			*/
			if (help.Get_Entry(xpos, ypos, tool_tip, description)) {
				Hide_Mouse();

				txt_clip.XPos = xpos;
				txt_clip.YPos = ypos + 16;
				txt_clip.Width = String_Pixel_Width(tool_tip) + 3;
				txt_clip.Height = FontHeight + 2;

				/*
				** Keep the text displayed entirely within the screen boundaries.
				*/
				if (txt_clip.XPos + txt_clip.Width >= UnknownViewport2.Get_Width()) {
					txt_clip.XPos = UnknownViewport2.Get_Width() - txt_clip.Width;
				}

				if (txt_clip.YPos + txt_clip.Height >= UnknownViewport2.Get_Height() || ypos > 380 && ypos < 400 && xpos < 24) {
					txt_clip.YPos = ypos - FontHeight - 2;
				}

				/*
				** Save a copy of the area without text to use to remove text later.
				*/
				UnknownViewport2.To_Buffer(txt_clip.XPos, txt_clip.YPos, txt_clip.Width, txt_clip.Height, &buff);

				Fancy_Text_Print(tool_tip, txt_clip.XPos + 1, txt_clip.YPos + 1, YELLOW, BLACK, TPF_MAP | TPF_NOSHADOW);
				UnknownViewport2.Draw_Rect(
					txt_clip.XPos, txt_clip.YPos, txt_clip.XPos + txt_clip.Width - 1,  txt_clip.YPos + txt_clip.Height - 1, YELLOW);

				if (!Clip_Rect(clip_rect, txt_clip)) {
					UnknownViewport2.Blit(
						UnknownViewport1, txt_clip.XPos, txt_clip.YPos, txt_clip.XPos, txt_clip.YPos, txt_clip.Width, txt_clip.Height);
				}

				UnknownViewport2.Blit(
					UnknownViewport1, clip_rect.XPos, clip_rect.YPos, clip_rect.XPos, clip_rect.YPos, clip_rect.Width, clip_rect.Height);
				buff.To_Page(txt_clip.XPos, txt_clip.YPos, txt_clip.Width, txt_clip.Height, UnknownViewport2);
				clip_rect.XPos = txt_clip.XPos;
				clip_rect.YPos = txt_clip.YPos;
				clip_rect.Width = txt_clip.Width;
				clip_rect.Height = txt_clip.Height;

				if (description != last_desc) {
					if (last_desc != NULL || description == NULL) {
						UnknownViewport2.Blit(UnknownViewport1, 2, 402, 2, 402, 474, 76);
					}

					last_desc = description;

					if (description != NULL) {
						char string[512];
						int w = 470;
						int h = 74;

						Sound_Effect(VOC_UP, VOL_3);
						strcpy(string, description);
						Format_Window_String(string, 470, w, h);
						Set_Logic_Page(UnknownViewport1);
						Fancy_Text_Print(string, 2u, 402, 5u, 0, TPF_MAP | TPF_FULLSHADOW);
						Set_Logic_Page(UnknownViewport2);
					}
				}

				Show_Mouse();
			} else if (clip_rect.Width != 0 && clip_rect.Height != 0) {
				Hide_Mouse();
				UnknownViewport2.Blit(
					UnknownViewport1, clip_rect.XPos, clip_rect.YPos, clip_rect.XPos, clip_rect.YPos, clip_rect.Width, clip_rect.Height);
				clip_rect.Width = 0;
				UnknownViewport2.Blit(UnknownViewport1, 2, 402, 2, 402, 476, 76);
				last_desc = NULL;
				Show_Mouse();
			}
		}

		Call_Back();

		if (AllSurfaces.SurfacesRestored) {
			AllSurfaces.SurfacesRestored = false;
			Hide_Mouse();
			Load_Title_Screen("SSHELP.PCX", &UnknownViewport2, Palette);
			memcpy(GamePalette, Palette, 0x300u);
			UnknownViewport2.Blit(UnknownViewport1);
			Fade_Palette_To(Palette, FADE_PALETTE_SLOW, Call_Back);
			Show_Mouse();
			Set_Logic_Page(UnknownViewport2);
			last_xpos = -1;
			last_ypos = -1;
			last_desc = 0;
		}

		if (Keyboard->Check() != KN_NONE) {
			KeyNumType key = Keyboard->Get();

			if (key == KN_BUTTON || key == KN_ESC || key == KN_SPACE) {
				breakout = true;
			}
		}

		Wait_Vert_Blank();
		Set_Palette(GamePalette);
		ms_sleep(20);
	}

	Set_Logic_Page(SeenBuff);
	Hide_Mouse();
	Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);
	UnknownViewport2.Clear();
	UnknownViewport1.Clear();
	Show_Mouse();
}


bool Clip_Rect(VPRect &rect1, VPRect &rect2)
{
	bool clip_x = false;
	bool clip_y = false;
	
	int x1 = rect1.XPos + rect1.Width;
	int y1 = rect1.YPos + rect1.Height;
	int x2 = rect2.XPos + rect2.Width;
	int y2 = rect2.YPos + rect2.Height;
	
	if (rect1.XPos < rect2.XPos) {
		if ( x1 >= rect2.XPos )
		{
			clip_x = true;
		}
	} else if (x2 >= rect1.XPos) {
		clip_x = true;
	}
	
	if (rect1.YPos < rect2.YPos) {
		if ( y1 >= rect2.YPos )
		{
			clip_y = true;
		}
	} else if (y2 >= rect1.YPos) {
		clip_y = true;
	}

	if (clip_x && clip_y) {
		int x;
		int y;
		int w;
		int h;

		if (rect1.XPos < rect2.XPos) {
			x = rect1.XPos;
		} else {
			x = rect2.XPos;
		}
		rect1.XPos = x;
		
		if (rect1.YPos < rect2.YPos) {
			y = rect1.YPos;
		} else {
			y = rect2.YPos;
		}
		
		rect1.YPos = y;
		
		if (x1 > x2) {
			w = x1;
		} else {
			w = x2;
		}
		rect1.Width = w;
		rect1.Width -= rect1.XPos;
		
		if (y1 > y2) {
			h = y1;
		} else {
			h = y2;
		}
		rect1.Height = h;
		rect1.Height -= rect1.YPos;
		
		return true;
	}

	return false;
}
