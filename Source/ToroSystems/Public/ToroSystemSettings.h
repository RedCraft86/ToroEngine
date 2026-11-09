// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "ToroSystemSettings.generated.h"

class UToroDialogWidgetBase;

UCLASS(Config = Game, DefaultConfig, NotBlueprintable, BlueprintType, DisplayName = "ToroSystems")
class TOROSYSTEMS_API UToroSystemSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UToroSystemSettings();

	UFUNCTION(BlueprintPure, Category = ToroEngine,
		DisplayName = "Get ToroSystem Settings", meta = (CompactNodeTitle = "ToroSystems"))
	static const UToroSystemSettings* Get()
	{
		return GetDefault<UToroSystemSettings>();
	}
};
