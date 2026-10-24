// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Interfaces/ExitableObject.h"
#include "Interfaces/ToroInterfaceUtils.h"

bool IExitableObject::RequestExit(UObject* Target, const UObject* Requester)
{
	return ObjectImplementsInterface<UExitableObject>(Target)
		&& Execute_RequestExit(Target, Requester);
}

void IExitableObject::NotifyObjectExited(UObject* Target, const UObject* ExitedObject)
{
	if (ObjectImplementsInterface<UExitableObject>(Target))
	{
		Execute_NotifyObjectExited(Target, ExitedObject);
	}
}
