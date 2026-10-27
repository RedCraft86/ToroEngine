// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DetailWidgetRow.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "SGameplayTagCombo.h"
#include "IPropertyUtilities.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "SInstancedStructPicker.h"
#include "InstancedStructDetails.h"
#include "DetailCategoryBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "DetailsPanels/ToroClassCustomization.h"
#include "DataAssets/ToroDatabase.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
#define CLASS_NAME UToroDatabase
class FToroDatabaseCustomization : public FToroClassCustomization
{
	class FEntryBuilder final : public FInstancedStructDataDetails
	{
	public:

		FEntryBuilder(
			int32 InIndex,
			const FString& InKeyFilter,
			const TSharedPtr<IPropertyHandle>& InRowProperty,
			const TSharedRef<FToroDatabaseCustomization> InCustomization
		)
			: FInstancedStructDataDetails(InRowProperty)
			, Index(InIndex)
			, KeyFilter(InKeyFilter)
			, KeyProperty(InRowProperty->GetKeyHandle())
			, WeakCustomization(InCustomization)
		{
		}

		bool IsValidBuilder() const
		{
			return !bDetached
				&& WeakCustomization.IsValid()
				&& StructProperty.IsValid()
				&& StructProperty->IsValidHandle()
				&& KeyProperty.IsValid()
				&& KeyProperty->IsValidHandle();
		}

		void DetachWidgets()
		{
			bDetached = true;
			if (KeyWidget.IsValid())
			{
				KeyWidget->SetContent(SNullWidget::NullWidget);
			}
			if (ValueWidget.IsValid())
			{
				ValueWidget->SetContent(SNullWidget::NullWidget);
			}
		}

		virtual FName GetName() const override
		{
			return *FString::Printf(TEXT("ToroDatabaseEntry_%d"), Index);
		}

		virtual TSharedPtr<IPropertyHandle> GetPropertyHandle() const override
		{
			return StructProperty;
		}

	private:

		int32 Index;
		FString KeyFilter;
		TSharedPtr<IPropertyHandle> KeyProperty;
		TWeakPtr<FToroDatabaseCustomization> WeakCustomization;
		TSharedPtr<SBox> KeyWidget, ValueWidget;
		bool bDetached = false;

		virtual void Tick(float DeltaTime) override
		{
			if (IsValidBuilder())
			{
				FInstancedStructDataDetails::Tick(DeltaTime);
			}
		}

		virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override
		{
			if (IsValidBuilder())
			{
				FInstancedStructDataDetails::GenerateChildContent(ChildrenBuilder);
			}
		}

		virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override
		{
			if (!IsValidBuilder())
			{
				return;
			}

			const TSharedPtr<FToroDatabaseCustomization> Customization = WeakCustomization.Pin();
			const TSharedPtr<IDetailLayoutBuilder> Builder = Customization->WeakBuilder.Pin();
			if (!Builder.IsValid())
			{
				return;
			}

			const TSharedPtr<IPropertyUtilities> Utilities = Builder->GetPropertyUtilities();
			if (!Utilities.IsValid())
			{
				return;
			}

			FInstancedStructDataDetails::GenerateHeaderRowContent(NodeRow);
			NodeRow.PropertyHandleList({KeyProperty, StructProperty});
			NodeRow.NameContent()
			[
				SAssignNew(KeyWidget, SBox)
				[
					SNew(SGameplayTagCombo)
					.PropertyHandle(KeyProperty)
					.DisplayShortTagNames(true)
					.Filter(KeyFilter)
				]
			]
			.ValueContent()
			[
				SAssignNew(ValueWidget, SBox)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.MinWidth(150.0f)
					.AutoWidth()
					[
						SNew(SInstancedStructPicker, StructProperty, Utilities)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.Padding(50.0f, 0.0f)
					.AutoWidth()
					[
						PropertyCustomizationHelpers::MakeClearButton(
							FSimpleDelegate::CreateSP(Customization.ToSharedRef(), &FToroDatabaseCustomization::OnRemoveEntry, Index),
							LOCTEXT("DatabaseEntryRemoveTooltip", "Removes this entry from the database.")
						)
					]
				]
			];
		}
	};

public:

	virtual ~FToroDatabaseCustomization() override
	{
		CleanupEntries();
	}

private:

	uint32 NumEntries = 0;
	FDelegateHandle RebuildHandle;
	TSharedPtr<IPropertyHandleMap> EntriesMap;
	TArray<TSharedRef<FEntryBuilder>> EntryBuilders;

	void DetachEntryWidgets()
	{
		for (const TSharedRef<FEntryBuilder>& Builder : EntryBuilders)
		{
			Builder->DetachWidgets();
		}
	}

	void CleanupEntries()
	{
		if (EntriesMap.IsValid() && RebuildHandle.IsValid())
		{
			EntriesMap->UnregisterOnNumElementsChanged(RebuildHandle);
		}
		RebuildHandle.Reset();

		DetachEntryWidgets();
		EntryBuilders.Empty();
		EntriesMap.Reset();
		NumEntries = 0;
	}

	void OnMapRebuilt()
	{
		DetachEntryWidgets();
		if (const TSharedPtr<IDetailLayoutBuilder> Builder = WeakBuilder.Pin())
		{
			Builder->GetPropertyUtilities()->RequestForceRefresh();
		}
	}

