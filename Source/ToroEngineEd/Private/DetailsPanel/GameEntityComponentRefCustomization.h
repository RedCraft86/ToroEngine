// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Brushes/SlateRoundedBoxBrush.h"
#include "CoreGlobals.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailsPanel/ToroStructCustomization.h"
#include "GameEntity/GameEntityComponentRef.h"
#include "Misc/Optional.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Styling/StyleColors.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define STRUCT_NAME FGameEntityComponentRef
#define LOCTEXT_NAMESPACE "GameEntityComponentRef"
class FGameEntityComponentRefCustomization final : public FToroStructCustomization
{
	struct FReferenceStatus final
	{
		FText TargetText = LOCTEXT("MultipleValues", "Multiple Values");
		FSlateColor Color = FSlateColor::UseForeground();
		bool bShowOwner = false;

		FText GetToolTipText() const
		{
			return bShowOwner ? FText::Format(LOCTEXT("StatusToolTip", "Owner \u2192 {0}"), TargetText) : TargetText;
		}
	};

	static FReferenceStatus GetStatus(TSharedRef<IPropertyHandle> StructHandle)
	{
		TArray<void*> RawData;
		StructHandle->AccessRawData(RawData);
		if (RawData.Num() > 1)
		{
			return FReferenceStatus(LOCTEXT("MultipleValues", "Multiple Values"), FSlateColor::UseSubduedForeground(), false);
		}

		if (RawData.IsEmpty() || RawData[0] == nullptr)
		{
			return FReferenceStatus();
		}

		const FGameEntityComponentRef* Ref = static_cast<const FGameEntityComponentRef*>(RawData[0]);
		if (!Ref->GetOwner())
		{
			return FReferenceStatus();
		}

		if (!Ref->GetClass())
		{
			return FReferenceStatus(LOCTEXT("NoClass", "???"), FStyleColors::AccentRed, true);
		}

		if (const UGameEntityComponent* Comp = Ref->FindComponent())
		{
			return FReferenceStatus(FText::FromString(Comp->GetName()), FStyleColors::AccentGreen, true);
		}

		return FReferenceStatus(LOCTEXT("NoReference", "None"), FStyleColors::AccentYellow, true);
	}

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		FToroStructCustomization::CustomizeHeader(StructHandle, HeaderRow, CustomizationUtils);

		HeaderRow.NameContent()
		[
			StructHandle->CreatePropertyNameWidget()
		];

		TArray<UObject*> OuterObjects;
		StructHandle->GetOuterObjects(OuterObjects);
		if (OuterObjects.IsEmpty() || !OuterObjects[0] || OuterObjects[0]->IsTemplate())
		{
			GET_STRUCT_PROPERTY_VAR_NS(ComponentClass, ComponentClass);
			HeaderRow.ValueContent()
			[
				ComponentClass->CreatePropertyValueWidget()
			];
		}
		else
		{
			const FReferenceStatus Status = GetStatus(StructHandle);
			const FSlateFontInfo MonoFont = FCoreStyle::GetDefaultFontStyle(TEXT("Mono"), 10);
			static const FSlateRoundedBoxBrush BadgeBrush(FStyleColors::Recessed, 4.0f);

			HeaderRow.ValueContent()
			.MaxDesiredWidth(400.0f)
			[
				SNew(SHorizontalBox)
				.ToolTipText(Status.GetToolTipText())
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(SHorizontalBox)
					.Visibility(Status.bShowOwner ? EVisibility::Visible : EVisibility::Collapsed)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SBorder)
						.BorderImage(&BadgeBrush)
						.Padding(FMargin(8.0f, 3.0f))
						[
							SNew(STextBlock)
							.Font(MonoFont)
							.ColorAndOpacity(FSlateColor::UseForeground())
							.Text(LOCTEXT("OwnerLabel", "Owner"))
						]
					]
					+ SHorizontalBox::Slot()
					.Padding(FMargin(6.0f, 0.0f))
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(20.0f)
						.HeightOverride(20.0f)
						[
							SNew(SImage)
							.Image(FAppStyle::GetBrush(TEXT("Icons.ArrowRight")))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
					]
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.BorderImage(&BadgeBrush)
					.Padding(FMargin(8.0f, 3.0f))
					[
						SNew(STextBlock)
						.Font(MonoFont)
						.Text(Status.TargetText)
						.ColorAndOpacity(Status.Color)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					]
				]
			];
		}
	}
};
#undef LOCTEXT_NAMESPACE
#undef STRUCT_NAME
