// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "NativeGameplayTags.h"
#include "GameplayTagContainer.h"

namespace GameFlagTags
{
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Base);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Transient);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Persistent);

	[[nodiscard]] inline bool IsValid(const FGameplayTag& InTag)
	{
		return InTag.IsValid()
			&& InTag.MatchesTag(Base.GetTag())
			&& InTag != Persistent.GetTag()
			&& InTag != Transient.GetTag()
			&& InTag != Base.GetTag();
	}

	[[nodiscard]] inline bool IsTransient(const FGameplayTag& InTag)
	{
		return InTag.IsValid()
			&& InTag != Transient.GetTag()
			&& InTag.MatchesTag(Transient.GetTag());
	}

	[[nodiscard]] inline bool IsPersistent(const FGameplayTag& InTag)
	{
		return InTag.IsValid()
			&& InTag != Persistent.GetTag()
			&& InTag.MatchesTag(Persistent.GetTag());
	}
}