// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DataTypes/ToroLevelInfo.h"
#include "Engine/DeveloperSettings.h"
#include "UserInterface/UserInterfaceConfig.h"
#include "ToroDevSettings.generated.h"

class UWorld;

UCLASS(Config = Game, DefaultConfig, NotBlueprintable, BlueprintType, DisplayName = "ToroEngine (Runtime)")
class TOROSYSTEMS_API UToroDevSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UToroDevSettings();

	UFUNCTION(BlueprintPure, Category = ToroEngine,
		DisplayName = "Get ToroEngine Settings", meta = (CompactNodeTitle = "ToroEngine"))
	static const UToroDevSettings* Get()
	{
		return GetDefault<UToroDevSettings>();
	}

	UPROPERTY(Config, EditAnywhere, Category = Framework)
	FUserInterfaceConfig UserInterface;

	UPROPERTY(Config, EditAnywhere, Category = Framework)
	TMap<TSoftObjectPtr<UWorld>, FToroLevelInfo> Levels;
};
