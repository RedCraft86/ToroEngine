// Copyright (C) RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Misc/AssertionMacros.h"
#include "UObject/ObjectMacros.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimpleCooldown.generated.h"

/**
 * Manually ticked repeating cooldown that reports at most one expiration per tick.
 * Missed intervals are skipped while preserving cadence; no world or timer is required.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FSimpleCooldown final
{
	GENERATED_BODY()

	/** Nonnegative, finite interval in seconds; zero permits expiration on every valid tick. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cooldown, meta = (ClampMin = 0.0, UIMin = 0.0))
	double Interval;

	/** Finite seconds remaining until expiration; zero or negative values are ready. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cooldown)
	double Cooldown;

	/** Constructs a ready cooldown with a 0.1-second interval. */
	FSimpleCooldown()
		: Interval(0.1), Cooldown(0.0)
	{}

	/**
	 * Constructs a cooldown with the absolute value of Time as its interval.
	 * Non-finite input emits an ensure diagnostic and uses a 0.1-second interval.
	 * @param Time Interval in seconds; negative finite values are converted to positive.
	 * @param bStartReady If true, starts ready; otherwise waits one normalized interval.
	 */
	FSimpleCooldown(double Time, bool bStartReady = true)
		: Interval(FMath::IsFinite(Time) ? FMath::Abs(Time) : 0.1)
	{
		ensureMsgf(FMath::IsFinite(Time), TEXT("Input time for FSimpleCooldown must be finite. Using 0.1 as default."));
		Cooldown = bStartReady ? 0.0 : Interval;
	}

	/**
	 * Constructs a cooldown with explicitly supplied remaining time.
	 * Non-finite input emits an ensure diagnostic and uses a 0.1-second interval.
	 * @param InInterval Interval in seconds; negative finite values are converted to positive.
	 * @param InCooldown Remaining seconds, clamped to [0, the normalized interval].
	 */
	FSimpleCooldown(double InInterval, double InCooldown)
		: FSimpleCooldown(FMath::IsFinite(InInterval) ? FMath::Abs(InInterval) : 0.1)
	{
		ensureMsgf(FMath::IsFinite(InInterval), TEXT("Input time for FSimpleCooldown must be finite. Using 0.1 as default."));
		Cooldown = FMath::Clamp(InCooldown, 0.0, Interval);
	}

	/** Restarts the cooldown using Interval; callers must keep Interval nonnegative and finite. */
	FORCEINLINE void Reset()
	{
		Cooldown = Interval;
	}

	/** Clears the remaining time so the next valid tick can report expiration. */
	FORCEINLINE void Ready()
	{
		Cooldown = 0.0;
	}

	/**
	 * Advances the cooldown and, on expiration, schedules the next interval.
	 * Overshoot preserves cadence, skipping missed triggers. Exact boundaries schedule
	 * a full interval; a zero interval expires on every valid tick.
	 * @param DeltaTime Positive, finite elapsed seconds.
	 *        Callers must keep Interval nonnegative and finite, and Cooldown finite.
	 * @return True if expiration occurs, at most once per call. Invalid delta time
	 *         emits an ensure diagnostic and returns false without modifying the cooldown.
	 */
	[[nodiscard]] bool Tick(double DeltaTime);
};

/** Blueprint construction, manual ticking, and state control for simple cooldowns. */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API USimpleCooldownLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Creates a cooldown using the absolute interval and the selected initial state.
	 * Non-finite Time emits an ensure diagnostic and uses a 0.1-second interval.
	 * @param Time Interval in seconds.
	 * @param bStartReady If true, starts ready; otherwise waits one normalized interval.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Cooldown")
	static FSimpleCooldown MakeSimpleCooldown(double Time = 0.1, bool bStartReady = true);

	/** Restarts the supplied cooldown; callers must keep its interval nonnegative and finite. */
	UFUNCTION(BlueprintCallable, Category = "Math|Cooldown")
	static void ResetCooldown(UPARAM(ref) FSimpleCooldown& Cooldown);

	/** Clears remaining time so the next valid tick can report expiration. */
	UFUNCTION(BlueprintCallable, Category = "Math|Cooldown")
	static void ReadyCooldown(UPARAM(ref) FSimpleCooldown& Cooldown);

	/**
	 * Advances the supplied cooldown, preserving cadence and skipping missed triggers.
	 * @param DeltaTime Positive, finite elapsed seconds.
	 *        Callers must keep Interval nonnegative and finite, and remaining time finite.
	 * @return True once on expiration; false for a pending cooldown or invalid delta time.
	 *         Invalid delta time emits an ensure diagnostic and leaves the cooldown unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|Cooldown")
	static bool TickCooldown(UPARAM(ref) FSimpleCooldown& Cooldown, double DeltaTime);
};
