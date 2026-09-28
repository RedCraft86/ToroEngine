// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Interfaces/ExitableObject.h"

namespace
{
	bool IsValidExitableTarget(const UObject* Target)
	{
		return IsValid(Target) && Target->Implements<UExitableObject>();
	}
}

bool IExitableObject::RequestExit(UObject* Target, const UObject* Requester)
{
	return IsValidExitableTarget(Target) && Execute_RequestExit(Target, Requester);
}

void IExitableObject::SendObjectExitedNotification(UObject* Receiver, const UObject* ExitedObject)
{
	if (IsValidExitableTarget(Receiver))
	{
		Execute_NotifyObjectExited(Receiver, ExitedObject);
	}
}
