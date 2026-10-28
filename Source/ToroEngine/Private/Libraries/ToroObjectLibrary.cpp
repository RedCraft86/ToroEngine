// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroObjectLibrary.h"
#include "Utilities/ToroLogging.h"
#include "UObject/UObjectHash.h"
#include "UObject/Class.h"
#include "Engine/World.h"
#include "Stats/Stats.h"

bool UToroObjectLibrary::CallObjectEvent(UObject* Target, FName EventName)
{
	if (!IsValid(Target) || EventName.IsNone())
	{
		return false;
	}

	UFunction* Function = Target->FindFunction(EventName);
	if (!Function || Function->NumParms != 0)
	{
		TORO_REPORT_ERROR(
			TEXT("Function %s must not have any parameters to be called via CallObjectEvent."),
			*EventName.ToString()
		);
		return false;
	}

	Target->ProcessEvent(Function, nullptr);
	return true;
}

bool UToroObjectLibrary::IsTemplateObject(UObject* Target)
{
	return IsValid(Target) && Target->IsTemplate();
}

bool UToroObjectLibrary::IsObjectBeingDestroyed(UObject* Target)
{
	return Target && Target->HasAnyFlags(RF_BeginDestroyed);
}

bool UToroObjectLibrary::HasObjectBeenDestroyed(UObject* Target)
{
	return Target && Target->HasAnyFlags(RF_FinishDestroyed);
}

void UToroObjectLibrary::GetAllObjectsOfClass(TArray<UObject*>& Result, TSubclassOf<UObject> InClass, const UWorld* WorldFilter)
{
	QUICK_SCOPE_CYCLE_COUNTER(UToroObjectLibrary_GetAllObjectsOfClass);
	Result.Reset();

	if (InClass == UObject::StaticClass())
	{
		TORO_REPORT_ERROR(
			TEXT("Called with UObject as the class. This is not allowed as it will cause severe performance issues.")
		);
		return;
	}

	if (IsValid(InClass))
	{
		GetObjectsOfClass(InClass, Result, true, RF_ClassDefaultObject, EInternalObjectFlags::Garbage);
		Result.RemoveAll([WorldFilter](const UObject* Element)
		{
			return !IsValid(Element) || (IsValid(WorldFilter) && Element->GetWorld() != WorldFilter);
		});
	}
}

UObject* UToroObjectLibrary::GetObjectOfClass(TSubclassOf<UObject> InClass, const UWorld* WorldFilter)
{
	QUICK_SCOPE_CYCLE_COUNTER(UToroObjectLibrary_GetObjectOfClass);

	if (InClass == UObject::StaticClass())
	{
		TORO_REPORT_ERROR(
			TEXT("Called with UObject as the class. This is not allowed as it will cause severe performance issues.")
		);
		return nullptr;
	}

	if (IsValid(InClass))
	{
		TArray<UObject*> Objects;
		GetObjectsOfClass(InClass, Objects);
		for (UObject* Object : Objects)
		{
			if (IsValid(Object) && (!IsValid(WorldFilter) || Object->GetWorld() == WorldFilter))
			{
				return Object;
			}
		}
	}

	return nullptr;
}
