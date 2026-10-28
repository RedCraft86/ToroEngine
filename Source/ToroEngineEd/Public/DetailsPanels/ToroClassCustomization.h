// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DetailLayoutBuilder.h"
#include "Utilities/ToroLogging.h"
#include "IDetailCustomization.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Internationalization/Text.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Concepts/StaticClassProvider.h"
#include "Misc/AssertionMacros.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Templates/UnrealTypeTraits.h"
#include "Templates/SharedPointer.h"
#include "Templates/Models.h"
#include "UObject/Object.h"
#include "UObject/Class.h"
#include "Containers/Set.h"
#include "Containers/Map.h"
#include "PropertyHandle.h"

/**
 * Looks up a property using a checked C++ member name and an accessible DetailBuilder reference.
 * @param Class C++ class type containing Member.
 * @param Member Member whose property handle is requested; it must be reflected to yield a valid handle.
 * @return Non-null property-handle reference, which may represent an invalid property.
 */
#define GET_CLASS_PROPERTY(Class, Member) \
	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(Class, Member))

/**
 * Looks up a property using CLASS_NAME as the C++ class type.
 * Requires CLASS_NAME and an accessible DetailBuilder reference.
 * @param Member Member whose property handle is requested.
 * @return Non-null property-handle reference, which may represent an invalid property.
 */
#define GET_CLASS_PROPERTY_NS(Member) \
	GET_CLASS_PROPERTY(CLASS_NAME, Member)

/**
 * Declares a property-handle reference and hides the property from automatic layout when valid.
 * Requires an accessible DetailBuilder reference; callers must check IsValidHandle() before further use.
 * Expands to multiple statements; use as a standalone statement, not an unbraced conditional body.
 * @param Class C++ class type containing Member.
 * @param Member Member whose property handle is requested.
 * @param VarName Name of the TSharedRef<IPropertyHandle> variable declared in the current scope.
 */
#define GET_CLASS_PROPERTY_VAR(Class, Member, VarName) \
	TSharedRef<IPropertyHandle> VarName = GET_CLASS_PROPERTY(Class, Member); \
	if (VarName->IsValidHandle()) \
	{ \
		VarName->MarkHiddenByCustomization(); \
	}

/**
 * Declares and conditionally hides a property handle using CLASS_NAME as the C++ class type.
 * Requires CLASS_NAME and an accessible DetailBuilder reference; the resulting handle may be invalid.
 * Expands to multiple statements and must be used as a standalone statement.
 * @param Member Member whose property handle is requested.
 * @param VarName Name of the property-handle reference declared in the current scope.
 */
#define GET_CLASS_PROPERTY_VAR_NS(Member, VarName) \
	GET_CLASS_PROPERTY_VAR(CLASS_NAME, Member, VarName)

/**
 * Base for class Details customizations with shared registration and category metadata processing.
 * Accepts valid objects of one exact class; mixed-class selections are rejected.
 * Templates are excluded unless bCustomizeTemplate is enabled; mixed template/instance selections are always rejected.
 * The shared-builder callback prepares selection state for each instance. The most-derived class
 * registered through this helper in the selected class's ancestry owns automatic category metadata
 * and visibility processing, performed before its reference-based CustomizeDetails() implementation.
 * Other registered instances still dispatch their UI customization; ownership does not establish
 * execution order or guarantee a single invocation per layout.
 * Register on the editor game thread and call UnregisterAll() before unloading customization code.
 */
class TOROENGINEED_API FToroClassCustomization : public IDetailCustomization
{
	/** Class names tracked for duplicate detection, category ownership selection, and shutdown cleanup. */
	static inline TSet<FName> RegisteredEntries = {};

public:

	/**
	 * Loads PropertyEditor if needed and registers a factory that constructs a new customization per request.
	 * The factory assigns Class's reflected name to RegisteredClassName to identify category ownership.
	 * Duplicate detection and ownership selection cover this helper's registrations, not other modules'.
	 * @tparam Customization Concrete FToroClassCustomization subclass constructible without arguments by MakeShared().
	 * @tparam Class Reflected UObject subclass exposing StaticClass().
	 * @return Registered class name, or NAME_None if module loading fails, the name is invalid,
	 * or this helper already tracks a customization for that class name.
	 */
	template<typename Customization, typename Class>
	static FName Register()
	{
		static_assert(TIsDerivedFrom<Customization, FToroClassCustomization>::Value,
			"Customization must derive from FToroClassCustomization");

		static_assert(TModels<CStaticClassProvider, Class>::Value && TIsDerivedFrom<Class, UObject>::Value,
			"Class must be a UCLASS that derive from UObject");

		FPropertyEditorModule* ModulePtr = FModuleManager::LoadModulePtr<FPropertyEditorModule>(TEXT("PropertyEditor"));
		if (!ModulePtr)
		{
			TORO_LOG_WARNING(TEXT("Attempting to register while PropertyEditor module is not loaded."));
			return NAME_None;
		}

		const FName EntryName = Class::StaticClass()->GetFName();
		if (EntryName.IsNone())
		{
			TORO_LOG_WARNING(TEXT("Attempting to register with an invalid class"));
			return NAME_None;
		}

		if (RegisteredEntries.Contains(EntryName))
		{
			TORO_LOG_WARNING(TEXT("Attempting to register multiple customizations for %s"), *EntryName.ToString());
			return NAME_None;
		}

		RegisteredEntries.Add(EntryName);
		ModulePtr->RegisterCustomClassLayout(EntryName,
			FOnGetDetailCustomizationInstance::CreateLambda([EntryName]() -> TSharedRef<IDetailCustomization>
				{
					TSharedRef<Customization> Instance = MakeShared<Customization>();
					Instance->RegisteredClassName = EntryName;
					return Instance;
				})
		);

		TORO_LOG_INFO(TEXT("Registered customization for %s"), *EntryName.ToString());
		return EntryName;
	}

