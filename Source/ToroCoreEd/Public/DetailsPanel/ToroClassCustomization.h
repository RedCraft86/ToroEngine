// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToroCoreEd.h"
#include "DetailLayoutBuilder.h"
#include "IDetailCustomization.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyEditorModule.h"
#include "PropertyHandle.h"

/**
 * Gets a reflected class member handle from the current customization callback. Requires its named builder/handle variable.
 */
#define GET_CLASS_PROPERTY(Class, Member) \
	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(Class, Member))

/**
 * Gets a member handle using the caller-defined CLASS_NAME type alias.
 */
#define GET_CLASS_PROPERTY_NS(Member) \
	GET_CLASS_PROPERTY(CLASS_NAME, Member)

/**
 * Declares a member handle and hides its default row; the member must resolve to a valid property handle.
 */
#define GET_CLASS_PROPERTY_VAR(Class, Member, VarName) \
	TSharedRef<IPropertyHandle> VarName = GET_CLASS_PROPERTY(Class, Member); \
	VarName->MarkHiddenByCustomization();

/**
 * Declares and hides a member handle using CLASS_NAME; the member must exist.
 */
#define GET_CLASS_PROPERTY_VAR_NS(Member, VarName) \
	GET_CLASS_PROPERTY_VAR(CLASS_NAME, Member, VarName)

/**
 * Base for class detail customizations with registration and category metadata support.
 * Requires a non-empty selection of one exact class; templates are skipped by default.
 * Parses RenameCategories and PrioritizeCategories metadata, filters category visibility,
 * then applies surviving settings before invoking the derived customization.
 * Derived helpers can adjust those settings.
 */
class TOROCOREED_API FToroClassCustomization : public IDetailCustomization
{
	static inline TSet<FName> ClassNames = {};

public:

	/**
	 * Registers a detail-customization factory with PropertyEditor and tracks it for cleanup.
	 * Duplicate registrations tracked by this helper are logged and ignored.
	 * @tparam Class Reflected UObject class to customize.
	 * @tparam Customization Default-constructible type derived from FToroClassCustomization.
	 */
	template <typename Class, typename Customization>
	static void Register()
	{
		static_assert(TModels<CStaticClassProvider, Class>::Value && TIsDerivedFrom<Class, UObject>::Value,
			"Class must be a UCLASS that derive from UObject");

		static_assert(TIsDerivedFrom<Customization, FToroClassCustomization>::Value,
			"Customization must derive from FToroClassCustomization");

		const FName ClassName = Class::StaticClass()->GetFName();
		FPropertyEditorModule* ModulePtr = FModuleManager::LoadModulePtr<FPropertyEditorModule>(TEXT("PropertyEditor"));
		if (ModulePtr && !ClassName.IsNone())
		{
			if (ClassNames.Contains(ClassName))
			{
				UE_LOG(LogToroCoreEd, Warning,
					TEXT("FToroClassCustomization: Attempting to register multiple Class Customizations for %s"), *ClassName.ToString()
				);
				return;
			}

			ClassNames.Add(ClassName);
			ModulePtr->RegisterCustomClassLayout(ClassName,
				FOnGetDetailCustomizationInstance::CreateLambda([]() -> TSharedRef<IDetailCustomization>
				{
					return MakeShared<Customization>();
				})
			);

			UE_LOG(LogToroCoreEd, Display, TEXT("FToroClassCustomization: Registering Class customization for %s"), *ClassName.ToString());
		}
	}

	/**
	 * Unregisters tracked class layouts when PropertyEditor is loaded and clears local tracking.
	 */
	static void UnregisterAll()
	{
		FPropertyEditorModule* ModulePtr = FModuleManager::GetModulePtr<FPropertyEditorModule>(TEXT("PropertyEditor"));
		if (ModulePtr && !ClassNames.IsEmpty())
		{
			for (const FName& ClassName : ClassNames)
			{
				ModulePtr->UnregisterCustomClassLayout(ClassName);
			}

			UE_LOG(LogToroCoreEd, Display, TEXT("FToroClassCustomization: Unregistered Class Customizations"));
		}

		ClassNames.Empty();
	}

protected:

	/**
	 * Display settings shared by metadata processing and derived category helpers.
	 */
	struct FToroCategoryInfo
	{
		/** Optional category display label; an empty label preserves the engine default. */
		FText DisplayName;

		/** Category ordering group used when editing the category. */
		ECategoryPriority::Type Priority;

		FToroCategoryInfo()
			: DisplayName(FText::GetEmpty()), Priority(ECategoryPriority::Default)
		{}
	};

	/** Allows class default objects and other templates when enabled by a derived customization. */
	bool bCustomizeTemplate = false;

	/** Exact class shared by the accepted selection; null when selection validation fails. */
	UClass* CustomizingClass = nullptr;

	/** Builder for the accepted customization pass, retained without extending its lifetime. */
	TWeakPtr<IDetailLayoutBuilder> WeakBuilder;

	/** Category display settings for the current pass; filtered categories are removed before application. */
	TMap<FName, FToroCategoryInfo> CategoryMap;

	/**
	 * Edits a category using its stored label and priority, or the engine defaults.
	 * @param CategoryName Internal category name, including any subcategory path.
	 * @return Category builder, or nullptr when no live detail builder is available.
	 */
	IDetailCategoryBuilder* FindOrAddCategory(FName CategoryName);

	/**
	 * Stores and applies a category label while preserving its configured priority.
	 * @param CategoryName Internal category name.
	 * @param DisplayName Display label; empty uses the engine default.
	 * @return Category builder, or nullptr when no live detail builder is available.
	 */
	IDetailCategoryBuilder* SetCategoryDisplayName(FName CategoryName, const FText& DisplayName);

	/**
	 * Stores and applies a category priority while preserving its configured label.
	 * @param CategoryName Internal category name.
	 * @param Priority Ordering group for the category.
	 * @return Category builder, or nullptr when no live detail builder is available.
	 */
	IDetailCategoryBuilder* SetCategoryPriority(FName CategoryName, ECategoryPriority::Type Priority);

	/**
	 * Provides categories kept when visibility uses an allow list; empty adds no restrictions.
	 * A non-empty list or HideCategories="*" activates allow-list filtering. Entries also match
	 * child categories separated by '|'. Force-show and show metadata take precedence over hide
	 * metadata, which takes precedence over this list.
	 */
	virtual TArray<FString> GetDefaultCategories();

	/**
	 * Provides categories exempt from this helper's visibility filtering.
	 * Defaults to Transform and TransformCommon. Entries also match child categories separated
	 * by '|'; this exemption does not undo categories already hidden by the detail builder.
	 */
	virtual TArray<FString> GetForceShowCategories();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override = 0;

private:

	void HandleCategoryRenames();
	void HandleCategoryPriority();
	void HandleCategoryVisibility();
	virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) override;
};
