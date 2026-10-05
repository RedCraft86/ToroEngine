// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Libraries/ToroClassLibrary.h"
#include "UObject/UObjectHash.h"

const UObject* UToroClassLibrary::GetClassDefaultObject(const TSubclassOf<UObject> InClass)
{
	return IsValid(InClass) ? InClass->GetDefaultObject() : nullptr;
}

void UToroClassLibrary::GetDerivedClasses(TArray<UClass*>& Results, const TSubclassOf<UObject> InClass, const bool bRecursive)
{
	Results.Reset();
	::GetDerivedClasses(InClass, Results, bRecursive);
}