	bool HasEntries() const
	{
		return NumEntries != 0;
	}

	FText GetNumEntriesText() const
	{
		return NumEntries == 1
			? LOCTEXT("DatabaseSingleEntry", "1 Entry")
			: FText::Format(LOCTEXT("DatabaseNumEntry", "{0} Entries"), FText::AsNumber(NumEntries));
	}

	void OnAddEntry() const
	{
		if (EntriesMap.IsValid())
		{
			EntriesMap->AddItem();
		}
	}

	void OnRemoveEntry(int32 Index) const
	{
		if (EntriesMap.IsValid())
		{
			EntriesMap->DeleteItem(Index);
		}
	}

	void OnClearEntries() const
	{
		if (EntriesMap.IsValid())
		{
			EntriesMap->Empty();
		}
	}

	virtual void PendingDelete() override
	{
		CleanupEntries();
	}

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override
	{
		CleanupEntries();

		GET_CLASS_PROPERTY_VAR_NS(Entries, EntriesHandle)
		if (!EntriesHandle->IsValidHandle())
		{
			return;
		}

		EntriesMap = EntriesHandle->AsMap();
		if (!EntriesMap.IsValid() || EntriesMap->GetNumElements(NumEntries) != FPropertyAccess::Success)
		{
			return;
		}

		RebuildHandle = EntriesMap->SetOnNumElementsChanged(FSimpleDelegate::CreateSP(this, &FToroDatabaseCustomization::OnMapRebuilt));
		if (!RebuildHandle.IsValid())
		{
			return;
		}

		IDetailCategoryBuilder& Entries = SetCategoryDisplayName(TEXT("Entries"), LOCTEXT("DatabaseEntryLabel", "Entries"));
		Entries.HeaderContent(
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(5.0f, 0.0f)
			.MinWidth(75.0f)
			.AutoWidth()
			[
				SNew(STextBlock)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Text(TAttribute<FText>::CreateSP(this, &FToroDatabaseCustomization::GetNumEntriesText))
			]
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(1.0f, 0.0f)
			.AutoWidth()
			[
				PropertyCustomizationHelpers::MakeAddButton(
					FSimpleDelegate::CreateSP(this, &FToroDatabaseCustomization::OnAddEntry),
					LOCTEXT("DatabaseEntryAddTooltip", "Adds a new entry to the database")
				)
			]
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(1.0f, 0.0f)
			.AutoWidth()
			[
				PropertyCustomizationHelpers::MakeDeleteButton(
					FSimpleDelegate::CreateSP(this, &FToroDatabaseCustomization::OnClearEntries),
					LOCTEXT("DatabaseEntryClearTooltip", "Removes all entries from the database"),
					TAttribute<bool>::CreateSP(this, &FToroDatabaseCustomization::HasEntries)
				)
			]
		);

		if (!HasEntries())
		{
			Entries.AddCustomRow(LOCTEXT("DatabaseEntryLabel", "Entries"))
			.WholeRowContent()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Text(LOCTEXT("DatabaseEmptyMessage", "No entries present. Click the add button to create some."))
			];
			return;
		}

		FString KeyFilter = FString();
		const TSharedRef<IPropertyHandle> RootTagHandle = GET_CLASS_PROPERTY_NS(RootTag);
		if (RootTagHandle->IsValidHandle())
		{
			if (const TSharedPtr<IPropertyHandle> TagNameHandle = RootTagHandle->GetChildHandle(0))
			{
				FName TagName = NAME_None;
				if (TagNameHandle->GetValue(TagName) == FPropertyAccess::Success && !TagName.IsNone())
				{
					KeyFilter = TagName.ToString();
				}
			}
		}

		FString ValueFilter = FString();
		const TSharedRef<IPropertyHandle> RootStructHandle = GET_CLASS_PROPERTY_NS(RootStruct);
		if (RootStructHandle->IsValidHandle())
		{
			const UObject* StructVal = nullptr;
			if (RootStructHandle->GetValue(StructVal) == FPropertyAccess::Success && IsValid(StructVal))
			{
				if (const UScriptStruct* StructPtr = Cast<UScriptStruct>(StructVal))
				{
					ValueFilter = StructPtr->GetPathName().TrimStartAndEnd();
				}
			}
		}

		for (int32 i = 0; i < static_cast<int32>(NumEntries); i++)
		{
			const TSharedRef<IPropertyHandle> EntryHandle = EntriesMap->GetElement(i);
			if (!EntryHandle->IsValidHandle())
			{
				continue;
			}

			const TSharedRef<FEntryBuilder> EntryBuilder = MakeShared<FEntryBuilder>(
				i, KeyFilter, EntryHandle, StaticCastSharedRef<FToroDatabaseCustomization>(AsShared())
			);
			if (!EntryBuilder->IsValidBuilder())
			{
				continue;
			}

			EntryHandle->SetInstanceMetaData(TEXT("BaseStruct"), ValueFilter);
			Entries.AddCustomBuilder(EntryBuilder);
			EntryBuilders.Add(EntryBuilder);
		}
	}
};
#undef CLASS_NAME
#undef LOCTEXT_NAMESPACE