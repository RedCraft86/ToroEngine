// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ClassViewerFilter.h"
#include "UObject/ObjectMacros.h"
#include "Containers/Set.h"

/**
 * Class Viewer filter for selecting asset classes from permitted base-class hierarchies.
 * Defaults to excluding Blueprint classes and classes with BadClassFlags.
 * Loaded candidates must also pass UClass::CanCreateAssetOfClass().
 * Derived filters can adjust the protected restrictions before use.
 */
class TOROENGINEED_API FToroAssetClassFilter : public IClassViewerFilter
{
protected:

	/**
	 * Rejects loaded Blueprint-generated classes and all unloaded Blueprint candidates when true.
	 * Defaults to true; disabling this restriction still applies ancestry and class-flag checks.
	 */
	bool bDisallowBlueprint;

	/**
	 * Non-owning base-class pointers; candidates may match any listed base or one of its descendants.
	 * An empty set removes the ancestry restriction. Entries must remain valid while the filter is used.
	 */
	TSet<const UClass*> AllowedBaseClasses;

	/** Rejects candidates carrying any of these flags; initialized from BadClassFlags. */
	EClassFlags DisallowedClassFlags;

public:

	/**
	 * Default exclusions: abstract, deprecated, superseded, and hidden-from-dropdown classes.
	 * Also shared by the asset factory when checking whether a selected class can be instantiated.
	 */
	static constexpr EClassFlags BadClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown;

	/**
	 * Restricts selection to a base-class hierarchy, excluding Blueprint classes and BadClassFlags.
	 * @param AllowedClass Non-null base class whose eligible descendants, including itself, may be selected.
	 * The pointer is not retained through garbage collection and must remain valid while the filter is used.
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
