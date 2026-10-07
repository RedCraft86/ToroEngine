// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "UE5Coro.h"
#include "DataTypes/MiscEnums.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroWidgetLibrary.generated.h"

class UWidgetAnimation;

/**
 * Widget-animation state initialization and playback helpers; invoke on the game thread.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroWidgetLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Creates or retrieves states for valid animations in the widget's Blueprint class hierarchy.
	 * Does not start playback. Invalid animation entries are skipped.
	 * @param Target Widget to initialize; invalid targets are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = Widget, meta = (DefaultToSelf = Target))
	static void InitWidgetAnimations(UUserWidget* Target);

	/**
	 * Requests widget-animation playback and checks the returned state's playback status.
	 * On the explicit restart path, the asset's start time (forward) or end time (reverse)
	 * is supplied as StartAtTime. The engine interprets reverse StartAtTime as an offset
	 * from the end, so this does not guarantee restart at the reverse playback boundary.
	 * @param Target Widget to animate; must be valid.
	 * @param Animation Valid animation to play.
	 * @param PlayMode Intended direction, Forward or Reverse.
	 * @param PlayRate Finite speed, clamped to at least 0.1 when starting playback.
	 * @param bResumePlay Continue from the current position when true. When false, stop and restart.
	 * @return True when the returned handle is valid and its state reports Playing;
	 *         false for invalid inputs, nonfinite rates, or a handle/state that does not report Playing.
	 */
	UFUNCTION(BlueprintCallable, Category = Widget, meta = (DefaultToSelf = Target))
	static bool PlayWidgetAnimation(UUserWidget* Target, UWidgetAnimation* Animation,
		EToroAnimationPlayMode PlayMode = EToroAnimationPlayMode::Forward, float PlayRate = 1.0f, bool bResumePlay = true);

	/**
	 * Binds an animation-finish listener before invoking PlayWidgetAnimation and waits if playback succeeds.
	 * Stopping can also emit the finish notification. Without a notification, the wait remains pending
	 * until latent cancellation. Cancellation ends the wait without explicitly stopping playback.
	 * @param bSuccess False initially and for rejected playback. After a finish notification, true only
	 *        if the observed state is valid and its current time approximately matches the asset's end time
	 *        for Forward or start time for Reverse; a finish notification alone does not imply success.
	 * @param Target Widget to animate; must be valid.
	 * @param Animation Valid animation whose state is observed.
	 * @param PlayMode Intended direction, Forward or Reverse; also selects the endpoint checked on completion.
	 * @param PlayRate Finite speed, clamped to at least 0.1 when starting playback; already-playing
	 *        animations on the convenience path retain their existing speed.
	 * @param bResumePlay Continue from the current position when true. When false, stop and restart.
	 */
	UFUNCTION(BlueprintCallable, Category = Widget, meta = (Latent, LatentInfo = LatentInfo, DefaultToSelf = Target))
	static FVoidCoroutine PlayWidgetAnimationAsync(FLatentActionInfo LatentInfo, bool& bSuccess, UUserWidget* Target, UWidgetAnimation* Animation,
		EToroAnimationPlayMode PlayMode = EToroAnimationPlayMode::Forward, float PlayRate = 1.0f, bool bResumePlay = true);
};
