// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "ActorBehaviors/Trigger/TriggerBehaviorBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

UActionBehaviorBase::UActionBehaviorBase()
	: bOneShot(false), bPersistent(false), Delay(0.0f), bRetriggerable(false), bTriggered(false)
{
	TriggerRef = FActorBehaviorRef::MakeFor<UTriggerBehaviorBase>(true);
}

void UActionBehaviorBase::BeginPlay()
{
	Super::BeginPlay();
	if (UTriggerBehaviorBase* Trigger = TriggerRef.Get<UTriggerBehaviorBase>())
	{
		BoundTrigger = Trigger;
		Trigger->OnBeginTriggerNative().AddUObject(this, &UActionBehaviorBase::DoTrigger);
	}
}

void UActionBehaviorBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindTrigger();
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DelayTimer);
	}

	Super::EndPlay(EndPlayReason);
}

bool UActionBehaviorBase::CanAcceptTrigger(const UTriggerBehaviorBase* Trigger) const
{
	const UWorld* World = GetWorld();
	return !bTriggered && Trigger && BoundTrigger.Get() == Trigger && World
		&& (bRetriggerable || !World->GetTimerManager().TimerExists(DelayTimer))
		&& CanTrigger();
}

void UActionBehaviorBase::DoTrigger()
{
	if (!CanAcceptTrigger(BoundTrigger.Get()))
	{
		return;
	}

	if (bOneShot)
	{
		bTriggered = true;
		UnbindTrigger();
	}

	if (Delay >= 0.1f)
	{
		GetWorld()->GetTimerManager().SetTimer(DelayTimer, this, &UActionBehaviorBase::ExecuteAction, Delay, false);
	}
	else
	{
		ExecuteAction();
	}
}

void UActionBehaviorBase::ExecuteAction()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DelayTimer);
	}

	OnTriggered();
	ReceiveOnTriggered();
}

void UActionBehaviorBase::UnbindTrigger()
{
	if (UTriggerBehaviorBase* Trigger = BoundTrigger.Get())
	{
		Trigger->OnBeginTriggerNative().RemoveAll(this);
	}
	BoundTrigger.Reset();
}
