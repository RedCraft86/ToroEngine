// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Trigger/InteractTriggerBehavior.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "Interfaces/ActivatableObject.h"
#include "GameFramework/Actor.h"

bool UInteractTriggerBehavior::CanBeInteractedWith() const
{
	const AActor* Owner = GetOwner();
	if (!bInteractable || !IsActive() || !IsValid(Owner)
		|| !IActivatableObject::GetActiveState(Owner) || Owner->IsHidden())
	{
		return false;
	}

	if (!bRequireCallableAction)
	{
		return true;
	}

	TInlineComponentArray<UActionBehaviorBase*> Actions;
	Owner->GetComponents<UActionBehaviorBase>(Actions);
	for (const UActionBehaviorBase* Action : Actions)
	{
		if (IsValid(Action) && Action->CanAcceptTrigger(this))
		{
			return true;
		}
	}

	return false;
}
