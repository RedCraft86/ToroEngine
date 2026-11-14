// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameplayTagContainer.h"
#include "UserSettingsConfig.generated.h"

class USoundControlBus;
class USoundControlBusMix;
class UUserSettingsBuilder;

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FUserSettingsConfig final
{
	GENERATED_BODY()

	FUserSettingsConfig();

	UPROPERTY(EditAnywhere, Category = Settings, NoClear, meta = (AllowAbstract = false))
	TSoftClassPtr<UUserSettingsBuilder> SettingsBuilder;

	UPROPERTY(EditAnywhere, Category = Audio)
	TSoftObjectPtr<USoundControlBusMix> AudioMix;

	UPROPERTY(EditAnywhere, Category = Audio, meta = (Categories = "UserSettings.Audio", ShowOnlyLeafTag))
	TMap<FGameplayTag, TSoftObjectPtr<USoundControlBus>> AudioChannels;

	[[nodiscard]] USoundControlBus* GetAudioChannel(const FGameplayTag& InChannel) const;

	[[nodiscard]] static const FUserSettingsConfig* Get();
};
