// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Actors/ToroActor.h"

AToroActor::AToroActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

bool AToroActor::GetActiveState_Implementation() const
{
	return IActivatableObject::GetActiveState_Implementation();
}

void AToroActor::SetActiveState_Implementation(bool bNewState)
{
	IActivatableObject::SetActiveState_Implementation(bNewState);
}
