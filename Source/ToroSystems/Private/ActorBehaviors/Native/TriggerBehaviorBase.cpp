// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Native/TriggerBehaviorBase.h"

UTriggerBehaviorBase::UTriggerBehaviorBase(): bTriggered(false), ElapsedTime(0.0f)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
}

void UTriggerBehaviorBase::StartTrigger()
{
	if (!bTriggered)
	{
		bTriggered = true;
		ElapsedTime = 0.0f;
		OnBeginTriggerNative.Broadcast();
		OnBeginTrigger.Broadcast();
		SetComponentTickEnabled(true);
	}
}

void UTriggerBehaviorBase::EndTrigger()
{
	if (bTriggered)
	{
		SetComponentTickEnabled(false);
		OnEndTriggerNative.Broadcast();
		OnEndTrigger.Broadcast();
		ElapsedTime = 0.0f;
		bTriggered = false;
	}
}

void UTriggerBehaviorBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunc)
{
	Super::TickComponent(DeltaTime, TickType, TickFunc);
	if (bTriggered)
	{
		ElapsedTime += DeltaTime;
		OnTriggerTickNative.Broadcast(ElapsedTime, DeltaTime);
		OnTriggerTick.Broadcast(ElapsedTime, DeltaTime);
	}
}
