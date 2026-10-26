// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UE5Coro.h"
#include "DataTypes/MiscEnums.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroSequenceLibrary.generated.h"

class ALevelSequenceActor;

/**
 * Blueprint helpers for level-sequence playback, stop completion behavior, and endpoint-based latent completion.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroSequenceLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Requests a stop with the selected completion behavior, then restores the player's previous completion override if still valid.
	 * Default uses configured section completion rules; Reset forces restoration of captured pre-playback state.
	 * Both use the native Stop() cursor reset, which depends on the player's current playback direction.
	 * CurrentTime keeps the evaluated state and cursor position; SkipToEnd jumps to the final valid time and keeps that state.
	 * @param Target Sequence actor to stop; invalid actors or players are ignored.
	 * @param StopType Stop behavior to request; unrecognized values issue no stop request.
	 */
	UFUNCTION(BlueprintCallable, Category = LevelSequence, meta = (DefaultToSelf = Target))
	static void StopLevelSequence(const ALevelSequenceActor* Target, EToroSequenceStopType StopType = EToroSequenceStopType::Default);

	/**
	 * Requests forward or reverse playback of an assigned level sequence.
	 * @param Target Sequence actor with a valid player and assigned sequence.
	 * @param PlayMode Forward or Reverse; unrecognized modes are rejected before changing playback state.
	 * @param PlayRate Finite playback rate, silently clamped to at least 0.1; direction is selected by PlayMode.
	 *        Nonfinite rates are rejected with an error message.
	 * @param bResumePlay Whether to retain the current playback position. False calls native RewindForReplay(),
	 *        which stops playback using the native cursor-reset rules and resets replay synchronization bookkeeping.
	 * @return True if a play request was issued, not independent confirmation that the engine started playback;
	 *         false for invalid inputs or a player or actor invalidated during rewind.
	 */
	UFUNCTION(BlueprintCallable, Category = LevelSequence, meta = (DefaultToSelf = Target))
	static bool PlayLevelSequence(const ALevelSequenceActor* Target, EToroAnimationPlayMode PlayMode = EToroAnimationPlayMode::Forward,
		float PlayRate = 1.0f, bool bResumePlay = true);

	/**
	 * Requests playback and waits for the player's OnFinished or OnStop event.
	 * Replay rewind occurs before binding those events so its own stop cannot complete this wait.
	 * The wait does not own playback exclusively; pausing does not complete it and looping can extend it indefinitely.
	 * @param bSuccess True after either event if the player remains valid and its current time is within 0.1 seconds
	 *        of the requested direction's endpoint: GetEndTime() for Forward or GetStartTime() for Reverse.
	 *        Explicit stops near that endpoint can succeed; invalid inputs or other ending positions leave this false.
	 * @param Target Sequence actor with a valid player and assigned sequence.
	 * @param PlayMode Forward or Reverse; unrecognized modes are rejected before changing playback state.
	 * @param PlayRate Finite playback rate, silently clamped to at least 0.1; direction is selected by PlayMode.
	 *        Nonfinite rates are rejected with an error message.
	 * @param bResumePlay Whether to retain the current position; false first performs native RewindForReplay(),
	 *        including its stop, cursor-reset rules, and replay synchronization reset.
	 */
	UFUNCTION(BlueprintCallable, Category = LevelSequence, meta = (Latent, LatentInfo = LatentInfo, DefaultToSelf = Target))
	static FVoidCoroutine PlayLevelSequenceAsync(FLatentActionInfo LatentInfo, bool& bSuccess, const ALevelSequenceActor* Target,
		EToroAnimationPlayMode PlayMode = EToroAnimationPlayMode::Forward, float PlayRate = 1.0f, bool bResumePlay = true);
};
