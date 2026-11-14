// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UObject/Object.h"
#include "UserSettingsBuilder.generated.h"

class UToroGameUserSettings;
class FUserSettingsRegistry;

UCLASS(NotBlueprintable, BlueprintType)
class TOROSYSTEMS_API UUserSettingsBuilder : public UObject
{
	GENERATED_BODY()

public:

	static void Build(FUserSettingsRegistry& Registry, UToroGameUserSettings& Settings);

protected:

	virtual void BuildRegistry(FUserSettingsRegistry& Registry, UToroGameUserSettings& Settings) const;
};
