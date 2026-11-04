// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Interfaces/ActivatableObject.h"
#include "Utilities/ToroUtilities.h"

bool IActivatableObject::GetActiveState(const UObject* Target)
{
	return !ToroEngine::ImplementsInterface<UActivatableObject>(Target)
		|| Execute_GetActiveState(Target);
}

void IActivatableObject::SetActiveState(UObject* Target, bool bNewState)
{
	if (ToroEngine::ImplementsInterface<UActivatableObject>(Target))
	{
		Execute_SetActiveState(Target, bNewState);
	}
}
