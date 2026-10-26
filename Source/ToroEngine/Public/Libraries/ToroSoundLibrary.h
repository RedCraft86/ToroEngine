// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

// ReSharper disable UnrealHeaderToolError
#pragma once

#include "UE5Coro.h"
#include "Components/AudioComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroSoundLibrary.generated.h"

class AAmbientSound;

/**
 * Blueprint helpers for ambient-sound playback, delayed stopping, and fades with elapsed-game-time waits.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroSoundLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Requests playback of the ambient actor's audio component at the supplied sound offset.
	 * Invalid actors or audio components and nonfinite offsets produce no request.
	 * @param Target Ambient sound actor to play.
	 * @param StartTime Finite offset in seconds; seeking support depends on the sound. Finite negative offsets use native handling.
	 */
	UFUNCTION(BlueprintCallable, Category = Audio, meta = (DefaultToSelf = Target))
	static void PlayAmbientSound(const AAmbientSound* Target, float StartTime = 0.0f);

	/**
	 * Requests an immediate or delayed stop without waiting for audio completion.
	 * Invalid actors or audio components are ignored.
	 * @param Target Ambient sound actor to stop.
	 * @param Delay Seconds before stopping, using the absolute value for finite inputs.
	 * Values at or below UE_KINDA_SMALL_NUMBER after taking the absolute value, and nonfinite inputs, stop immediately.
	 */
	UFUNCTION(BlueprintCallable, Category = Audio, meta = (DefaultToSelf = Target))
	static void StopAmbientSound(const AAmbientSound* Target, float Delay = 0.0f);

	/**
	 * Requests playback with a volume fade-in, then waits for the sanitized duration in game time.
	 * The wait does not track interruption, audio-thread completion, or whether playback actually started.
	 * Invalid actors or audio components and nonfinite start offsets or target levels cause no request or wait.
	 * @param Target Ambient sound actor to play and fade in.
	 * @param Duration Seconds to fade and wait; finite negative values and nonfinite values become zero, adding no coroutine wait.
	 * @param TargetLevel Finite multiplier of the component's calculated volume, passed to native fade handling.
	 * @param StartTime Finite playback offset in seconds; seeking support and finite negative offsets use native handling.
	 * @param FadeCurve Native interpolation curve for the volume fade.
	 */
	UFUNCTION(BlueprintCallable, Category = Audio, meta = (Latent, LatentInfo = LatentInfo, DefaultToSelf = Target))
	static FVoidCoroutine FadeInAmbientSound(FLatentActionInfo LatentInfo, const AAmbientSound* Target,
		float Duration = 1.0f, float TargetLevel = 1.0f, float StartTime = 0.0f, EAudioFaderCurve FadeCurve = EAudioFaderCurve::Linear);

	/**
	 * Requests a native volume fade-out and delayed stop, then waits for the sanitized duration in game time.
	 * Native fade-out schedules stopping even with a nonzero target level, but may ignore requests on inactive audio,
	 * without an audio device, or when the requested level is at least the current fader target; concurrency fades can also limit requests.
	 * The wait does not track interruption, audio-thread completion, or whether the request was accepted.
	 * Invalid actors or audio components and nonfinite target levels cause no request or wait.
	 * @param Target Ambient sound actor to fade out and stop.
	 * @param Duration Seconds to fade and wait; finite negative values and nonfinite values become zero, adding no coroutine wait.
	 * @param TargetLevel Finite multiplier of the component's calculated volume, passed to native fade handling.
	 * @param FadeCurve Native interpolation curve for the volume fade.
	 */
	UFUNCTION(BlueprintCallable, Category = Audio, meta = (Latent, LatentInfo = LatentInfo, DefaultToSelf = Target))
	static FVoidCoroutine FadeOutAmbientSound(FLatentActionInfo LatentInfo, const AAmbientSound* Target,
		float Duration = 1.0f, float TargetLevel = 0.0f, EAudioFaderCurve FadeCurve = EAudioFaderCurve::Linear);
};
