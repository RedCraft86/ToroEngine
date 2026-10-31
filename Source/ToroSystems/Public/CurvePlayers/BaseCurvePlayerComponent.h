// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseCurvePlayerComponent.generated.h"

/** Playback direction, or the stopped state. */
UENUM(BlueprintType)
enum class ECurvePlayerState : uint8
{
	/** Playback time does not advance automatically. */
	Stopped,

	/** Playback advances toward the end of the curve. */
	Forward,

	/** Playback advances toward the start of the curve. */
	Reverse
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCurvePlayerStateChangedDelegate, const ECurvePlayerState, NewState);

/**
 * Shared time, direction, looping, and notification controls for typed curve players.
 * Playback starts stopped. Non-looping playback stops at its destination; looping playback
 * wraps excess time across the cached range, including steps spanning multiple loops.
 * BeginPlay initializes the range and resets time to its start without evaluation notification.
 * Before BeginPlay, derived players require SetCurve with nonempty data to initialize the range.
 * Playback and seeking require a valid curve with a duration greater than 0.1 seconds.
 */
UCLASS(Abstract, NotBlueprintable, BlueprintType)
class TOROSYSTEMS_API UBaseCurvePlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UBaseCurvePlayerComponent();

	/**
	 * Stops playback and disables ticking, broadcasting state if it changed.
	 * @param bReset Also seeks to the start when the curve is valid, notifying evaluation listeners if time changes.
	 */
	UFUNCTION(BlueprintCallable, Category = CurvePlayer)
	void StopPlaying(bool bReset = false);

	/**
	 * Plays toward the end, resuming at the current time unless a restart is needed.
	 * Non-looping playback at the end restarts from the start. Invalid curves call StopPlaying(true).
	 * Calling while already playing forward has no effect unless bFromStart is true.
	 * State changes are broadcast before any restart seek.
	 * @param bFromStart Seek to the start, notifying evaluation listeners if time changes.
	 */
	UFUNCTION(BlueprintCallable, Category = CurvePlayer)
	void PlayForward(bool bFromStart = false);

	/**
	 * Plays toward the start, resuming at the current time unless a restart is needed.
	 * Non-looping playback at the start restarts from the end. Invalid curves call StopPlaying(true).
	 * Calling while already playing in reverse has no effect unless bFromEnd is true.
	 * State changes are broadcast before any restart seek.
	 * @param bFromEnd Seek to the end, notifying evaluation listeners if time changes.
	 */
	UFUNCTION(BlueprintCallable, Category = CurvePlayer)
	void PlayReverse(bool bFromEnd = false);

	/**
	 * Seeks within the cached range without changing playback state; ignores invalid curves.
	 * A clamped time nearly equal to the current time leaves time unchanged and emits no notification.
	 * @param InTime Requested curve time in seconds, clamped to the cached start and end.
	 * @param bNotify Broadcast evaluation when the clamped time changes beyond the near-equality tolerance.
	 */
	UFUNCTION(BlueprintCallable, Category = CurvePlayer)
	void SetPlaybackTime(float InTime, bool bNotify = true);

	/** Gets the current curve time in seconds. */
	UFUNCTION(BlueprintPure, Category = CurvePlayer)
	[[nodiscard]] FORCEINLINE float GetPlaybackTime() const { return CurrentTime; }

	/**
	 * Seeks to the cached start when the curve is valid, without changing playback state.
	 * @param bNotify Broadcast evaluation if time changes beyond the near-equality tolerance.
	 */
	UFUNCTION(BlueprintCallable, Category = CurvePlayer)
	void SnapToStart(bool bNotify = true) { SetPlaybackTime(TimeRange.X, bNotify); }

	/**
	 * Seeks to the cached end when the curve is valid, without changing playback state.
	 * @param bNotify Broadcast evaluation if time changes beyond the near-equality tolerance.
	 */
	UFUNCTION(BlueprintCallable, Category = CurvePlayer)
	void SnapToEnd(bool bNotify = true) { SetPlaybackTime(TimeRange.Y, bNotify); }

	/**
	 * Controls endpoint wrapping without changing playback state or time.
	 * @param bInLooping Whether playback wraps instead of stopping at its destination.
	 */
	UFUNCTION(BlueprintCallable, Category = CurvePlayer)
	FORCEINLINE void SetLooping(bool bInLooping) { bLooping = bInLooping; }

	/** Returns whether playback wraps at the curve endpoints. */
	UFUNCTION(BlueprintPure, Category = CurvePlayer)
	[[nodiscard]] FORCEINLINE bool IsLooping() const { return bLooping; }

	/**
	 * Sets the playback speed multiplier without changing direction.
	 * @param InPlayRate Requested multiplier, clamped to at least 0.05; use PlayReverse to change direction.
	 */
	UFUNCTION(BlueprintCallable, Category = CurvePlayer)
	FORCEINLINE void SetPlayRate(float InPlayRate) { PlayRate = FMath::Max(InPlayRate, 0.05f); }

	/** Gets the speed multiplier; 1.0 advances one curve second per tick second. */
	UFUNCTION(BlueprintPure, Category = CurvePlayer)
	[[nodiscard]] FORCEINLINE float GetPlayRate() const { return PlayRate; }

	/** Gets cached start (X) and end (Y) times in seconds; both are zero before initialization. */
	UFUNCTION(BlueprintPure, Category = CurvePlayer)
	[[nodiscard]] FORCEINLINE FVector2D GetTimeRange() const { return TimeRange; }

	/** Gets the current playback direction or stopped state. */
	UFUNCTION(BlueprintPure, Category = CurvePlayer)
	[[nodiscard]] FORCEINLINE ECurvePlayerState GetPlayState() const { return PlayState; }

	/** Returns whether the playback state is forward or reverse, independently of curve validity. */
	UFUNCTION(BlueprintPure, Category = CurvePlayer)
	[[nodiscard]] FORCEINLINE bool IsPlaying() const { return PlayState != ECurvePlayerState::Stopped; }

	/** Returns whether time is nearly equal to the cached start, regardless of curve validity. */
	UFUNCTION(BlueprintPure, Category = CurvePlayer)
	[[nodiscard]] FORCEINLINE bool IsAtStart() const { return FMath::IsNearlyEqual(TimeRange.X, CurrentTime); }

	/** Returns whether time is nearly equal to the cached end, regardless of curve validity. */
	UFUNCTION(BlueprintPure, Category = CurvePlayer)
	[[nodiscard]] FORCEINLINE bool IsAtEnd() const { return FMath::IsNearlyEqual(TimeRange.Y, CurrentTime); }

	/**
	 * Returns whether the cached duration exceeds 0.1 seconds.
	 * Derived players also require curve data; durations of 0.1 seconds or less cannot play or seek.
	 */
	UFUNCTION(BlueprintPure, Category = CurvePlayer)
	[[nodiscard]] virtual bool IsValidCurve() const { return TimeRange.Y - TimeRange.X > 0.1f; }

	/** Blueprint playback-state notification, broadcast after native state listeners. */
	UPROPERTY(BlueprintAssignable, DisplayName = "On State Changed")
	FCurvePlayerStateChangedDelegate OnStateChanged;

	DECLARE_MULTICAST_DELEGATE_OneParam(FStateChangedDelegateNative, const ECurvePlayerState);
	/** Native playback-state notification, broadcast before OnStateChanged. */
	FStateChangedDelegateNative OnStateChangedNative;

