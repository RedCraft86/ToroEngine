// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "GameEntity/GameEntityComponentRef.h"
#include "Serialization/Archive.h"

void FGameEntityComponentRef::BindOwner(AActor* InOwner)
{
	OwnerActor = InOwner;
	Reset();
}

UGameEntityComponent* FGameEntityComponentRef::FindComponent() const
{
	if (!OwnerActor.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("No owner actor bound."), ELogVerbosity::Error);
		return nullptr;
	}

	if (!IsValid(ComponentClass))
	{
		FFrame::KismetExecutionMessage(TEXT("No component class to get."), ELogVerbosity::Error);
		return nullptr;
	}

	return Cast<UGameEntityComponent>(OwnerActor->GetComponentByClass(ComponentClass));
}

void FGameEntityComponentRef::Reset()
{
	Reference.Reset();
}

UGameEntityComponent* FGameEntityComponentRef::Get()
{
	if (Reference.IsValid())
	{
		return Reference.Get();
	}

	Reference = FindComponent();

	// Fix: Failure formatting must remain safe when the owner or configured class is missing.
	checkf(Reference.IsValid(),
		TEXT("Could not find component class %s in actor %s."),
		*GetNameSafe(ComponentClass.Get()), *GetNameSafe(GetOwner()))

	return Reference.Get();
}

void FGameEntityComponentRef::PostSerialize(const FArchive& Ar)
{
	if (Ar.IsLoading())
	{
		BindOwner(nullptr);
	}
}
