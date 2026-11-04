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
		Trigger->OnBeginTriggerNative.AddUObject(this, &UActionBehaviorBase::DoTrigger);
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

void UActionBehaviorBase::DoTrigger()
{
	const UWorld* World = GetWorld();
	if (bTriggered || !World)
	{
		return;
	}

	FTimerManager& Timers = World->GetTimerManager();
	if (Timers.TimerExists(DelayTimer) && !bRetriggerable)
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
		Timers.SetTimer(DelayTimer, this, &UActionBehaviorBase::ExecuteAction, Delay, false);
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
	if (UTriggerBehaviorBase* Trigger = TriggerRef.Get<UTriggerBehaviorBase>())
	{
		Trigger->OnBeginTriggerNative.RemoveAll(this);
	}
}
