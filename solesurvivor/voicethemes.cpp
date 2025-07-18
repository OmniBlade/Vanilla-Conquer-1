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

AudVoiceThemeClass::AudVoiceThemeClass()
{
	for (VoiceSoundType i = VOX_THEME_SND_FIRST; i < VOX_THEME_SND_COUNT; i++) {
		Vox[i] = VOX_NONE;
		Variations[i] = 0;
	}
	ThemeName[0] = '\0';
}

void AudVoiceThemeClass::Add(VoiceSoundType type, int data, int extra)
{
	if (type >= VOX_THEME_SND_FIRST && type < VOX_THEME_SND_COUNT) {
		Vox[type] = (VoxType)(data);
		Variations[type] = extra;
	}
}

void AudVoiceThemeClass::Play(VoiceSoundType type)
{
	if (Vox[(char)type] == VOX_NONE) {
		return;
	}

	int offset = 0;

	if ((signed int)(unsigned short)Variations[(char)type] > 1) {
		offset = WDT_Random_Pick(0, (unsigned short)Variations[(char)type] - 1);
	}

	Speak((VoxType)(Vox[(char)type] + offset));
}

WavVoiceThemeClass::WavVoiceThemeClass()
{
	VoiceSoundType j;
	int i;
	for (i = 0; i < VARAIATION_COUNT; i++) {
		for (j = VOX_THEME_SND_FIRST; j < VOX_THEME_SND_COUNT; j++) {
			FileNames[i][j][0] = '\0';
		}
	}
}

void WavVoiceThemeClass::Add(VoiceSoundType type, char *data)
{
	char str[MAX_PATH];
	if (type < VOX_THEME_SND_FIRST && type >= VOX_THEME_SND_COUNT) {
		return;
	}
	
	strcpy(str, data);
	char* name = strtok(str, ";");
	Variation[type] = 0;

	while (name != NULL) {
		strcpy(FileNames[(unsigned short)Variation[type]][type], name);

		Variation[type]++;
		if ((unsigned short)Variation[type] >= VARAIATION_COUNT) {
			break;
		}

		name = strtok(NULL, ";");
	}
}

void WavVoiceThemeClass::Play(VoiceSoundType type)
{
	bool special = 0;
	
	if (type == VOX_THEME_SND_ION || type == VOX_THEME_SND_ARMAGEDDON) {
		special = 1;
	}
	
	int var = 0;
	if ((unsigned short)Variation[type] > 1) {
		var = WDT_Random_Pick(0, (unsigned short)Variation[type] - 1);
	}
	
	char *filename = FileNames[var][type];
	if (strlen(filename) > 0) {
		Play_Wave(filename, special);
	}
}
