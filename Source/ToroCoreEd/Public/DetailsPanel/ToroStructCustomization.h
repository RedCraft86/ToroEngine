// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToroCoreEd.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyEditorModule.h"
#include "PropertyHandle.h"

/**
 * Gets a reflected struct member handle from the current customization callback. Requires its named builder/handle variable.
 */
#define GET_STRUCT_PROPERTY(Struct, Member) \
	StructHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(Struct, Member))

/**
 * Gets a member handle using the caller-defined STRUCT_NAME type alias.
 */
#define GET_STRUCT_PROPERTY_NS(Member) \
	GET_STRUCT_PROPERTY(STRUCT_NAME, Member)

/**
 * Declares a member handle and hides its default row; the member must resolve to a valid property handle.
 */
#define GET_STRUCT_PROPERTY_VAR(Struct, Member, VarName) \
	TSharedPtr<IPropertyHandle> VarName = GET_STRUCT_PROPERTY(Struct, Member); \
	VarName->MarkHiddenByCustomization();

/**
 * Declares and hides a member handle using STRUCT_NAME; the member must exist.
 */
#define GET_STRUCT_PROPERTY_VAR_NS(Member, VarName) \
	GET_STRUCT_PROPERTY_VAR(STRUCT_NAME, Member, VarName)

/**
 * Base for registered struct detail customizations and parent-to-child metadata forwarding.
 * Derived header/children callbacks must call the base callback or assign WeakStructHandle
 * before using ForwardMetadata. The base callbacks add no widgets.
 */
class TOROCOREED_API FToroStructCustomization : public IPropertyTypeCustomization
{
	static inline TSet<FName> StructNames = {};

public:

	/**
	 * Registers a property-type customization factory and tracks it for cleanup.
	 * Duplicate registrations tracked by this helper are logged and ignored.
	 * @tparam Struct Reflected struct providing StaticStruct().
	 * @tparam Customization Default-constructible type derived from FToroStructCustomization.
	 */
	template <typename Struct, typename Customization>
	static void Register()
	{
		static_assert(TModels<CStaticStructProvider, Struct>::Value,
			"Struct must be a USTRUCT");

		static_assert(TIsDerivedFrom<Customization, FToroStructCustomization>::Value,
			"Customization must derive from FToroStructCustomization");

		const FName StructName = Struct::StaticStruct()->GetFName();
		FPropertyEditorModule* ModulePtr = FModuleManager::LoadModulePtr<FPropertyEditorModule>(TEXT("PropertyEditor"));
		if (ModulePtr && !StructName.IsNone())
		{
			if (StructNames.Contains(StructName))
			{
				UE_LOG(LogToroCoreEd, Warning,
					TEXT("FToroStructCustomization: Attempting to register multiple Struct Customizations for %s"), *StructName.ToString()
				);
				return;
			}

			StructNames.Add(StructName);
			ModulePtr->RegisterCustomPropertyTypeLayout(StructName,
				FOnGetPropertyTypeCustomizationInstance::CreateLambda([]() -> TSharedRef<IPropertyTypeCustomization>
				{
					return MakeShared<Customization>();
				})
			);

			UE_LOG(LogToroCoreEd, Display, TEXT("FToroStructCustomization: Registering Struct customization for %s"), *StructName.ToString());
		}
	}

	/**
	 * Unregisters tracked property-type layouts when PropertyEditor is loaded and clears local tracking.
	 */
	static void UnregisterAll()
	{
		FPropertyEditorModule* ModulePtr = FModuleManager::GetModulePtr<FPropertyEditorModule>(TEXT("PropertyEditor"));
		if (ModulePtr && !StructNames.IsEmpty())
		{
			for (const FName& StructName : StructNames)
			{
				ModulePtr->UnregisterCustomPropertyTypeLayout(StructName);
			}

			UE_LOG(LogToroCoreEd, Display, TEXT("FToroStructCustomization: Unregistered Struct Customizations"));
		}

		StructNames.Empty();
	}

protected:

	/** Source property handle whose reflected metadata is forwarded to child handles. */
	TWeakPtr<IPropertyHandle> WeakStructHandle;

	/**
	 * Copies the source property's reflected metadata into the destination's instance metadata.
	 * Invalid or expired handles cause no changes. Gameplay-tag struct destinations are unsupported
	 * and log an error; use a custom SGameplayTagCombo instead. Existing matching keys are overwritten.
	 * @param Property Destination property handle, normally a child of the customized struct.
	 */
	void ForwardMetadata(const TSharedPtr<IPropertyHandle>& Property) const;

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle, IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
};
