// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ClassViewerFilter.h"

/**
 * Class-picker filter for asset-creatable subclasses of a supplied base.
 * Defaults to native classes only and excludes abstract, deprecated, superseded,
 * and dropdown-hidden classes. Derived filters can adjust the protected restrictions.
 */
class TOROCOREED_API FToroAssetClassFilter : public IClassViewerFilter
{
protected:

	/** Rejects generated Blueprint classes and unloaded Blueprint candidates when enabled. */
	bool bDisallowBlueprint;

	/** Base classes accepted by the class-viewer inheritance filter. */
	TSet<const UClass*> AllowedBaseClasses;

	/** Rejects a candidate when any of these class flags are set. */
	EClassFlags DisallowedClassFlags;

public:

	static constexpr EClassFlags BadClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown;

	/**
	 * Initializes the native-only asset filter beneath a base class.
	 * @param AllowedClass Valid base class whose eligible descendants may be selected.
	 */
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
