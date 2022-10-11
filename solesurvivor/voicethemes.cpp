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
#include "voicethemes.h"
#include "function.h"


class AudVoiceThemeClass : public VoiceThemeClass
{
public:
    AudVoiceThemeClass();
    virtual ~AudVoiceThemeClass()
    {
    }
    virtual void Play(VoiceSoundType type) override;
    virtual void Add(VoiceSoundType type, uintptr_t data, int extra) override;

private:
    VoxType Vox[VOX_THEME_SND_COUNT];
    int Variations[VOX_THEME_SND_COUNT];
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
    virtual void Play(VoiceSoundType type) override;
    virtual void Add(VoiceSoundType type, uintptr_t data, int extra) override;

private:
    char FileNames[VARAIATION_COUNT][VOX_THEME_SND_COUNT][260];
    int Variation[VOX_THEME_SND_COUNT];
};

AudVoiceThemeClass::AudVoiceThemeClass()
{
    for (VoiceSoundType i = VOX_THEME_SND_FIRST; i < VOX_THEME_SND_COUNT; ++i) {
        Vox[i] = VOX_NONE;
        Variations[i] = 0;
    }
}

void AudVoiceThemeClass::Play(VoiceSoundType type)
{
    if (Vox[type] == VOX_NONE) {
        return;
    }

    int offset = 0;

    if (Variations[type] > 1) {
        offset = Random_Pick(0, Variations[type] - 1);
    }

    Speak(static_cast<VoxType>(Vox[type] + offset));
}

void AudVoiceThemeClass::Add(VoiceSoundType type, uintptr_t data, int extra)
{
    Vox[type] = static_cast<VoxType>(data);
    Variations[type] = extra;
}

WavVoiceThemeClass::WavVoiceThemeClass()
{
    for (int i = 0; i < VARAIATION_COUNT; ++i) {
        for (VoiceSoundType j = VOX_THEME_SND_FIRST; j < VOX_THEME_SND_COUNT; ++j) {
            FileNames[i][j][0] = '\0';
        }

        Variation[i] = 0;
    }
}

void WavVoiceThemeClass::Play(VoiceSoundType type)
{
    // TODO Needs Play_Wave code adding to audio engine.
}

void WavVoiceThemeClass::Add(VoiceSoundType type, uintptr_t data, int extra)
{
    if (type >= VOX_THEME_SND_FIRST && type < VOX_THEME_SND_COUNT) {
        char str[MAX_PATH];
        strcpy(str, reinterpret_cast<const char*>(data));
        char* name = strtok(str, ";");
        Variation[type] = 0;

        while (name != nullptr) {
            strcpy(this->FileNames[Variation[type]][type], name);

            if (++Variation[type] >= VARAIATION_COUNT) {
                break;
            }

            name = strtok(nullptr, ";");
        }
    }
}

