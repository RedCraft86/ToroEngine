// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "ActorBehaviors/Trigger/TriggerBehaviorBase.h"

UActionBehaviorBase::UActionBehaviorBase()
	: bOneShot(false), bPersistent(false), bTriggered(false)
{
	TriggerRef = FActorBehaviorRef::MakeFor<UTriggerBehaviorBase>(true);
}

void UActionBehaviorBase::BeginPlay()
{
	Super::BeginPlay();
	if (UTriggerBehaviorBase* Trigger = TriggerRef.Get<UTriggerBehaviorBase>())
	{
		Trigger->OnBeginTriggerNative.AddUObject(this, &UActionBehaviorBase::DoTrigger);
	}
}

void UActionBehaviorBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindTrigger();
	Super::EndPlay(EndPlayReason);
}

void UActionBehaviorBase::DoTrigger()
{
	if (!bTriggered)
	{
		if (bOneShot)
		{
			bTriggered = true;
			UnbindTrigger();
		}

		OnTriggered();
		ReceiveOnTriggered();
	}
}

void UActionBehaviorBase::UnbindTrigger()
{
	if (UTriggerBehaviorBase* Trigger = TriggerRef.Get<UTriggerBehaviorBase>())
	{
		Trigger->OnBeginTriggerNative.RemoveAll(this);
	}
}
