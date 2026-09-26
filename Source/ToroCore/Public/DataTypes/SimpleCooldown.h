// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Math/UnrealMathUtility.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimpleCooldown.generated.h"

/**
 * A manually ticked cooldown that starts ready and resets automatically on expiration.
 * Reports at most one expiration per tick and discards overshoot rather than preserving a periodic schedule.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FSimpleCooldown final
{
	GENERATED_BODY()

	/** Duration in seconds restored on reset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cooldown)
	float Interval;

	/** Remaining seconds for the cooldown. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cooldown)
	float Cooldown;

	/** Starts ready with a default interval of 0.1 seconds. */
	FSimpleCooldown()
		: Interval(0.1f), Cooldown(0.0f)
	{}

	/**
	 * Starts ready with the supplied interval.
	 * @param Time Duration in seconds.
	 */
	FSimpleCooldown(const float Time)
		: Interval(FMath::Abs(Time)), Cooldown(0.0f)
	{}

	/** Normalizes the interval, then restores it as the remaining time in seconds. */
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
	[[nodiscard]] bool Tick(const float DeltaTime);
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

	/** Normalizes the cooldown's interval, then restores it as the remaining time. */
	UFUNCTION(BlueprintCallable, Category = SimpleCooldown)
	static void ResetCooldown(UPARAM(ref) FSimpleCooldown& Cooldown);

	/** Clears remaining time so the next accepted tick expires; leaves the interval unchanged. */
	UFUNCTION(BlueprintCallable, Category = SimpleCooldown)
	static void ReadyCooldown(UPARAM(ref) FSimpleCooldown& Cooldown);

	/**
	 * Advances the cooldown, normalizing invalid state and resetting on expiration.
	 * Reports at most one expiration per call and discards overshoot, allowing timing drift.
	 * Once ready, a zero interval expires on every accepted tick, including zero delta.
	 * @param Cooldown Cooldown to update in place.
	 * @param DeltaTime Elapsed seconds. Negative or non-finite values leave state unchanged.
	 * @return True on expiration; false while counting down or when DeltaTime is rejected.
	 */
	UFUNCTION(BlueprintCallable, Category = SimpleCooldown)
	static bool TickCooldown(UPARAM(ref) FSimpleCooldown& Cooldown, const float DeltaTime);
};
