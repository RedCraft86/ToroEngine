// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MiscEnums.generated.h"

/**
 * Playback direction for widget animations and level sequences.
 */
UENUM(BlueprintType)
enum class EToroAnimationPlayMode : uint8
{
	/** Play toward the end of the animation or sequence. */
	Forward,

	/** Play toward the start of the animation or sequence. */
	Reverse
};

/**
 * Controls the playback position and completion behavior when stopping a level sequence.
 */
UENUM(BlueprintType)
enum class EToroSequenceStopType : uint8
{
	/** Stop using the sequence's configured completion behavior. */
	Default,

	/** Stop and force restoration of the state captured before playback. */
	Reset,

	/** Stop at the current playback position and keep the evaluated state. */
	CurrentTime,

	/** Jump to the end, stop playback, and keep the evaluated end state. */
	SkipToEnd
};

/**
 * Requested level loading and visibility state; asynchronous transitions may still be in progress.
 */
UENUM(BlueprintType)
enum class EToroLevelStreamState : uint8
{
	/** The level should be unloaded. */
	Unloaded,

	/** The level should be loaded without being visible. */
	Loaded,

	/** The level should be loaded and visible. */
	Visible
};
