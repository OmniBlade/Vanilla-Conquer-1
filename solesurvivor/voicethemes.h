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
#ifndef VOICETHEMES_H
#define VOICETHEMES_H

enum VoiceThemesType {
	VOX_THEME_EVA,
	VOX_THEME_COMMANDO,
	VOX_THEME_LETS_MAKE_A_KILL,
	VOX_THEME_1_900_KILL_YOU,
	VOX_THEME_COUNT,
};

class VoiceThemeClass
{
public:
	VoiceThemeClass()
	{
		//ThemeName[0] = '\0';
	}
	virtual ~VoiceThemeClass()
	{
	}
	virtual void Set_Theme_Name(const char* name)
	{
		strcpy(ThemeName, name);
	}
	virtual const char* Get_Theme_Name()
	{
		return ThemeName;
	}
	virtual void Play(VoiceSoundType type) = 0;

protected:
	char ThemeName[255];
};


class AudVoiceThemeClass : public VoiceThemeClass
{
public:
	AudVoiceThemeClass();
	virtual ~AudVoiceThemeClass()
	{
	}
	virtual void Play(VoiceSoundType type);
	virtual void Add(VoiceSoundType type, int data, int extra);

private:
	VoxType Vox[VOX_THEME_SND_COUNT];
	short Variations[VOX_THEME_SND_COUNT];
};


class WavVoiceThemeClass : public VoiceThemeClass
{
	enum
	{
		VARAIATION_COUNT = 3,
	};

public:
	WavVoiceThemeClass();
	virtual ~WavVoiceThemeClass()
	{
	}
	virtual void Play(VoiceSoundType type);
	virtual void Add(VoiceSoundType type, char *data);

private:
	char FileNames[VARAIATION_COUNT][VOX_THEME_SND_COUNT][260];
	short Variation[VOX_THEME_SND_COUNT];
};

#endif
