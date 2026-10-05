// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

// ReSharper disable UnrealHeaderToolError
#pragma once

#include "UE5Coro.h"
#include "Components/AudioComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroSoundLibrary.generated.h"

class AAmbientSound;

/**
 * Ambient-sound playback and time-based fade helpers; invoke on the game thread.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroSoundLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Starts playback on an ambient sound's audio component; invalid actors or missing components are ignored.
	 * @param Target Ambient sound actor.
	 * @param StartTime Playback offset in seconds, forwarded to the audio component.
	 */
	UFUNCTION(BlueprintCallable, Category = Audio, DisplayName = "Play Sound", meta = (DefaultToSelf = Target))
	static void PlayAmbientSound(const AAmbientSound* Target, float StartTime = 0.0f);

	/**
	 * Stops ambient playback immediately or schedules a delayed stop.
	 * @param Target Ambient sound actor; invalid actors or missing components are ignored.
	 * @param Delay Finite delay in seconds; its absolute value is used, and values at most UE_KINDA_SMALL_NUMBER stop immediately.
	 */
	UFUNCTION(BlueprintCallable, Category = Audio, DisplayName = "Stop Sound", meta = (DefaultToSelf = Target))
	static void StopAmbientSound(const AAmbientSound* Target, float Delay = 0.0f);

	/**
	 * Starts ambient playback with a volume fade and waits for its configured duration.
	 * The wait measures elapsed world time, not audio completion or interruption.
	 * @param Target Ambient sound actor; invalid actors or missing components are ignored.
	 * @param Duration Finite fade duration in seconds, clamped to zero; nonfinite inputs are ignored.
	 * @param TargetLevel Target volume multiplier, forwarded to the audio component.
	 * @param StartTime Playback offset in seconds.
	 * @param FadeCurve Audio volume interpolation curve.
	 */
	UFUNCTION(BlueprintCallable, Category = Audio, DisplayName = "Fade In Sound", meta = (Latent, LatentInfo = LatentInfo, DefaultToSelf = Target))
	static FVoidCoroutine FadeInAmbientSound(FLatentActionInfo LatentInfo, const AAmbientSound* Target, float Duration = 1.0f,
		float TargetLevel = 1.0f, float StartTime = 0.0f, EAudioFaderCurve FadeCurve = EAudioFaderCurve::Linear);

	/**
	 * Fades ambient playback out, schedules stopping, and waits for the configured duration.
	 * The wait measures elapsed world time, not audio completion or interruption.
	 * @param Target Ambient sound actor; invalid actors or missing components are ignored.
	 * @param Duration Finite fade duration in seconds, clamped to zero; nonfinite inputs are ignored.
	 * @param TargetLevel Target volume multiplier, forwarded to the audio component.
	 * @param FadeCurve Audio volume interpolation curve.
	 */
	UFUNCTION(BlueprintCallable, Category = Audio, DisplayName = "Fade Out Sound", meta = (Latent, LatentInfo = LatentInfo, DefaultToSelf = Target))
	static FVoidCoroutine FadeOutAmbientSound(FLatentActionInfo LatentInfo, const AAmbientSound* Target, float Duration = 1.0f,
		float TargetLevel = 0.0f, EAudioFaderCurve FadeCurve = EAudioFaderCurve::Linear);
};
