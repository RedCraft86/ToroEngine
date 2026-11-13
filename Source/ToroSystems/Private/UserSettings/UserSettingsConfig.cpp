// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/UserSettingsConfig.h"
#include "UserSettings/UserSettingsTypes.h"
#include "SoundControlBusMix.h"
#include "SoundControlBus.h"
#include "ToroDevSettings.h"

FUserSettingsConfig::FUserSettingsConfig()
{
	AudioMix = FSoftObjectPath(TEXT("/ToroEngine/Audio/CBM_ToroEngine.CBM_ToroEngine"));
	AudioChannels = {
		{
			UserSettingsTags::Audio_MasterVolume.GetTag(),
			TSoftObjectPtr<USoundControlBus>(FSoftObjectPath(
				TEXT("/ToroEngine/Audio/Control/CB_Volume_Master.CB_Volume_Master")))
		},
		{
			UserSettingsTags::Audio_MusicVolume.GetTag(),
			TSoftObjectPtr<USoundControlBus>(FSoftObjectPath(
				TEXT("/ToroEngine/Audio/Control/CB_Volume_Music.CB_Volume_Music")))
		},
		{
			UserSettingsTags::Audio_EffectsVolume.GetTag(),
			TSoftObjectPtr<USoundControlBus>(FSoftObjectPath(
				TEXT("/ToroEngine/Audio/Control/CB_Volume_Effects.CB_Volume_Effects")))
		},
		{
			UserSettingsTags::Audio_DialogueVolume.GetTag(),
			TSoftObjectPtr<USoundControlBus>(FSoftObjectPath(
				TEXT("/ToroEngine/Audio/Control/CB_Volume_Dialogue.CB_Volume_Dialogue")))
		},
		{
			UserSettingsTags::Audio_UIVolume.GetTag(),
			TSoftObjectPtr<USoundControlBus>(FSoftObjectPath(
				TEXT("/ToroEngine/Audio/Control/CB_Volume_UserInterface.CB_Volume_UserInterface")))
		}
	};
}

USoundControlBus* FUserSettingsConfig::GetAudioChannel(const FGameplayTag& InChannel) const
{
	const TSoftObjectPtr<USoundControlBus>* SC = AudioChannels.Find(InChannel);
	return (SC && !SC->IsNull()) ? SC->LoadSynchronous() : nullptr;
}

const FUserSettingsConfig* FUserSettingsConfig::Get()
{
	const UToroDevSettings* Settings = GetDefault<UToroDevSettings>();
	return IsValid(Settings) ? &Settings->UserSettings : nullptr;
}
