// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroComponentLibrary.h"
#include "Components/ActorComponent.h"
#include "Utilities/ToroUtilities.h"
#include "UObject/UObjectIterator.h"
#include "Engine/World.h"

bool UToroComponentLibrary::GetAllowAnyoneToDestroyMe(const UActorComponent* Target)
{
	return IsValid(Target) && Target->bAllowAnyoneToDestroyMe;
}

void UToroComponentLibrary::SetAllowAnyoneToDestroyMe(UActorComponent* Target, bool bAllow)
{
	if (IsValid(Target))
	{
		Target->bAllowAnyoneToDestroyMe = bAllow;
	}
}

void UToroComponentLibrary::GetAllComponentsOfClass(const UObject* ContextObject,
	TSubclassOf<UActorComponent> InClass, TArray<UActorComponent*>& Result)
{
	Result.Reset();

	const UWorld* World = ToroEngine::GetWorld(ContextObject);
	if (IsValid(InClass) && IsValid(World))
	{
		for (TObjectIterator<UActorComponent> It(RF_ClassDefaultObject, true, EInternalObjectFlags::Garbage); It; ++It)
		{
			UActorComponent* Component = *It;
			if (IsValid(Component) && Component->GetWorld() == World && Component->IsA(InClass))
			{
				Result.Add(Component);
			}
		}
	}
}

UActorComponent* UToroComponentLibrary::GetComponentOfClass(const UObject* ContextObject, TSubclassOf<UActorComponent> InClass)
{
	const UWorld* World = ToroEngine::GetWorld(ContextObject);
	if (IsValid(InClass) && IsValid(World))
	{
		for (TObjectIterator<UActorComponent> It(RF_ClassDefaultObject, true, EInternalObjectFlags::Garbage); It; ++It)
		{
			UActorComponent* Component = *It;
			if (IsValid(Component) && Component->GetWorld() == World && Component->IsA(InClass))
			{
				return Component;
			}
		}
	}

	return nullptr;
}
