// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/UserSettingsConfig.h"
#include "ToroDevSettings.h"

USoundClass* FUserSettingsConfig::GetAudioChannel(const FGameplayTag& InChannel) const
{
	const TSoftObjectPtr<USoundClass>* SC = AudioChannels.Find(InChannel);
	return (SC && !SC->IsNull()) ? SC->LoadSynchronous() : nullptr;
}

const FUserSettingsConfig* FUserSettingsConfig::Get()
{
	const UToroDevSettings* Settings = GetDefault<UToroDevSettings>();
	return IsValid(Settings) ? &Settings->UserSettings : nullptr;
}
