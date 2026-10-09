// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "AssetTypes/ToroAssetClassFilter.h"
#include "Engine/Blueprint.h"

bool FToroAssetClassFilter::IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions,
	const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs)
{
	if (InClass->HasAnyClassFlags(DisallowedClassFlags) || !InClass->CanCreateAssetOfClass()
		|| InFilterFuncs->IfInChildOfClassesSet(AllowedBaseClasses, InClass) == EFilterReturn::Failed)
	{
		return false;
	}

	if (bDisallowBlueprint && IsValid(Cast<UBlueprint>(InClass->ClassGeneratedBy)))
	{
		return false;
	}

	return true;
}

bool FToroAssetClassFilter::IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions,
	const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs)
{
	return !bDisallowBlueprint
		&& !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags)
		&& InFilterFuncs->IfInChildOfClassesSet(AllowedBaseClasses, InUnloadedClassData) != EFilterReturn::Failed;
}
