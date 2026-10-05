// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "DataTypes/SimpleCooldown.h"

bool FSimpleCooldown::Tick(float DeltaTime)
{
	if ((Cooldown -= DeltaTime) <= 0.0f)
	{
		Reset();
		return true;
	}

	return false;
}

void USimpleCooldownLibrary::ResetCooldown(FSimpleCooldown& Cooldown)
{
	Cooldown.Reset();
}

void USimpleCooldownLibrary::ReadyCooldown(FSimpleCooldown& Cooldown)
{
	Cooldown.Ready();
}

bool USimpleCooldownLibrary::TickCooldown(FSimpleCooldown& Cooldown, float DeltaTime)
{
	return Cooldown.Tick(DeltaTime);
}
