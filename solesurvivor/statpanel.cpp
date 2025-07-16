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
#include "statpanel.h"

StatsPanelClass::StatsPanelClass(void)
{
	IsEnabled = false;
}

StatsPanelClass::~StatsPanelClass(void)
{
	
}

extern int Sum_Object_Stats(ObjectClass *);

short TeamPlayerCounts[4];
int TimeLeftTimerVal = -1;

//stack doesn't match
void StatsPanelClass::Render(void)
{
	bool some_bool;
	int y;
	int time_left;
	int index;
	int player_count;
	char buffer[80];
	int sum;
	
	y = 401;
	player_count = 0;
	
	for (index = 0; index < SidebarClass::ColorListInstance1->Count(); index++) {
		if (SidebarClass::ColorListInstance1->Colors[index] != 13) {
			player_count++;
		}
	}
	some_bool = GameParams.IsMaxNumAIsScaled && GameParams.NumTeams == 1 && !GameParams.AllowNoTeam;
	if (!some_bool) {
		player_count -= GameParams.NumTeams;
	}
	
	for (index = 0; index < 4; index++) {
		TeamPlayerCounts[index] = 0;
	}
	
	for (index = 0; index < SidebarClass::ColorListInstance1->Count(); index++) {
		if (SidebarClass::ColorListInstance1->Colors[index] == (char)219) {
			TeamPlayerCounts[0]++;
		}
		else if (SidebarClass::ColorListInstance1->Colors[index] == 25 ) {
			TeamPlayerCounts[1]++;
		}
		else if (SidebarClass::ColorListInstance1->Colors[index] == 3 ) {
			TeamPlayerCounts[2]++;
		}
		else if (SidebarClass::ColorListInstance1->Colors[index] == (char)210) {
			TeamPlayerCounts[3]++;
		}
	}
	
	if (!some_bool) {
		for ( index = 0; index < GameParams.NumTeams; index++) {
			--TeamPlayerCounts[index];
		}
	}
	
	if (!OfflineMode) {
		int v44 = TeamPlayerCounts[0] + TeamPlayerCounts[1] + TeamPlayerCounts[2] + TeamPlayerCounts[3];
		if ( v44 > 0 ) {
			sprintf(buffer, Text_String(TXT_NUM_PLAYERS_COLON), player_count);
		} else {
			sprintf(buffer, Text_String(TXT_NUM_PLAYERS), player_count);
		}
				
		Conquer_Clip_Text_Print(buffer, 482u, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160u, 0);
		if ( GameParams.NumTeams > 0 )
		{
			sprintf(buffer, "%2d", TeamPlayerCounts[0]);
			Conquer_Clip_Text_Print(buffer, 557u, y, 0xDBu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160u, 0);
		}
		if ( GameParams.NumTeams > 1 )
		{
			sprintf(buffer, "%2d", TeamPlayerCounts[1]);
			Conquer_Clip_Text_Print(buffer, 578u, y, 0x19u, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160u, 0);
		}
		if ( GameParams.NumTeams > 2 )
		{
			sprintf(buffer, "%2d", TeamPlayerCounts[2]);
			Conquer_Clip_Text_Print(buffer, 599u, y, 3u, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160u, 0);
		}
		if ( GameParams.NumTeams > 3 )
		{
			sprintf(buffer, "%2d", TeamPlayerCounts[3]);
			Conquer_Clip_Text_Print(buffer, 620u, y, 0xD2u, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160u, 0);
		}
	}
	y += 12;
	if ( GameParams.TimeLimit > 0 ) {
		time_left = 60 * GameParams.TimeLimit - WDTGameTimer.Time() / 60;
		
		if (time_left > 60) {
			sprintf(buffer, Text_String(TXT_TIME_LEFT_MINS), (time_left - 1) / 60 + 1);
			Conquer_Clip_Text_Print(buffer, 482u, y, 0xFu, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160u, 0);
		} else {
			if ( time_left < 0 )
			{
				time_left = 0;
			}
			sprintf(buffer, Text_String(TXT_TIME_LEFT_SECS), time_left);
			Conquer_Clip_Text_Print(buffer, 482, y, 5u, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160u, 0);
			
			if ( OfflineMode && time_left <= 5 && time_left != TimeLeftTimerVal ) {
				if ( time_left > 0 ) {
					Sound_Effect(VOC_TARGET);
				}
				
				TimeLeftTimerVal = time_left;
			}
		}
		y += 12;
	}
	
	if (GameParams.LifeLimit > 0) {
		sprintf(buffer, Text_String(TXT_LIVES_LEFT), GameParams.LifeLimit - (PlayerPtr->Int1 - 1));
		Conquer_Clip_Text_Print(buffer, 482u, y, 15u, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160u, 0);
		y += 12;
	}
	
	if (GameParams.ScoreLimit > 0) {
		sprintf(buffer, Text_String(TXT_SCORE_LIMIT), GameParams.ScoreLimit);
		Conquer_Clip_Text_Print(buffer, 482u, y, 15u, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160u, 0);
		y += 12;
	}
	
	if (GameParams.IsCrates && CurrentObject.Count() > 0 && CurrentObject[0]->Is_Techno()) {
		sum = Sum_Object_Stats(CurrentObject[0]);
		int iondanger = 100 * sum / WDTCrateIonFactor;
		sprintf(buffer, "%s ", Text_String(TXT_ION_DANGER));
		int remapper;

		if (!GameParams.IonCannon || iondanger < 40 || CurrentObject[0]->What_Am_I() == RTTI_INFANTRY) {
			remapper = 4;
			sprintf(&buffer[strlen(buffer)], Text_String(TXT_ION_DANGER_NONE));
		} else if (iondanger < 55) {
			remapper = 5;
			sprintf(&buffer[strlen(buffer)], Text_String(TXT_ION_DANGER_LOW));
		} else {
			if (iondanger < 70) {
				remapper = 25;
				sprintf(&buffer[strlen(buffer)], Text_String(TXT_ION_DANGER_MEDIUM));
			} else {
				remapper = 8;
				sprintf(&buffer[strlen(buffer)], Text_String(TXT_ION_DANGER_HIGH));
			}
			
		}
		
		Conquer_Clip_Text_Print(buffer, 482u, y, remapper, 0, TPF_6POINT | TPF_NOSHADOW | TPF_BRIGHT_COLOR, 160u, 0);
		
		y += 12;
	}
}
