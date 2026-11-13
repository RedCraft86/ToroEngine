// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameplayTagContainer.h"
#include "UserSettingsConfig.generated.h"

class USoundClass;

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FUserSettingsConfig final
{
	GENERATED_BODY()

	// TODO: Setup proper type
	UPROPERTY(EditAnywhere, Category = Settings)
	TSoftClassPtr<UObject> SettingsExtension;

	UPROPERTY(EditAnywhere, Category = Audio, meta = (Categories = "UserSettings.Audio", ShowOnlyLeafTag))
	TMap<FGameplayTag, TSoftObjectPtr<USoundClass>> AudioChannels;

	[[nodiscard]] USoundClass* GetAudioChannel(const FGameplayTag& InChannel) const;

	[[nodiscard]] static const FUserSettingsConfig* Get();
};
