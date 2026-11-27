// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Objectives/ObjectiveTaskActionBehavior.h"
#include "Engine/World.h"
#include "Objectives/ObjectiveSubsystem.h"
#include "Objectives/ObjectiveSystemConfig.h"

bool UObjectiveTaskActionBehavior::CanTrigger() const
{
	const UWorld* World = GetWorld();
	return Amount > 0 && ObjectiveTags::IsValid(Task) && World && World->GetSubsystem<UObjectiveSubsystem>();
}

void UObjectiveTaskActionBehavior::OnTriggered()
{
	if (const UWorld* World = GetWorld())
	{
		if (UObjectiveSubsystem* Objectives = World->GetSubsystem<UObjectiveSubsystem>())
		{
			Objectives->FinishTask(Task, Amount);
		}
	}
}
