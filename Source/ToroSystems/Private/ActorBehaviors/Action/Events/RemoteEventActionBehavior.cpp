// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Action/Events/RemoteEventActionBehavior.h"
#include "Libraries/ToroWorldLibrary.h"

void URemoteEventActionBehavior::OnTriggered()
{
	UToroWorldLibrary::CallRemoteEvent(this, EventName);
}
