// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "UE5Coro.h"
#include "DataTypes/MiscEnums.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroSequenceLibrary.generated.h"

class ALevelSequenceActor;

/**
 * Completion-state policy applied when stopping a level sequence.
 */
UENUM(BlueprintType)
enum class EToroSequenceStopType : uint8
{
	/** Use the sequence's configured completion behavior. */
	Default,

	/** Restore captured state when stopped. */
	Reset,

	/** Keep state at the current playback time. */
	CurrentTime,

	/** Move to the end and keep the resulting state. */
	SkipToEnd
};

/**
 * Level-sequence playback and stopping helpers; invoke on the game thread.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroSequenceLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Stops a valid sequence player using the selected completion behavior.
	 * @param Target Sequence actor; invalid actors or missing players are ignored.
	 * @param StopType Select default completion, restored state, current-time state, or end-time state.
	 */
	UFUNCTION(BlueprintCallable, Category = LevelSequence, DisplayName = "Stop Sequence", meta = (DefaultToSelf = Target))
	static void StopLevelSequence(const ALevelSequenceActor* Target, EToroSequenceStopType StopType = EToroSequenceStopType::Default);

	/**
	 * Submits forward or reverse playback on the game thread using a validated sequence player.
	 * @param Target Sequence actor with a valid player and assigned sequence.
	 * @param PlayMode Forward or Reverse; unnamed enum values are rejected.
	 * @param PlayRate Finite speed magnitude, clamped to at least 0.1; PlayMode controls direction.
	 * @param bResumePlay Continue from the current position when true. When false, stop and restart.
	 * @return True when a valid playback request is submitted, not confirmation that the engine
	 *         started playback; false for invalid inputs or a player invalidated during restart preparation.
	 */
	UFUNCTION(BlueprintCallable, Category = LevelSequence, DisplayName = "Play Sequence", meta = (DefaultToSelf = Target))
	static bool PlayLevelSequence(const ALevelSequenceActor* Target, EToroAnimationPlayMode PlayMode = EToroAnimationPlayMode::Forward,
		float PlayRate = 1.0f, bool bResumePlay = true);

	/**
	 * Submits playback on the game thread and waits for natural completion or an explicit stop.
	 * Restart preparation occurs before completion listeners are bound. Pausing, infinite looping,
	 * or an engine refusal to start can leave the wait pending until a finish/stop notification or
	 * latent cancellation. Cancellation removes the listeners without explicitly stopping playback.
	 * @param bSuccess False for rejected requests; true after natural completion or explicit stopping.
	 * @param Target Sequence actor with a valid player and assigned sequence.
	 * @param PlayMode Forward or Reverse; unnamed enum values are rejected.
	 * @param PlayRate Finite speed magnitude, clamped to at least 0.1; PlayMode controls direction.
	 * @param bResumePlay Continue from the current position when true. When false, stop and restart.
	 */
	UFUNCTION(BlueprintCallable, Category = LevelSequence, DisplayName = "Play Sequence (Async)",
		meta = (Latent, LatentInfo = LatentInfo, DefaultToSelf = Target))
	static FVoidCoroutine PlayLevelSequenceAsync(FLatentActionInfo LatentInfo, bool& bSuccess, const ALevelSequenceActor* Target,
		EToroAnimationPlayMode PlayMode = EToroAnimationPlayMode::Forward, float PlayRate = 1.0f, bool bResumePlay = true);
};
