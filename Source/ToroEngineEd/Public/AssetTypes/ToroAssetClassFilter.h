// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ClassViewerFilter.h"

class TOROENGINEED_API FToroAssetClassFilter : public IClassViewerFilter
{
protected:

	bool bDisallowBlueprint;
	TSet<const UClass*> AllowedBaseClasses;
	EClassFlags DisallowedClassFlags;

public:

	static constexpr EClassFlags BadClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown;

	FToroAssetClassFilter(const UClass* AllowedClass)
		: bDisallowBlueprint(true)
		, AllowedBaseClasses({AllowedClass})
		, DisallowedClassFlags(BadClassFlags)
	{}

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions,
		const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override;

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions,
		const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData,
		TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override;
};