protected:

	/** Whether playback wraps at endpoints; initially false. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (DisplayPriority = -1))
	bool bLooping;

	/** Playback speed multiplier; initially 1.0, with an editor and setter minimum of 0.05. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.05f, UIMin = 0.05f, DisplayPriority = -1))
	float PlayRate;

	/** Current curve time in seconds; initially zero and reset to the start by InitializeCurve. */
	UPROPERTY(VisibleAnywhere, Category = Settings, meta = (DisplayPriority = -1))
	float CurrentTime;

	/** Cached start (X) and end (Y) in seconds; initially zero and refreshed by InitializeCurve. */
	FVector2D TimeRange;

	/** Current playback direction or stopped state; initially Stopped. */
	ECurvePlayerState PlayState;

	/**
	 * Broadcasts the current state to native listeners, followed by Blueprint listeners.
	 * Each broadcast reads PlayState, so native listeners can change the state seen by Blueprint listeners.
	 */
	void BroadcastState() const;

	/**
	 * Implemented by derived players to broadcast the evaluated value and current time in seconds.
	 * Called on valid playback ticks, notified seeks that change time, and accepted curve replacements.
	 */
	virtual void BroadcastEval() const PURE_VIRTUAL(UBaseCurvePlayerComponent::BroadcastEval,);

	/**
	 * Implemented by derived players to refresh TimeRange and reset CurrentTime to its start.
	 * Must not broadcast evaluation; called by BeginPlay and after an accepted curve replacement.
	 */
	virtual void InitializeCurve() PURE_VIRTUAL(UBaseCurvePlayerComponent::InitializeCurve,);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Func) override;
};