// TODO: Init_Voice_Themes should be in audio engine, but not practical to add there as its shared.
// Add to init.cpp perhaps as that is custom per game or keep here?
void Init_Voice_Themes()
{
    // No audio theme.
    VoiceThemeClass* vtc = new AudVoiceThemeClass;
    vtc->Set_Theme_Name(Text_String(TXT_SS_NONE));
    VoiceThemes.Add(vtc);

    // Eva theme.
    vtc = new AudVoiceThemeClass;
    vtc->Set_Theme_Name("EVA");
    vtc->Add(VOX_THEME_SND_ARMOR, VOX_E_ARMOR1, 1);
    vtc->Add(VOX_THEME_SND_MEGAARMOR, VOX_E_MEGAA1, 1);
    vtc->Add(VOX_THEME_SND_WEAPON, VOX_E_WEAPN1, 1);
    vtc->Add(VOX_THEME_SND_MEGAWEAPON, VOX_E_MEGAW1, 1);
    vtc->Add(VOX_THEME_SND_SPEED, VOX_E_SPEED1, 1);
    vtc->Add(VOX_THEME_SND_MEGASPEED, VOX_E_MEGAS1, 1);
    vtc->Add(VOX_THEME_SND_RAPIDRELOAD, VOX_E_RAPID1, 1);
    vtc->Add(VOX_THEME_SND_MEGARAPIDRELOAD, VOX_E_MEGARR, 1);
    vtc->Add(VOX_THEME_SND_RANGE, VOX_E_RANGE1, 1);
    vtc->Add(VOX_THEME_SND_MEGARANGE, VOX_E_MGARNG, 1);
    vtc->Add(VOX_THEME_SND_HEAL, VOX_E_HEAL1, 1);
    vtc->Add(VOX_THEME_SND_STEALTHON, VOX_E_STLTHA, 1);
    vtc->Add(VOX_THEME_SND_STEALTHOFF, VOX_E_STLTHD, 1);
    vtc->Add(VOX_THEME_SND_TELEPORT, VOX_E_TELEP1, 1);
    vtc->Add(VOX_THEME_SND_NUKE, VOX_E_NUKE1, 1);
    vtc->Add(VOX_THEME_SND_ION, VOX_E_ION1, 1);
    vtc->Add(VOX_THEME_SND_UNCLOAKALL, VOX_E_STLALL, 1);
    vtc->Add(VOX_THEME_SND_RESHROUD, VOX_E_DRK1, 1);
    vtc->Add(VOX_THEME_SND_UNSHROUD, VOX_E_MAPUP1, 1);
    vtc->Add(VOX_THEME_SND_RADAR, VOX_E_RADAR1, 1);
    vtc->Add(VOX_THEME_SND_ARMAGEDDON, VOX_E_ARMGD1, 1);
    vtc->Add(VOX_THEME_SND_YOULOSE, VOX_E_LOSER1, 2);
    vtc->Add(VOX_THEME_SND_TEST, VOX_E_HELLO1, 1);
    vtc->Add(VOX_THEME_SND_UNIT1, VOX_E_UNIT1, 1);
    VoiceThemes.Add(vtc);

    // Commando theme.
    vtc = new AudVoiceThemeClass;
    vtc->Set_Theme_Name("Commando");
    vtc->Add(VOX_THEME_SND_ARMOR, VOX_C_ARMOR1, 1);
    vtc->Add(VOX_THEME_SND_MEGAARMOR, VOX_C_MEGAA1, 1);
    vtc->Add(VOX_THEME_SND_WEAPON, VOX_C_WEAPN1, 1);
    vtc->Add(VOX_THEME_SND_MEGAWEAPON, VOX_C_MEGAW1, 1);
    vtc->Add(VOX_THEME_SND_SPEED, VOX_C_SPEED1, 1);
    vtc->Add(VOX_THEME_SND_MEGASPEED, VOX_C_MEGAS1, 1);
    vtc->Add(VOX_THEME_SND_RAPIDRELOAD, VOX_C_RAPID1, 1);
    vtc->Add(VOX_THEME_SND_MEGARAPIDRELOAD, VOX_C_MEGARR, 1);
    vtc->Add(VOX_THEME_SND_RANGE, VOX_C_RANGE1, 1);
    vtc->Add(VOX_THEME_SND_MEGARANGE, VOX_C_MGARNG, 1);
    vtc->Add(VOX_THEME_SND_HEAL, VOX_C_HEAL1, 1);
    vtc->Add(VOX_THEME_SND_STEALTHON, VOX_C_STLTHA, 1);
    vtc->Add(VOX_THEME_SND_STEALTHOFF, VOX_C_STLTHD, 1);
    vtc->Add(VOX_THEME_SND_TELEPORT, VOX_C_TELEP1, 1);
    vtc->Add(VOX_THEME_SND_NUKE, VOX_C_NUKE1, 1);
    vtc->Add(VOX_THEME_SND_ION, VOX_C_ION1, 1);
    vtc->Add(VOX_THEME_SND_UNCLOAKALL, VOX_C_STLALL, 1);
    vtc->Add(VOX_THEME_SND_RESHROUD, VOX_C_DRK1, 1);
    vtc->Add(VOX_THEME_SND_UNSHROUD, VOX_C_MAPUP1, 1);
    vtc->Add(VOX_THEME_SND_RADAR, VOX_C_RADAR1, 1);
    vtc->Add(VOX_THEME_SND_ARMAGEDDON, VOX_C_ARMGD1, 1);
    vtc->Add(VOX_THEME_SND_YOULOSE, VOX_C_LOSER1, 2);
    vtc->Add(VOX_THEME_SND_TEST, VOX_C_HELLO1, 1);
    vtc->Add(VOX_THEME_SND_UNIT1, VOX_C_UNIT1, 1);
    VoiceThemes.Add(vtc);

    // Let's Make a Kill theme
    vtc = new AudVoiceThemeClass;
    vtc->Set_Theme_Name(Text_String(TXT_LETS_MAKE_KILL));
    vtc->Add(VOX_THEME_SND_ARMOR, VOX_M_ARMOR1, 1);
    vtc->Add(VOX_THEME_SND_MEGAARMOR, VOX_M_MEGAA1, 1);
    vtc->Add(VOX_THEME_SND_WEAPON, VOX_M_WEAPN1, 1);
    vtc->Add(VOX_THEME_SND_MEGAWEAPON, VOX_M_MEGAW1, 1);
    vtc->Add(VOX_THEME_SND_SPEED, VOX_M_SPEED1, 1);
    vtc->Add(VOX_THEME_SND_MEGASPEED, VOX_M_MEGAS1, 1);
    vtc->Add(VOX_THEME_SND_RAPIDRELOAD, VOX_M_RAPID1, 1);
    vtc->Add(VOX_THEME_SND_MEGARAPIDRELOAD, VOX_M_MEGARR, 1);
    vtc->Add(VOX_THEME_SND_RANGE, VOX_M_RANGE1, 1);
    vtc->Add(VOX_THEME_SND_MEGARANGE, VOX_M_MGARNG, 1);
    vtc->Add(VOX_THEME_SND_HEAL, VOX_M_HEAL1, 1);
    vtc->Add(VOX_THEME_SND_STEALTHON, VOX_M_STLTHA, 1);
    vtc->Add(VOX_THEME_SND_STEALTHOFF, VOX_M_STLTHD, 1);
    vtc->Add(VOX_THEME_SND_TELEPORT, VOX_M_TELEP1, 1);
    vtc->Add(VOX_THEME_SND_NUKE, VOX_M_NUKE1, 1);
    vtc->Add(VOX_THEME_SND_ION, VOX_M_ION1, 1);
    vtc->Add(VOX_THEME_SND_UNCLOAKALL, VOX_M_STLALL, 1);
    vtc->Add(VOX_THEME_SND_RESHROUD, VOX_M_DRK1, 1);
    vtc->Add(VOX_THEME_SND_UNSHROUD, VOX_M_MAPUP1, 1);
    vtc->Add(VOX_THEME_SND_RADAR, VOX_M_RADAR1, 1);
    vtc->Add(VOX_THEME_SND_ARMAGEDDON, VOX_M_ARMGD1, 1);
    vtc->Add(VOX_THEME_SND_YOULOSE, VOX_M_LOSER1, 2);
    vtc->Add(VOX_THEME_SND_TEST, VOX_M_HELLO1, 1);
    vtc->Add(VOX_THEME_SND_UNIT1, VOX_M_UNIT1, 1);
    VoiceThemes.Add(vtc);

    // 1-900-KILL-YOU theme
    vtc = new AudVoiceThemeClass;
    vtc->Set_Theme_Name(Text_String(TXT_1_900_KILL_YOU));
    vtc->Add(VOX_THEME_SND_ARMOR, VOX_S_ARMOR1, 1);
    vtc->Add(VOX_THEME_SND_MEGAARMOR, VOX_S_MEGAA1, 1);
    vtc->Add(VOX_THEME_SND_WEAPON, VOX_S_WEAPN1, 1);
    vtc->Add(VOX_THEME_SND_MEGAWEAPON, VOX_S_MEGAW1, 1);
    vtc->Add(VOX_THEME_SND_SPEED, VOX_S_SPEED1, 1);
    vtc->Add(VOX_THEME_SND_MEGASPEED, VOX_S_MEGAS1, 1);
    vtc->Add(VOX_THEME_SND_RAPIDRELOAD, VOX_S_RAPID1, 1);
    vtc->Add(VOX_THEME_SND_MEGARAPIDRELOAD, VOX_S_MEGARR, 1);
    vtc->Add(VOX_THEME_SND_RANGE, VOX_S_RANGE1, 1);
    vtc->Add(VOX_THEME_SND_MEGARANGE, VOX_S_MGARNG, 1);
    vtc->Add(VOX_THEME_SND_HEAL, VOX_S_HEAL1, 1);
    vtc->Add(VOX_THEME_SND_STEALTHON, VOX_S_STLTHA, 1);
    vtc->Add(VOX_THEME_SND_STEALTHOFF, VOX_S_STLTHD, 1);
    vtc->Add(VOX_THEME_SND_TELEPORT, VOX_S_TELEP1, 1);
    vtc->Add(VOX_THEME_SND_NUKE, VOX_S_NUKE1, 1);
    vtc->Add(VOX_THEME_SND_ION, VOX_S_ION1, 1);
    vtc->Add(VOX_THEME_SND_UNCLOAKALL, VOX_S_STLALL, 1);
    vtc->Add(VOX_THEME_SND_RESHROUD, VOX_S_DRK1, 1);
    vtc->Add(VOX_THEME_SND_UNSHROUD, VOX_S_MAPUP1, 1);
    vtc->Add(VOX_THEME_SND_RADAR, VOX_S_RADAR1, 1);
    vtc->Add(VOX_THEME_SND_ARMAGEDDON, VOX_S_ARMGD1, 1);
    vtc->Add(VOX_THEME_SND_YOULOSE, VOX_S_LOSER1, 2);
    vtc->Add(VOX_THEME_SND_TEST, VOX_S_HELLO1, 1);
    vtc->Add(VOX_THEME_SND_UNIT1, VOX_S_UNIT1, 1);
    VoiceThemes.Add(vtc);

    // TODO: Handle parsing voices.ini for custom themes, needs wave support or only support aud files via ffmpeg conversion?
}
