// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/ToroBehaviorRef.h"
#include "Utilities/ToroLogging.h"
#include "ActorBehaviors/ToroBehaviorComponent.h"
#include "Containers/UnrealString.h"
#include "Serialization/Archive.h"
#include "Misc/AssertionMacros.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "GameFramework/Actor.h"

bool FToroBehaviorRef::IsInvalid() const
{
	return !IsValid(Class) || !Owner.IsValid();
}

bool FToroBehaviorRef::CanResolveAs(TSubclassOf<UToroBehaviorComponent> InClass, bool bErrorIfInvalid) const
{
	const bool bValid = IsValid(Class) && IsValid(InClass) && Class->IsChildOf(InClass);
	if (!bValid && bErrorIfInvalid)
	{
		TORO_REPORT_ERROR(TEXT("Resolving expects %s but BehaviorRef can only find %s"), *GetNameSafe(InClass), *GetNameSafe(Class));
	}

	return bValid;
}

void FToroBehaviorRef::Reset()
{
	Reference.Reset();
}

void FToroBehaviorRef::BindOwner(const AActor* InActor)
{
	Owner = InActor;
	Reset();
}

UToroBehaviorComponent* FToroBehaviorRef::Find() const
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

	return Cast<UToroBehaviorComponent>(Owner->FindComponentByClass(Class));
}

UToroBehaviorComponent* FToroBehaviorRef::Get()
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

void FToroBehaviorRef::PostSerialize(const FArchive& Ar)
{
	if (Ar.IsLoading())
	{
		BindOwner(nullptr);
	}
}

bool UToroBehaviorRefLibrary::IsBehaviorRefInvalid(const FToroBehaviorRef& BehaviorRef)
{
	return BehaviorRef.IsInvalid();
}

UToroBehaviorComponent* UToroBehaviorRefLibrary::FindBehaviorRef(
	const FToroBehaviorRef& BehaviorRef, TSubclassOf<UToroBehaviorComponent> InClass)
{
	return BehaviorRef.CanResolveAs(InClass, true) ? BehaviorRef.Find() : nullptr;
}

UToroBehaviorComponent* UToroBehaviorRefLibrary::GetBehaviorRef(
	FToroBehaviorRef& BehaviorRef, TSubclassOf<UToroBehaviorComponent> InClass)
{
	return BehaviorRef.CanResolveAs(InClass, true) ? BehaviorRef.Get() : nullptr;
}

void UToroBehaviorRefLibrary::ResetBehaviorRef(FToroBehaviorRef& BehaviorRef)
{
	BehaviorRef.Reset();
}

void UToroBehaviorRefLibrary::BindBehaviorRefOwner(FToroBehaviorRef& BehaviorRef, const AActor* InActor)
{
	BehaviorRef.BindOwner(InActor);
}
