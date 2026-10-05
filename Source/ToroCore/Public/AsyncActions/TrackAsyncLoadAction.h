// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Containers/Ticker.h"
#include "ToroAsyncActionBase.h"
#include "TrackAsyncLoadAction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTrackAsyncLoadDelegate, const FIntPoint, Packages, const float, Percent);
DECLARE_MULTICAST_DELEGATE_TwoParams(FTrackAsyncLoadDelegateNative, const FIntPoint, const float);

/**
 * Polls the global pending async-package count and reports estimated loading progress.
 * Tracking waits until a positive pending count has been observed, then finishes when
 * the count reaches the configured threshold. An always-idle loader never completes
 * this action. The observed packages are not restricted to the supplied world context.
 * Create and activate on the game thread. Repeated activation while active or after
 * completion has no effect; create a new action to track another loading period.
 */
UCLASS(MinimalAPI, NotBlueprintable, NotBlueprintType)
class UTrackAsyncLoadAction final : public UToroAsyncActionBase
{
	GENERATED_BODY()

public:

	/**
	 * Blueprint progress notification. Packages.X is the peak observed pending count
	 * minus the current pending count; Packages.Y is that peak, not a known total.
	 * Percent is a nondecreasing estimate from 0 to 1 and can differ from X / Y when
	 * new packages arrive. Completion reports Packages = (peak, peak) and Percent = 1,
	 * even if packages remain within the threshold. Native listeners are notified first.
	 */
	UPROPERTY(BlueprintAssignable)
	FTrackAsyncLoadDelegate OnUpdate;

	/** Native progress notification with the same payload as OnUpdate; broadcast before it. */
	FTrackAsyncLoadDelegateNative OnUpdateNative;

	/**
	 * Creates a tracker and registers it with the resolved world's game
	 * instance when available. Call only from the game thread. Blueprint async nodes
	 * activate after binding; native callers bind callbacks and call Activate() through
	 * the public UBlueprintAsyncActionBase interface.
	 * @param TestInterval Finite polling interval in seconds, clamped to at least 0.01.
	 *        The first active ticker callback polls immediately; later polls depend on ticker timing.
	 * @param FinishThreshold Maximum pending-package count accepted as complete after
	 *        a positive count has been observed. Zero waits for no pending packages.
	 * @note If calling in C++, make sure to call ->Activate on the action object.
	 */
	UFUNCTION(BlueprintCallable, Category = AsyncLoading, DisplayName = "Track Async Loading", meta = (BlueprintInternalUseOnly = true, WorldContext = ContextObject))
	[[nodiscard]] static TOROCORE_API UTrackAsyncLoadAction* TrackAsyncLoading(const UObject* ContextObject, const float TestInterval = 0.1f, const uint8 FinishThreshold = 5);

private:

	float Interval = 0.1f;
	uint8 Threshold = 5;

	float Percent = 0.0f;
	int32 MaxPackages = 0;
	float TickInterval = 0.0f;
	bool bInitialized = false;
	bool bFinished = false;
	FTSTicker::FDelegateHandle TickHandle;

	void OnFinished();
	bool OnTick(float DeltaTime);

	virtual void Activate() override;
	virtual void BeginDestroy() override;
};
