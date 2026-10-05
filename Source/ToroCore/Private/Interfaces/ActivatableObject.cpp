// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Interfaces/ActivatableObject.h"

namespace
{
	bool IsValidActivatableTarget(const UObject* Target)
	{
		return IsValid(Target) && Target->Implements<UActivatableObject>();
	}
}

bool IActivatableObject::GetActiveState(const UObject* Target)
{
	return IsValidActivatableTarget(Target) && Execute_GetActiveState(Target);
}

void IActivatableObject::SetActiveState(UObject* Target, bool bNewState)
{
	if (IsValidActivatableTarget(Target))
	{
		Execute_SetActiveState(Target, bNewState);
	}
}
