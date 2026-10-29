// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/ActorBehaviorRef.h"
#include "Utilities/ToroLogging.h"
#include "ActorBehaviors/ActorBehaviorComponent.h"
#include "Containers/UnrealString.h"
#include "Serialization/Archive.h"
#include "Misc/AssertionMacros.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "GameFramework/Actor.h"

bool FActorBehaviorRef::IsInvalid() const
{
	return !IsValid(Class) || !Owner.IsValid();
}

bool FActorBehaviorRef::CanResolveAs(TSubclassOf<UActorBehaviorComponent> InClass, bool bErrorIfInvalid) const
{
	const bool bValid = IsValid(Class) && IsValid(InClass) && Class->IsChildOf(InClass);
	if (!bValid && bErrorIfInvalid)
	{
		TORO_REPORT_ERROR(TEXT("Resolving expects %s but BehaviorRef can only find %s"), *GetNameSafe(InClass), *GetNameSafe(Class));
	}

	return bValid;
}

void FActorBehaviorRef::Reset()
{
	Reference.Reset();
}

void FActorBehaviorRef::BindOwner(const AActor* InActor)
{
	Owner = InActor;
	Reset();
}

UActorBehaviorComponent* FActorBehaviorRef::Find() const
{
	if (!Owner.IsValid())
	{
		TORO_REPORT_ERROR(TEXT("No owner actor bound."));
		return nullptr;
	}

	if (!IsValid(Class))
	{
		TORO_REPORT_ERROR(TEXT("No component class to get."));
		return nullptr;
	}

	return Cast<UActorBehaviorComponent>(Owner->FindComponentByClass(Class));
}

UActorBehaviorComponent* FActorBehaviorRef::Get()
{
	if (!Reference.IsValid())
	{
		Reference = Find();
	}

	if (bRequired && !Reference.IsValid())
	{
		TORO_REPORT_FATAL(
			TEXT("Failed to find behavior component %s in actor %s but it is required by a different behavior."),
			*GetNameSafe(Class.Get()), *GetNameSafe(Owner.Get())
		);
		return nullptr;
	}

	return Reference.Get();
}

void FActorBehaviorRef::PostSerialize(const FArchive& Ar)
{
	if (Ar.IsLoading())
	{
		BindOwner(nullptr);
	}
}

bool UActorBehaviorRefLibrary::IsBehaviorRefInvalid(const FActorBehaviorRef& BehaviorRef)
{
	return BehaviorRef.IsInvalid();
}

UActorBehaviorComponent* UActorBehaviorRefLibrary::FindBehaviorRef(
	const FActorBehaviorRef& BehaviorRef, TSubclassOf<UActorBehaviorComponent> InClass)
{
	return BehaviorRef.CanResolveAs(InClass, true) ? BehaviorRef.Find() : nullptr;
}

UActorBehaviorComponent* UActorBehaviorRefLibrary::GetBehaviorRef(
	FActorBehaviorRef& BehaviorRef, TSubclassOf<UActorBehaviorComponent> InClass)
{
	return BehaviorRef.CanResolveAs(InClass, true) ? BehaviorRef.Get() : nullptr;
}

void UActorBehaviorRefLibrary::ResetBehaviorRef(FActorBehaviorRef& BehaviorRef)
{
	BehaviorRef.Reset();
}

void UActorBehaviorRefLibrary::BindBehaviorRefOwner(FActorBehaviorRef& BehaviorRef, const AActor* InActor)
{
	BehaviorRef.BindOwner(InActor);
}
