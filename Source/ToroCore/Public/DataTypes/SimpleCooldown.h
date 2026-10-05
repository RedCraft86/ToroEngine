// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Math/UnrealMathUtility.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimpleCooldown.generated.h"

/**
 * A manually ticked cooldown that starts ready and resets automatically on expiration.
 * Callers must keep Interval finite and nonnegative, and Cooldown finite.
 * Reports at most one expiration per tick and discards overshoot rather than preserving a periodic schedule.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FSimpleCooldown final
{
	GENERATED_BODY()

	/** Duration in seconds restored on reset; must be finite. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cooldown, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float Interval;

	/** Remaining seconds in the cooldown. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cooldown)
	float Cooldown;

	/** Starts ready with a default interval of 0.1 seconds. */
	FSimpleCooldown()
		: Interval(0.1f), Cooldown(0.0f)
	{}

	/**
	 * Starts ready with the supplied interval.
	 * @param Time Finite duration in seconds; its absolute value is used as the interval.
	 */
	FSimpleCooldown(float Time)
		: Interval(FMath::Abs(Time)), Cooldown(0.0f)
	{
		ensureAlwaysMsgf(FMath::IsFinite(Time), TEXT("Cooldown time %f is not finite."), Time);
	}

	/** Restores the interval as the remaining time in seconds. */
	FORCEINLINE void Reset()
	{
		Cooldown = Interval;
	}

	/** Clears remaining time so the next accepted tick expires; leaves the interval unchanged. */
	FORCEINLINE void Ready()
	{
		Cooldown = 0.0f;
	}

	/**
	 * Advances the countdown, resetting to the full interval on expiration.
	 * Reports at most one expiration per call and discards overshoot, allowing timing drift.
	 * Once ready, a zero interval expires on every accepted tick, including zero delta.
	 * @param DeltaTime Elapsed seconds. Negative or non-finite values leave state unchanged.
	 * @return True on expiration; false while counting down or when DeltaTime is rejected.
	 */
	[[nodiscard]] bool Tick(float DeltaTime);
};

/**
 * Blueprint helpers for resetting, readying, and manually ticking FSimpleCooldown.
 * Operations update the supplied struct in place and use its input safeguards.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API USimpleCooldownLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Restores the cooldown's interval as the remaining time. */
	UFUNCTION(BlueprintCallable, Category = SimpleCooldown)
	static void ResetCooldown(UPARAM(ref) FSimpleCooldown& Cooldown);

	/** Clears remaining time so the next accepted tick expires; leaves the interval unchanged. */
	UFUNCTION(BlueprintCallable, Category = SimpleCooldown)
	static void ReadyCooldown(UPARAM(ref) FSimpleCooldown& Cooldown);

	/**
	 * Advances the cooldown, resetting to the full interval on expiration.
	 * Reports at most one expiration per call and discards overshoot, allowing timing drift.
	 * Once ready, a zero interval expires on every accepted tick, including zero delta.
	 * @param Cooldown Cooldown to update in place.
	 * @param DeltaTime Elapsed seconds. Negative or non-finite values leave state unchanged.
	 * @return True on expiration; false while counting down or when DeltaTime is rejected.
	 */
	UFUNCTION(BlueprintCallable, Category = SimpleCooldown)
	static bool TickCooldown(UPARAM(ref) FSimpleCooldown& Cooldown, float DeltaTime);
};
