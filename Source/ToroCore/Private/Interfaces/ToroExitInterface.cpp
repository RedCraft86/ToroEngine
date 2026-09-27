// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Interfaces/ToroExitInterface.h"

namespace
{
	bool ImplementsInterface(const UObject* Target)
	{
		return IsValid(Target) && Target->Implements<UToroExitInterface>();
	}
}

bool IToroExitInterface::SendExitRequest(UObject* Target, const UObject* Requester)
{
	return ImplementsInterface(Target) && Execute_RequestExit(Target, Requester);
}

void IToroExitInterface::SendObjectExitedNotification(UObject* Receiver, const UObject* ExitedObject)
{
	if (ImplementsInterface(Receiver))
	{
		Execute_NotifyObjectExited(Receiver, ExitedObject);
	}
}
