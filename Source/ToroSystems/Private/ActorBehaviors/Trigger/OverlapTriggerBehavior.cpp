// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Trigger/OverlapTriggerBehavior.h"

void UOverlapTriggerBehavior::OnBeginOverlap(AActor*, AActor* OtherActor)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (IsValid(Pawn) && Pawn->IsPlayerControlled())
	{
		StartTrigger();
	}
}

void UOverlapTriggerBehavior::OnEndOverlap(AActor*, AActor* OtherActor)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (IsValid(Pawn) && Pawn->IsPlayerControlled())
	{
		EndTrigger();
	}
}

void UOverlapTriggerBehavior::BeginPlay()
{
	Super::BeginPlay();
	AActor* OwnerActor = GetOwner();
	if (IsValid(OwnerActor))
	{
		OwnerActor->OnActorBeginOverlap.AddUniqueDynamic(this, &UOverlapTriggerBehavior::OnBeginOverlap);
		OwnerActor->OnActorEndOverlap.AddUniqueDynamic(this, &UOverlapTriggerBehavior::OnEndOverlap);
	}
}

void UOverlapTriggerBehavior::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AActor* OwnerActor = GetOwner();
	if (IsValid(OwnerActor))
	{
		OwnerActor->OnActorBeginOverlap.RemoveAll(this);
		OwnerActor->OnActorEndOverlap.RemoveAll(this);
	}
	Super::EndPlay(EndPlayReason);
}
