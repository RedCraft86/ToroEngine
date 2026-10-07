// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "CoreTypes.h"
#include "MiscEnums.generated.h"

/**
 * Playback direction shared by widget animations and level sequences.
 * Direction does not determine whether playback resumes or restarts; callers select that separately.
 * Numeric values match the corresponding EUMGSequencePlayMode values for native conversion.
 */
UENUM(BlueprintType)
enum class EToroAnimationPlayMode : uint8
{
	/** Plays toward the end of the animation or sequence. */
	Forward = 0,

	/** Plays toward the beginning of the animation or sequence. */
	Reverse = 1
};
