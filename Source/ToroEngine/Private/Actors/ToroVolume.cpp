// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Actors/ToroVolume.h"

AToroVolume::AToroVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

bool AToroVolume::GetActiveState_Implementation() const
{
	return IActivatableObject::GetActiveState_Implementation();
}

void AToroVolume::SetActiveState_Implementation(bool bNewState)
{
	IActivatableObject::SetActiveState_Implementation(bNewState);
}
