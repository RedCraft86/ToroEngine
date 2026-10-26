// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Containers/Ticker.h"
#include "ToroAsyncActionBase.h"
#include "ToroAsyncPackageLoadMonitor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTrackAsyncLoadDelegate, int32, NumCompleted, int32, NumPending, bool, bIsLoading);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FTrackAsyncLoadDelegateNative, int32, int32, bool);

/**
 * Observes global async package-loading activity on the game thread until a consecutive idle window elapses.
 * Does not initiate or own load requests, and completion does not indicate that all loads succeeded.
 * Package progress is an estimate based on the peak sampled outstanding request count, not an exact
 * total or percentage when requests arrive between samples or while monitoring.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType, meta = (ExposedAsyncProxy = Action))
class UToroAsyncPackageLoadMonitor : public UToroAsyncActionBase
{
	GENERATED_BODY()

public:

	/**
	 * Broadcast when the sampled outstanding package count changes.
	 * NumCompleted is peak minus outstanding requests; NumPending is the sampled peak.
	 * bIsLoading reports the global loading state observed on that tick. It does not imply monitoring has stopped.
	 */
	UPROPERTY(BlueprintAssignable, Category = AsyncLoading)
	FTrackAsyncLoadDelegate OnUpdate;

	/**
	 * Native counterpart of OnUpdate, broadcast before the Blueprint delegate.
	 * Cancelling from a native listener suppresses the subsequent Blueprint update.
	 */
	FTrackAsyncLoadDelegateNative OnUpdateNative;

	/**
	 * Broadcast once after loading is observed idle for the configured duration; never broadcast on cancellation.
	 * NumCompleted and NumPending both contain the sampled peak, or zero if no packages were observed.
	 * bIsLoading is false; this notification does not report individual load success.
	 */
	UPROPERTY(BlueprintAssignable, Category = AsyncLoading)
	FTrackAsyncLoadDelegate OnCompleted;

	/** Native counterpart of OnCompleted, broadcast first after the action has been marked finished. */
	FTrackAsyncLoadDelegateNative OnCompletedNative;

	/**
	 * Observes global async package-loading activity on the game thread until a consecutive idle window elapses.
	 * Does not initiate or own load requests, and completion does not indicate that all loads succeeded.
	 * Package progress is an estimate based on the peak sampled outstanding request count, not an exact
	 * total or percentage when requests arrive between samples or while monitoring.
	 *
	 * Creates a game-thread-only global loading monitor and registers it with the context's game instance when available.
	 * Blueprint async nodes activate it after binding delegates; native callers must call Activate().
	 * Loading state and idle time are observed every core-ticker frame; package counts are sampled at TestInterval.
	 * @param TestInterval Package-count sampling interval in seconds, clamped to at least 0.01; nonfinite values use 0.1.
	 * @param IdleDuration Consecutive observed idle seconds before completion, clamped to at least 0.1; nonfinite values use 0.2.
	 * @return A new inactive action; bind delegates before activating it.
	 */
	UFUNCTION(BlueprintCallable, Category = AsyncLoading, DisplayName = "Monitor Async Package Loading",
		meta = (BlueprintInternalUseOnly = true, WorldContext = ContextObject))
	[[nodiscard]] static TOROENGINE_API UToroAsyncPackageLoadMonitor* MonitorAsyncPackageLoading(
		const UObject* ContextObject, float TestInterval = 0.1f, float IdleDuration = 0.2f);

	/**
	 * Silently stops monitoring and releases game-instance registration on the game thread.
	 * Safe before activation and on repeated calls; a cancelled action cannot be reactivated.
	 * Does not cancel package loads or broadcast completion.
	 */
	UFUNCTION(BlueprintCallable, Category = AsyncLoading)
	TOROENGINE_API void Cancel();

	TOROENGINE_API virtual void Activate() override;

private:

	/** Sanitized interval in seconds between package-count samples. */
	float Interval = 0.1f;

	/** Required consecutive idle seconds, configured by the factory before activation. */
	float IdleWindow = 0.3f;

	/** Remaining idle seconds before completion; reset whenever loading is observed. */
	float IdleTime = 0.0;

	/** Countdown to the next package-count sample; reset on activation for an immediate first sample. */
	float TickTime = -1.0;

	/** Terminal completion or cancellation state, set before completion delegates are dispatched. */
	bool bFinished = false;

	/** Peak outstanding request count observed at package-count samples. */
	int32 TotalPackages = 0;

	/** Outstanding request count from the previous sample. */
	int32 RemainPackages = 0;

	/** Per-frame core ticker registration retained while the action is active. */
	FTSTicker::FDelegateHandle TickHandle;

	/** Marks the action terminal and removes its ticker; game-instance release is handled by the caller. */
	void MarkFinished();

	/**
	 * Observes loading and idle time each frame and samples package counts when due.
	 * @param DeltaTime Seconds supplied for the latest core-ticker frame.
	 * @return True to continue monitoring; false after completion or cancellation.
	 */
	bool OnTick(float DeltaTime);

	virtual void BeginDestroy() override;
};
