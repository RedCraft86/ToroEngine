// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UObject/Object.h"
#include "UserSettingsExtension.generated.h"

class UToroGameUserSettings;
struct FUserSettingsRegistry;

UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API UUserSettingsExtension : public UObject
{
	GENERATED_BODY()

public:

	virtual void RegisterSettings(FUserSettingsRegistry& Registry, UToroGameUserSettings* Settings) const
		PURE_VIRTUAL(UUserSettingsExtension::RegisterSettings,)
};
