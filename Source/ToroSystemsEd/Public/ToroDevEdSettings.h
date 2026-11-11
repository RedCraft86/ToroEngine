// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "DataTypes/ToroPropertyButton.h"
#include "ToroDevEdSettings.generated.h"

UCLASS(Config = Editor, DefaultConfig, NotBlueprintable, NotBlueprintType, DisplayName = "ToroEngine (Editor)")
class TOROSYSTEMSED_API UToroDevEdSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UToroDevEdSettings();

	static const UToroDevEdSettings* Get()
	{
		return GetDefault<UToroDevEdSettings>();
	}

	/**
	 * Applies ToroEngine's rendering, AI, Audio, and framework preset to this project's DefaultEngine.ini.
	 * Restart Unreal Editor afterward. Nothing is applied automatically; later project edits are allowed.
	 */
	UPROPERTY(EditAnywhere, Category = Framework, meta = (WholeRowButton, Label = "Apply Default Configs"))
	FToroPropertyButton ApplyDefaultConfigsButton;

private:

	static void ApplyDefaultConfigs();
};
