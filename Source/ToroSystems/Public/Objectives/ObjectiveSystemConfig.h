// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "NativeGameplayTags.h"
#include "GameplayTagContainer.h"
#include "ObjectiveSystemConfig.generated.h"

class UObjectiveGraph;

namespace ObjectiveTags
{
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Base);

	[[nodiscard]] inline bool IsValid(const FGameplayTag& InTag)
	{
		return InTag.IsValid()
			&& InTag != Base.GetTag()
			&& InTag.MatchesTag(Base.GetTag());
	}
}

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FObjectiveSystemConfig final
{
	GENERATED_BODY()

	FObjectiveSystemConfig();

	UPROPERTY(EditAnywhere, Category = Settings, meta = (Categories = "LevelType"))
	FGameplayTag AllowedOnMap;

	UPROPERTY(EditAnywhere, Category = Settings, NoClear, meta = (AllowAbstract = false))
	TSoftClassPtr<UObjectiveGraph> Objective;

	[[nodiscard]] static const FObjectiveSystemConfig* Get();
};
