// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Containers/Ticker.h"
#include "ToroAsyncActionBase.h"
#include "TrackAsyncLoadAction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTrackAsyncLoadDelegate, FIntPoint, Packages, bool, bIsLoading);
DECLARE_MULTICAST_DELEGATE_TwoParams(FTrackAsyncLoadDelegateNative, FIntPoint, bool);

/**
 * Observes global async-package loading and completes after a continuous observed idle window.
 * Activity is sampled every core-ticker frame, independently of the update notification interval.
 * An initially idle loader completes after the same idle window. Activity entirely between
 * samples can be missed, and completion does not guarantee that new loads will not arrive.
 * Tracking is global, not restricted to the supplied world or a particular load request,
 * and does not indicate rendering readiness. Create, activate, and cancel on the game thread.
 * Repeated activation while active or after completion/cancellation has no effect.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType, meta = (ExposedAsyncProxy = AsyncAction))
class UTrackAsyncLoadAction final : public UToroAsyncActionBase
{
	GENERATED_BODY()

public:

	/**
	 * Reports the current async-package count and loader activity, without an estimated total.
	 * The engine uses different counters for these values; a zero count alone does not
	 * establish idle. The first sample and final completion bypass the update interval.
	 * Native listeners run first. Cancellation from a native update suppresses the
	 * subsequent Blueprint update, except once completion has already been committed.
	 */
	UPROPERTY(BlueprintAssignable, Category = AsyncLoading)
	FTrackAsyncLoadDelegate OnUpdate;

	/** Native activity notification with the same payload as OnUpdate; broadcast before it. */
	FTrackAsyncLoadDelegateNative OnUpdateNative;

	/**
	 * Reports completion once after the idle window, following the final OnUpdate.
	 * Carries the package count and inactive loader state sampled at completion.
	 * Completion is committed before any final callbacks; cancellation during them has no effect.
	 */
	UPROPERTY(BlueprintAssignable, Category = AsyncLoading)
	FTrackAsyncLoadDelegate OnCompleted;

	/** Native completion notification with the same payload as OnCompleted; broadcast before it. */
	FTrackAsyncLoadDelegateNative OnCompletedNative;

	/**
	 * Creates a global loader observer and registers it with the resolved world's game
	 * instance when available. Blueprint nodes bind and activate automatically;
	 * native callers bind callbacks and call Activate().
	 * @param TestInterval Minimum seconds between ordinary updates, clamped to at least 0.01.
	 *        Nonfinite values use 0.1 seconds. Loader activity is still sampled every frame.
	 * @param IdleDuration Required observed idle time in seconds, clamped to at least zero.
	 *        Nonfinite values use 0.2 seconds. Zero completes on the first idle sample.
	 *        Idle timing begins at the first idle sample and resets whenever activity is observed.
	 */
	UFUNCTION(BlueprintCallable, Category = AsyncLoading, DisplayName = "Track Async Loading", meta = (BlueprintInternalUseOnly = true, WorldContext = ContextObject))
	[[nodiscard]] static TOROCORE_API UTrackAsyncLoadAction* TrackAsyncLoading(const UObject* ContextObject, float TestInterval = 0.1f, float IdleDuration = 0.2f);

	/**
	 * Stops observation silently and releases game-instance registration, including before
	 * activation. Does not cancel engine loads. Repeated calls and calls after completion do nothing.
	 */
	UFUNCTION(BlueprintCallable, Category = AsyncLoading)
	TOROCORE_API void Cancel();

	TOROCORE_API virtual void Activate() override;

private:

	float Interval = 0.1f;
	float IdleWindow = 0.3f;

	float IdleTime = 0.0;
	float TickTime = -1.0;
	bool bFinished = false;
	int32 TotalPackages = 0;
	int32 RemainPackages = 0;
	FTSTicker::FDelegateHandle TickHandle;

	void StopTracking();
	bool OnTick(float DeltaTime);
	virtual void BeginDestroy() override;
};
