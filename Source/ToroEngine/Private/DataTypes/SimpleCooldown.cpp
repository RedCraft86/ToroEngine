// Copyright (C) RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DataTypes/SimpleCooldown.h"

bool FSimpleCooldown::Tick(double DeltaTime)
{
	if (!ensureMsgf(DeltaTime > 0.0 && FMath::IsFinite(DeltaTime),
		TEXT("FSimpleCooldown::Tick called with an invalid DeltaTime.")))
	{
		return false;
	}

	if (Interval == 0.0)
	{
		Cooldown = 0.0;
		return true;
	}

	const double Remaining = Cooldown - DeltaTime;
	if (Remaining <= 0.0)
	{
		// Carry overshoot forward in phase, skipping missed triggers and resetting exact boundaries.
		Cooldown = Interval + FMath::Fmod(Remaining, Interval);
		return true;
	}

	Cooldown = Remaining;
	return false;
}

FSimpleCooldown USimpleCooldownLibrary::MakeSimpleCooldown(double Time, bool bStartReady)
{
	return FSimpleCooldown(Time, bStartReady);
}

void USimpleCooldownLibrary::ResetCooldown(FSimpleCooldown& Cooldown)
{
	Cooldown.Reset();
}

void USimpleCooldownLibrary::ReadyCooldown(FSimpleCooldown& Cooldown)
{
	Cooldown.Ready();
}

bool USimpleCooldownLibrary::TickCooldown(FSimpleCooldown& Cooldown, double DeltaTime)
{
	return Cooldown.Tick(DeltaTime);
}