	/**
	 * Unregisters tracked class layouts if PropertyEditor is still loaded, then clears local tracking.
	 * @note Automatically called by ToroEngineEd -> Module Shutdown.
	 */
	static void UnregisterAll()
	{
		FPropertyEditorModule* ModulePtr = FModuleManager::GetModulePtr<FPropertyEditorModule>(TEXT("PropertyEditor"));
		if (ModulePtr && !RegisteredEntries.IsEmpty())
		{
			for (const FName& Entry : RegisteredEntries)
			{
				ModulePtr->UnregisterCustomClassLayout(Entry);
			}

			TORO_LOG_INFO(TEXT("Unregistered all class customizations"));
		}

		RegisteredEntries.Empty();
	}

protected:

	/** Display label and priority overrides associated with an internal category name. */
	struct FToroCategoryInfo
	{
		/** Optional display label; an empty value leaves label selection to the Details builder. */
		FText DisplayName;

		/** Sorting priority passed to the Details builder when editing the category. */
		ECategoryPriority::Type Priority;

		/** Starts with no display-name override and the default category priority. */
		FToroCategoryInfo()
			: DisplayName(FText::GetEmpty())
			, Priority(ECategoryPriority::Default)
		{}
	};

	/** Allows homogeneous template selections, including class defaults and archetypes, when true; disabled by default. */
	bool bCustomizeTemplate = false;

	/**
	 * Reflected class name assigned by the registration factory for this instance.
	 * Identifies its registration rather than the selected object's actual class; defaults to NAME_None.
	 */
	FName RegisteredClassName = NAME_None;

	/** Non-owning class of the accepted selection, reset before each customization pass. */
	UClass* CustomizingClass = nullptr;

	/**
	 * Whether the accepted selection consists of templates, including class defaults and archetypes.
	 * Reset to false before each customization pass and remains false when the selection is rejected.
	 */
	bool bIsTemplateObject = false;

	/** Accepted layout builder held without extending its lifetime. */
	TWeakPtr<IDetailLayoutBuilder> WeakBuilder;

	/** Category overrides for the current pass, keyed by internal name and cleared before each pass. */
	TMap<FName, FToroCategoryInfo> CategoryMap;

	/**
	 * Edits or creates a category, applying stored display-name and priority overrides when present.
	 * @param CategoryName Internal category name.
	 * @return Reference to the category.
	 */
	IDetailCategoryBuilder& FindOrAddCategory(FName CategoryName);

	/**
	 * Records a category display label and edits the category if the layout builder is available.
	 * The internal category name is unchanged.
	 * @param CategoryName Internal category name.
	 * @param DisplayName User-visible category label.
	 * @return Reference to the category.
	 */
	IDetailCategoryBuilder& SetCategoryDisplayName(FName CategoryName, const FText& DisplayName);

	/**
	 * Records a category priority and edits the category if the layout builder is available.
	 * @param CategoryName Internal category name.
	 * @param Priority Sorting priority supplied to the Details builder.
	 * @return Reference to the category.
	 */
	IDetailCategoryBuilder& SetCategoryPriority(FName CategoryName, ECategoryPriority::Type Priority);

	/**
	 * Supplies categories retained by default when the visibility pass uses an allow list.
	 * The base implementation returns an empty array. A non-empty result or a hide-categories wildcard
	 * enables allow-list behavior; explicit hide entries beat defaults, while show and force-show entries win.
	 * Entries match the named category and its pipe-delimited descendants.
	 */
	virtual TArray<FString> GetDefaultCategories();

	/**
	 * Supplies categories preserved by this helper's visibility pass even when hide metadata matches.
	 * The base list contains Transform and TransformCommon.
	 * Entries also match pipe-delimited child categories.
	 */
	virtual TArray<FString> GetForceShowCategories();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override = 0;

private:

	/**
	 * Determines whether this instance owns automatic category processing for the accepted selection.
	 * Walks from CustomizingClass through its ancestors and stops at the first class in RegisteredEntries.
	 * @return True if that class matches RegisteredClassName; false if the selection is unset,
	 *         no tracked registration is found, or this instance belongs to another registration.
	 */
	bool ShouldManageCategories() const;

	/**
	 * Reads RenameCategories metadata as comma-separated Old=New pairs.
	 * Trims names, ignores incomplete entries, and records display labels without changing category keys.
	 */
	void HandleCategoryRenames();

	/**
	 * Records Important priority for the categories listed by the selected class's prioritization metadata.
	 */
	void HandleCategoryPriority();

	/**
	 * Hides existing categories using ForceShow, Show, Hide, then Default precedence.
	 * Matches include pipe-delimited descendants. A hide wildcard or non-empty default list
	 * enables allow-list behavior. Categories marked IsEmpty() are exempt from HideCategory().
	 * All categories selected by the hide rules lose their pending CategoryMap overrides,
	 * including empty categories that remain exempt from hiding.
	 */
	void HandleCategoryVisibility();

	virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) override;
};
