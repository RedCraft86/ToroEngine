// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "UE5Coro.h"
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
	static void StopLevelSequence(const ALevelSequenceActor* Target, const EToroSequenceStopType StopType = EToroSequenceStopType::Default);

	/**
	 * Starts forward playback and optionally waits for natural completion or an explicit stop.
	 * Looping playback can wait indefinitely until stopped; stopped playback still reports success.
	 * @param bSuccess False for invalid targets, missing players, or nonfinite rates; true after starting or the optional wait.
	 * @param Target Sequence actor to play.
	 * @param PlayRate Finite playback rate, clamped to at least 0.1.
	 * @param bWaitForFinished Await OnFinished or OnStop rather than return after starting.
	 */
	UFUNCTION(BlueprintCallable, Category = LevelSequence, DisplayName = "Play Sequence",
		meta = (Latent, LatentInfo = LatentInfo, DefaultToSelf = Target))
	static FVoidCoroutine PlayLevelSequence(FLatentActionInfo LatentInfo, bool& bSuccess,
		const ALevelSequenceActor* Target, const float PlayRate = 1.0f, const bool bWaitForFinished = true);

	/**
	 * Starts reverse playback and optionally waits for natural completion or an explicit stop.
	 * Looping playback can wait indefinitely until stopped; stopped playback still reports success.
	 * @param bSuccess False for invalid targets, missing players, or nonfinite rates; true after starting or the optional wait.
	 * @param Target Sequence actor to play in reverse.
	 * @param PlayRate Finite positive rate magnitude, clamped to at least 0.1.
	 * @param bWaitForFinished Await OnFinished or OnStop rather than return after starting.
	 */
	UFUNCTION(BlueprintCallable, Category = LevelSequence, DisplayName = "Reverse Sequence",
		meta = (Latent, LatentInfo = LatentInfo, DefaultToSelf = Target))
	static FVoidCoroutine ReverseLevelSequence(FLatentActionInfo LatentInfo, bool& bSuccess,
		const ALevelSequenceActor* Target, const float PlayRate = 1.0f, const bool bWaitForFinished = true);
};
