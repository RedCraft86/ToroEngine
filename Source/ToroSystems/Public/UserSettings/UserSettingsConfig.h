// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameplayTagContainer.h"
#include "UserSettingsConfig.generated.h"

class USoundClass;
class UUserSettingsExtension;

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FUserSettingsConfig final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	TSoftClassPtr<UUserSettingsExtension> SettingsExtension;

	UPROPERTY(EditAnywhere, Category = Audio, meta = (Categories = "UserSettings.Audio", ShowOnlyLeafTag))
	TMap<FGameplayTag, TSoftObjectPtr<USoundClass>> AudioChannels;

	[[nodiscard]] USoundClass* GetAudioChannel(const FGameplayTag& InChannel) const;

	[[nodiscard]] static const FUserSettingsConfig* Get();
};
