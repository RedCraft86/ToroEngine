// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "IPropertyTypeCustomization.h"
#include "Concepts/StaticStructProvider.h"
#include "PropertyCustomizationHelpers.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Templates/UnrealTypeTraits.h"
#include "Templates/SharedPointer.h"
#include "Templates/Models.h"
#include "UObject/UnrealType.h"
#include "UObject/Class.h"
#include "Containers/Set.h"
#include "PropertyHandle.h"
#include "ToroEngineEd.h"

/**
 * Looks up a reflected child by its checked C++ member name.
 * Requires an accessible, valid StructHandle supporting GetChildHandle(); a C++ member need not be reflected.
 * @param Struct C++ struct type containing Member.
 * @param Member Member whose property handle is requested.
 * @return Child handle, which may be null or invalid when the reflected property is unavailable.
 */
#define GET_STRUCT_PROPERTY(Struct, Member) \
	StructHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(Struct, Member))

/**
 * Looks up a child using STRUCT_NAME as the C++ struct type.
 * Requires STRUCT_NAME to name the struct and an accessible, valid StructHandle.
 * @param Member Member whose property handle is requested.
 * @return Child handle, which may be null or invalid.
 */
#define GET_STRUCT_PROPERTY_NS(Member) \
	GET_STRUCT_PROPERTY(STRUCT_NAME, Member)

/**
 * Declares a child-handle variable and hides the child from automatic layout when its handle is valid.
 * Requires an accessible, valid StructHandle. Callers must check the resulting handle before using it.
 * Expands to multiple statements; use as a standalone statement, not an unbraced conditional body.
 * @param Struct C++ struct type containing Member.
 * @param Member Member whose property handle is requested.
 * @param VarName Name of the TSharedPtr<IPropertyHandle> variable declared in the current scope.
 */
#define GET_STRUCT_PROPERTY_VAR(Struct, Member, VarName) \
	TSharedPtr<IPropertyHandle> VarName = GET_STRUCT_PROPERTY(Struct, Member); \
	if (VarName.IsValid() && VarName->IsValidHandle()) \
	{ \
		VarName->MarkHiddenByCustomization(); \
	}

/**
 * Declares and conditionally hides a child handle using STRUCT_NAME as the C++ struct type.
 * Requires STRUCT_NAME and an accessible, valid StructHandle; the resulting handle may be null or invalid.
 * Expands to multiple statements and must be used as a standalone statement.
 * @param Member Member whose property handle is requested.
 * @param VarName Name of the child-handle variable declared in the current scope.
 */
#define GET_STRUCT_PROPERTY_VAR_NS(Member, VarName) \
	GET_STRUCT_PROPERTY_VAR(STRUCT_NAME, Member, VarName)

/**
 * Base for reflected struct Details customizations with shared registration and metadata forwarding.
 * Base layout callbacks only cache the struct handle; derived customizations supply their own layout.
 * Call the corresponding base callback before ForwardMetadata() so the cached handle is available.
 * Register on the editor game thread and call UnregisterAll() during module shutdown.
 */
class TOROENGINEED_API FToroStructCustomization : public IPropertyTypeCustomization
{
	/** Struct names registered through this helper, tracked for duplicate detection and cleanup. */
	static inline TSet<FName> RegisteredEntries = {};

public:

	/**
	 * Loads PropertyEditor if necessary and registers a factory for a struct customization.
	 * Each factory invocation constructs a new customization instance for the requested layout.
	 * Duplicate detection covers registrations made through this helper, not other modules.
	 * @tparam Customization Concrete FToroStructCustomization subclass constructible without arguments by MakeShared().
	 * @tparam Struct Reflected struct type exposing StaticStruct().
	 * @return Registered struct name, or NAME_None if module loading fails, the name is invalid,
	 *         or this helper already tracks a customization for that name.
	 */
	template<typename Customization, typename Struct>
	static FName Register()
	{
		static_assert(TIsDerivedFrom<Customization, FToroStructCustomization>::Value,
			"Customization must derive from FToroStructCustomization");

		static_assert(TModels<CStaticStructProvider, Struct>::Value,
			"Struct must be a USTRUCT");

		FPropertyEditorModule* ModulePtr = FModuleManager::LoadModulePtr<FPropertyEditorModule>(TEXT("PropertyEditor"));
		if (!ModulePtr)
		{
			UE_LOG(LogToroEngineEd, Warning, TEXT("FToroStructCustomization: Attempting to register while PropertyEditor module is not loaded."))
			return NAME_None;
		}

		const FName EntryName = Struct::StaticStruct()->GetFName();
		if (EntryName.IsNone())
		{
			UE_LOG(LogToroEngineEd, Warning, TEXT("FToroStructCustomization: Attempting to register with an invalid struct"))
			return NAME_None;
		}

		if (RegisteredEntries.Contains(EntryName))
		{
			UE_LOG(LogToroEngineEd, Warning,
				TEXT("FToroStructCustomization: Attempting to register multiple customizations for %s"), *EntryName.ToString()
			);
			return NAME_None;
		}

		RegisteredEntries.Add(EntryName);
		ModulePtr->RegisterCustomPropertyTypeLayout(EntryName,
			FOnGetPropertyTypeCustomizationInstance::CreateLambda([]() -> TSharedRef<IPropertyTypeCustomization>
			{
				return MakeShared<Customization>();
			})
		);

		UE_LOG(LogToroEngineEd, Display, TEXT("FToroStructCustomization: Registered customization for %s"), *EntryName.ToString());
		return EntryName;
	}

	/**
	 * Removes tracked property-type layouts if PropertyEditor is still loaded, then clears local tracking.
	 * @note Automatically called by ToroEngineEd -> Module Shutdown.
	 */
	static void UnregisterAll()
	{
		FPropertyEditorModule* ModulePtr = FModuleManager::GetModulePtr<FPropertyEditorModule>(TEXT("PropertyEditor"));
		if (ModulePtr && !RegisteredEntries.IsEmpty())
		{
			for (const FName& Entry : RegisteredEntries)
			{
				ModulePtr->UnregisterCustomPropertyTypeLayout(Entry);
			}

			UE_LOG(LogToroEngineEd, Display, TEXT("FToroStructCustomization: Unregistered all struct customizations"));
		}

		RegisteredEntries.Empty();
	}

protected:

	/** Struct handle cached by the base layout callbacks without extending its lifetime. */
	TWeakPtr<IPropertyHandle> WeakStructHandle;

	/**
	 * Copies the cached struct property's FProperty metadata into a target handle's instance metadata.
	 * Matching target instance keys are overwritten; parent handle instance metadata is not copied.
	 * Does nothing if either handle is null, expired, or invalid, or the source property has no metadata.
	 * @param Property Handle receiving the copied instance metadata.
	 */
	void ForwardMetadata(const TSharedPtr<IPropertyHandle>& Property) const;

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle, IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
};
