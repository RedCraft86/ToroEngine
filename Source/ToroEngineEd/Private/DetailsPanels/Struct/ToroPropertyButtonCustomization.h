// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DetailWidgetRow.h"
#include "DetailsPanels/ToroStructCustomization.h"
#include "DataTypes/ToroPropertyButton.h"

class FToroPropertyButtonCustomization final : public FToroStructCustomization
{
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle,
		FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		FToroStructCustomization::CustomizeHeader(StructHandle, HeaderRow, CustomizationUtils);

		TArray<void*> RawData;
		StructHandle->AccessRawData(RawData);
		if (RawData.Num() > 1)
		{
			HeaderRow.NameContent()
			[
				StructHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values"))
			];
			return;
		}

		static const FName META_ButtonLabel(TEXT("Label"));
		FText Label = FText::FromString(StructHandle->GetMetaData(META_ButtonLabel));
		if (Label.IsEmptyOrWhitespace())
		{
			Label = StructHandle->GetPropertyDisplayName();
		}

		static const FName META_WholeRowButton(TEXT("WholeRowButton"));
		if (StructHandle->HasMetaData(META_WholeRowButton))
		{
			HeaderRow.WholeRowContent()
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.Padding(2.0f)
				.AutoHeight()
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(StructHandle->GetToolTipText())
				]
				+SVerticalBox::Slot()
				.Padding(2.0f)
				.AutoHeight()
				[
					CreateButton(Label, FText::GetEmpty())
				]
			];
		}
		else
		{
			HeaderRow.NameContent()
			[
				StructHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				CreateButton(Label, StructHandle->GetToolTipText())
			];
		}
	}

	FReply OnClicked() const
	{
		if (WeakStructHandle.IsValid())
		{
			TArray<void*> RawData;
			WeakStructHandle.Pin()->AccessRawData(RawData);
			if (const FToroPropertyButton* Button = !RawData.IsEmpty() && RawData[0] != nullptr
				? static_cast<FToroPropertyButton*>(RawData[0]) : nullptr)
			{
				Button->Delegate.Broadcast();
			}
		}

		return FReply::Handled();
	}

	TSharedRef<SWidget> CreateButton(const FText& Label, const FText& Tooltip)
	{
		return SNew(SButton)
			.Text(Label)
			.ToolTipText(Tooltip)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked(FOnClicked::CreateSP(this, &FToroPropertyButtonCustomization::OnClicked));
	}
};