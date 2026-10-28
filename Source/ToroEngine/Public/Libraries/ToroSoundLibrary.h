// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

// ReSharper disable UnrealHeaderToolError
#pragma once

#include "UE5Coro.h"
#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "Engine/LatentActionManager.h"
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
	 * Invalid actors or audio components produce no request. Nonfinite offsets report an error through Toro logging and produce no request.
	 * @param Target Ambient sound actor to play.
	 * @param StartTime Finite offset in seconds; negative values are silently clamped to zero. Seeking support depends on the sound.
	 */
	UFUNCTION(BlueprintCallable, Category = Audio, meta = (DefaultToSelf = Target))
	static void PlayAmbientSound(const AAmbientSound* Target, float StartTime = 0.0f);

	/**
	 * Requests an immediate or delayed stop without waiting for audio completion.
	 * Invalid actors or audio components are ignored. Nonfinite delays report an error through Toro logging without stopping audio.
	 * @param Target Ambient sound actor to stop.
	 * @param Delay Finite seconds before stopping; negative values are silently clamped to zero.
	 *        Values at or below UE_KINDA_SMALL_NUMBER stop immediately.
	 */
	UFUNCTION(BlueprintCallable, Category = Audio, meta = (DefaultToSelf = Target))
	static void StopAmbientSound(const AAmbientSound* Target, float Delay = 0.0f);

	/**
	 * Requests playback with a volume fade-in, then waits for the clamped duration in game time.
	 * The wait does not track interruption, audio-thread completion, or whether playback actually started.
	 * Invalid actors or audio components cause no request or wait. Nonfinite duration, start offset, or target level
	 * reports an error through Toro logging and causes no request or wait.
	 * @param Target Ambient sound actor to play and fade in.
	 * @param Duration Finite seconds to fade and wait; negative values are silently clamped to zero, which adds no coroutine wait.
	 * @param TargetLevel Finite multiplier of the component's calculated volume, passed to native fade handling.
	 * @param StartTime Finite playback offset in seconds; negative values are silently clamped to zero. Seeking support depends on the sound.
	 * @param FadeCurve Native interpolation curve for the volume fade.
	 */
	UFUNCTION(BlueprintCallable, Category = Audio, meta = (Latent, LatentInfo = LatentInfo, DefaultToSelf = Target))
	static FVoidCoroutine FadeInAmbientSound(FLatentActionInfo LatentInfo, const AAmbientSound* Target,
		float Duration = 1.0f, float TargetLevel = 1.0f, float StartTime = 0.0f, EAudioFaderCurve FadeCurve = EAudioFaderCurve::Linear);

	/**
	 * Requests a native volume fade-out and delayed stop, then waits for the clamped duration in game time.
	 * Native fade-out schedules stopping even with a nonzero target level, but may ignore requests on inactive audio,
	 * without an audio device, or when the requested level is at least the current fader target; concurrency fades can also limit requests.
	 * The wait does not track interruption, audio-thread completion, or whether the request was accepted.
	 * Invalid actors or audio components cause no request or wait. Nonfinite duration or target level
	 * reports an error through Toro logging and causes no request or wait.
	 * @param Target Ambient sound actor to fade out and stop.
	 * @param Duration Finite seconds to fade and wait; negative values are silently clamped to zero, which adds no coroutine wait.
	 * @param TargetLevel Finite multiplier of the component's calculated volume, passed to native fade handling.
	 * @param FadeCurve Native interpolation curve for the volume fade.
	 */
	UFUNCTION(BlueprintCallable, Category = Audio, meta = (Latent, LatentInfo = LatentInfo, DefaultToSelf = Target))
	static FVoidCoroutine FadeOutAmbientSound(FLatentActionInfo LatentInfo, const AAmbientSound* Target,
		float Duration = 1.0f, float TargetLevel = 0.0f, EAudioFaderCurve FadeCurve = EAudioFaderCurve::Linear);
};
