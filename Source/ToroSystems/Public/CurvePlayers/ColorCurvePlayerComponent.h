// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DataTypes/InlineCurves.h"
#include "BaseCurvePlayerComponent.h"
#include "ColorCurvePlayerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FColorCurvePlayerEvaluateDelegate, FLinearColor, Value, float, Time);

/**
 * Plays a curve of linear RGBA values, broadcasting its value and time as playback advances.
 * Uses inline keys or an external curve asset through FInlineColorCurve.
 * Defaults to a one-second curve from black to white; playback is initially stopped.
 * BeginPlay initializes the cached range and resets time to its start without notification.
 * SetCurve can initialize the range earlier; playback requires a duration greater than 0.1 seconds.
 */
UCLASS(NotBlueprintable, BlueprintType, DisplayName = "Curve Player (Color)",
	ClassGroup = (Animation), meta = (BlueprintSpawnableComponent))
class TOROSYSTEMS_API UColorCurvePlayerComponent final : public UBaseCurvePlayerComponent
{
	GENERATED_BODY()

public:

	UColorCurvePlayerComponent();

	/**
	 * Replaces the curve when it contains data; empty input leaves the player unchanged.
	 * Stops playback, refreshes the time range, resets time to its start, and broadcasts evaluation.
	 * Nonempty curves are accepted even when their duration is too short for playback or seeking.
	 * @param InCurve Curve data to copy, including any assigned external asset reference.
	 */
	UFUNCTION(BlueprintCallable, Category = CurvePlayer)
	void SetCurve(const FInlineColorCurve& InCurve);

	/** Gets the stored curve wrapper by const reference. */
	UFUNCTION(BlueprintPure, Category = CurvePlayer)
	[[nodiscard]] const FInlineColorCurve& GetCurve() const { return Curve; }

	/** Evaluates the curve at the current time without notifying listeners or requiring playback validity. */
	UFUNCTION(BlueprintPure, Category = CurvePlayer)
	[[nodiscard]] FLinearColor GetValue() const { return Curve.GetValue(CurrentTime); }

	/**
	 * Broadcasts the evaluated value and curve time in seconds after native evaluation listeners.
	 * Emitted on playback ticks, accepted curve replacements, and notified seeks that change time.
	 */
	UPROPERTY(BlueprintAssignable, DisplayName = "On Evaluate")
	FColorCurvePlayerEvaluateDelegate OnEvaluate;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FEvaluateDelegateNative, FLinearColor, float);
	/** Native evaluated-value and current-time notification, broadcast before OnEvaluate. */
	FEvaluateDelegateNative OnEvaluateNative;

	virtual bool IsValidCurve() const override
	{
		return Super::IsValidCurve() && Curve.HasAnyData();
	}

protected:

	/** Inline keys or an external asset; use SetCurve at runtime to refresh the cached range. */
	UPROPERTY(EditAnywhere, Category = Settings)
	FInlineColorCurve Curve;

	virtual void InitializeCurve() override;
	virtual void BroadcastEval() const override;
};
