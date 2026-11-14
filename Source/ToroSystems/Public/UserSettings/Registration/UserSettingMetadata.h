// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
namespace EUserSettingImpact
{
	enum Type : uint8
	{
		None = 0,
		Low = 1,
		Medium = 2,
		High = 3
	};

	inline FText ToText(Type InImpact)
	{
		switch (InImpact)
		{
		case EUserSettingImpact::None: return LOCTEXT("PerformanceImpactNone", "None");
		case EUserSettingImpact::Low: return LOCTEXT("PerformanceImpactLow", "Low");
		case EUserSettingImpact::Medium: return LOCTEXT("PerformanceImpactMedium", "Medium");
		case EUserSettingImpact::High: return LOCTEXT("PerformanceImpactHigh", "High");
		default: return LOCTEXT("PerformanceImpactUnknown", "Unknown");
		}
	}
}

struct TOROSYSTEMS_API FUserSettingMetadata
{
	FText DisplayName = FText::GetEmpty();
	FText Description = FText::GetEmpty();
	EUserSettingImpact::Type Impact = EUserSettingImpact::None;
	bool bIsCategory = false;
};
#undef LOCTEXT_NAMESPACE
