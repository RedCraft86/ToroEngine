// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Actors/ToroCharacter.h"

AToroCharacter::AToroCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

bool AToroCharacter::GetActiveState_Implementation() const
{
	return IActivatableObject::GetActiveState_Implementation();
}

void AToroCharacter::SetActiveState_Implementation(bool bNewState)
{
	IActivatableObject::SetActiveState_Implementation(bNewState);
}
