// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "GameFlags/GameFlagActionBehavior.h"

bool UGameFlagActionBehavior::CanTrigger() const
{
	return Flag.IsValid() && UGameFlagManager::Get(this);
}

void UGameFlagActionBehavior::OnTriggered()
{
	if (UGameFlagManager* Flags = UGameFlagManager::Get(this))
	{
		Flags->SetGameFlag(Flag, Scope);
	}
}
