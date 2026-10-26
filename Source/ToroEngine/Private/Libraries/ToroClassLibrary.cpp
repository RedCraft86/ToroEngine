// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroClassLibrary.h"
#include "UObject/UObjectHash.h"
#include "UObject/Class.h"

const UObject* UToroClassLibrary::GetClassDefaultObject(TSubclassOf<UObject> InClass)
{
	return IsValid(InClass) ? InClass->GetDefaultObject() : nullptr;
}

UClass* UToroClassLibrary::GetClassParent(TSubclassOf<UObject> InClass)
{
	return IsValid(InClass) ? InClass->GetSuperClass() : nullptr;
}

void UToroClassLibrary::GetClassParentHierarchy(TArray<UClass*>& Results, TSubclassOf<UObject> InClass)
{
	if (!IsValid(InClass))
	{
		Results.Empty();
		return;
	}

	Results.Reset();
	for (UClass* Class = InClass->GetSuperClass(); Class; Class = Class->GetSuperClass())
	{
		if (IsValid(Class))
		{
			Results.Add(Class);
		}
	}
}

void UToroClassLibrary::GetDerivedClasses(TArray<UClass*>& Results, TSubclassOf<UObject> InClass, bool bRecursive)
{
	Results.Reset();
	::GetDerivedClasses(InClass, Results, bRecursive);
}
