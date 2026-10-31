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
 * Base for class Details customizations with shared registration and selection validation.
 * Requires a nonempty selection of valid objects of one exact class; mixed-class selections are rejected.
 * Templates are excluded unless bCustomizeTemplate is enabled; mixed template/instance selections are always rejected.
 * The shared-builder callback resets cached state, validates the selection, and prepares accepted
 * selection state before dispatching to the derived reference-based CustomizeDetails() implementation.
 * IsPrimaryCustomization() identifies the most-derived tracked registration in the selected class's
 * ancestry. It does not gate dispatch, enforce execution order, or process category metadata or visibility.
 * Register on the editor game thread and call UnregisterAll() before unloading customization code.
 */
class TOROENGINEED_API FToroClassCustomization : public IDetailCustomization
{
	/** Class names tracked for duplicate detection, primary-registration lookup, and shutdown cleanup. */
	static inline TSet<FName> RegisteredEntries = {};

public:

	/**
	 * Loads PropertyEditor if needed and registers a factory that constructs a new customization per request.
	 * The factory assigns Class's reflected name to RegisteredClassName to identify the instance's registration.
	 * Duplicate detection and primary-registration lookup cover this helper's registrations, not other modules'.
	 * @tparam Customization Concrete FToroClassCustomization subclass constructible without arguments by MakeShared().
	 * @tparam Class Reflected UObject subclass exposing StaticClass().
	 * @return Registered class name, or NAME_None if module loading fails, the name is invalid,
	 *         or this helper already tracks a customization for that class name.
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

	/** Allows homogeneous template selections, including class defaults and archetypes, when true; disabled by default. */
	bool bCustomizeTemplate = false;

	/**
	 * Reflected class name assigned by the registration factory for this instance.
	 * Identifies its registration rather than the selected object's actual class; defaults to NAME_None.
	 */
	FName RegisteredClassName = NAME_None;

	/** Non-owning class shared by all accepted objects; reset to null before each customization pass. */
	UClass* CustomizingClass = nullptr;

	/**
	 * Whether the accepted selection consists of templates, including class defaults and archetypes.
	 * Reset to false before each customization pass and remains false when the selection is rejected.
	 */
	bool bIsTemplateObject = false;

	/** Accepted layout builder held without extending its lifetime; reset before each customization pass. */
	TWeakPtr<IDetailLayoutBuilder> WeakBuilder;

	/**
	 * Determines whether this instance is the primary customization for the accepted selection.
	 * The most-derived registered class in the selected class's ancestry takes precedence.
	 * This is a query only: it does not suppress other customizations or perform shared processing.
	 * @return True if that registration matches this instance's RegisteredClassName.
	 *         False if the selection is unset or no matching registration exists.
	 */
	bool IsPrimaryCustomization() const;

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override = 0;

private:

	virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) override;
};
