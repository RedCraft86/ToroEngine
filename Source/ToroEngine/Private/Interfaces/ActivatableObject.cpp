// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Interfaces/ActivatableObject.h"
#include "Interfaces/ToroInterfaceUtils.h"

bool IActivatableObject::GetActiveState(const UObject* Target)
{
	return !ObjectImplementsInterface<UActivatableObject>(Target)
		|| Execute_GetActiveState(Target);
}

void IActivatableObject::SetActiveState(UObject* Target, bool bNewState)
{
	if (ObjectImplementsInterface<UActivatableObject>(Target))
	{
		Execute_SetActiveState(Target, bNewState);
	}
}
