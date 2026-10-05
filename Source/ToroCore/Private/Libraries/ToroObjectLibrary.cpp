// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Libraries/ToroObjectLibrary.h"
#include "Misc/OutputDeviceNull.h"
#include "UObject/UObjectHash.h"
#include "ToroCore.h"

void UToroObjectLibrary::CallObjectEvent(UObject* Target, FName EventName)
{
	if (IsValid(Target) && !EventName.IsNone())
	{
		FOutputDeviceNull Ar;
		Target->CallFunctionByNameWithArguments(*EventName.ToString(), Ar, nullptr, true);
	}
}

void UToroObjectLibrary::GetAllObjectsOfClass(TArray<UObject*>& Result, TSubclassOf<UObject> InClass)
{
	QUICK_SCOPE_CYCLE_COUNTER(UToroObjectLibrary_GetAllObjectsOfClass);
	Result.Reset();

	if (InClass == UObject::StaticClass())
	{
		UE_LOG(LogToroCore, Warning,
			TEXT("UToroObjectLibrary::GetAllObjectsOfClass called with UObject as the class. This includes EVERY active object. Call cancelled.")
		);

		return;
	}

	if (IsValid(InClass))
	{
		GetObjectsOfClass(InClass, Result);
	}
}

UObject* UToroObjectLibrary::GetObjectOfClass(TSubclassOf<UObject> InClass)
{
	QUICK_SCOPE_CYCLE_COUNTER(UToroObjectLibrary_GetObjectOfClass);

	if (InClass == UObject::StaticClass())
	{
		UE_LOG(LogToroCore, Warning,
			TEXT("UToroObjectLibrary::GetObjectOfClass called with UObject as the class. This includes EVERY active object. Call cancelled.")
		);

		return nullptr;
	}

	if (IsValid(InClass))
	{
		TArray<UObject*> Objects;
		GetObjectsOfClass(InClass, Objects);
		for (UObject* Object : Objects)
		{
			if (IsValid(Object))
			{
				return Object;
			}
		}
	}

	return nullptr;
}
