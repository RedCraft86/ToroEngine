// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"

enum class EUserSettingImpact : uint8
{
	None	= 0,
	Low		= 1,
	Medium	= 2,
	High	= 3
};

struct TOROSYSTEMS_API FUserSettingMetadata
{
	FText DisplayName = FText::GetEmpty();
	FText Description = FText::GetEmpty();
	EUserSettingImpact Impact = EUserSettingImpact::None;
	bool bIsCategory = false;
};
