// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "NativeGameplayTags.h"
#include "GameplayTagContainer.h"
#include "CutsceneTypes.generated.h"

namespace CutsceneTags
{
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Base)

	[[nodiscard]] inline bool IsValid(const FGameplayTag& InTag)
	{
		return InTag.IsValid()
			&& InTag != Base.GetTag()
			&& InTag.MatchesTag(Base.GetTag());
	}
}

UENUM(BlueprintType)
enum class ECutscenePlayResult : uint8
{
	Success,
	InvalidContext,
	InvalidCutscene,
	CutscenePlaying
};