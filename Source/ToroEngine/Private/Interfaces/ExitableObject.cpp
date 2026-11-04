// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Interfaces/ExitableObject.h"
#include "Utilities/ToroUtilities.h"

bool IExitableObject::RequestExit(UObject* Target, const UObject* Requester)
{
	return ToroEngine::ImplementsInterface<UExitableObject>(Target)
		&& Execute_RequestExit(Target, Requester);
}

void IExitableObject::NotifyObjectExited(UObject* Target, const UObject* ExitedObject)
{
	if (ToroEngine::ImplementsInterface<UExitableObject>(Target))
	{
		Execute_NotifyObjectExited(Target, ExitedObject);
	}
}
