// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Containers/Array.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "GameFramework/Actor.h"
#include "PropertyHandle.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateColor.h"
#include "Styling/StyleColors.h"
#include "Templates/SharedPointer.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Internationalization/Text.h"
#include "DetailsPanels/ToroStructCustomization.h"
#include "ActorBehaviors/ActorBehaviorComponent.h"
#include "ActorBehaviors/ActorBehaviorRef.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
#define STRUCT_NAME FActorBehaviorRef
class FActorBehaviorRefCustomization final : public FToroStructCustomization
{
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle,
		FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		FToroStructCustomization::CustomizeHeader(StructHandle, HeaderRow, CustomizationUtils);

		HeaderRow.NameContent()
		[
			StructHandle->CreatePropertyNameWidget()
		];

		auto SetValueText = [&HeaderRow](const FText& Text)
		{
			HeaderRow.ValueContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Text(Text)
			];
		};

		TArray<UObject*> OuterObjects;
		StructHandle->GetOuterObjects(OuterObjects);
		if (OuterObjects.Num() != 1 || !IsValid(OuterObjects[0]))
		{
			SetValueText(LOCTEXT("BehaviorRefInvalidSelection", "Invalid Selection"));
			return;
		}

		if (OuterObjects[0]->IsTemplate())
		{
			GET_STRUCT_PROPERTY_VAR_NS(Class, ClassHandle);
			GET_STRUCT_PROPERTY_VAR_NS(bRequired, RequiredHandle);
			if (!ClassHandle.IsValid() || !ClassHandle->IsValidHandle()
				|| !RequiredHandle.IsValid() || !RequiredHandle->IsValidHandle())
			{
				SetValueText(LOCTEXT("BehaviorRefInvalidSelection", "Invalid Selection"));
				return;
			}

			HeaderRow.ValueContent()
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 2.0f)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.ColorAndOpacity(FSlateColor::UseForeground())
						.Text(LOCTEXT("BehaviorRefIsRequired", "Required"))
					]
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.Padding(5.0f, 0.0f)
					.AutoWidth()
					[
						RequiredHandle->CreatePropertyValueWidget()
					]
				]
				+SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 2.0f)
				.AutoHeight()
				[
					ClassHandle->CreatePropertyValueWidget()
				]
			];

			return;
		}

		TArray<void*> RawData;
		StructHandle->AccessRawData(RawData);
		if (RawData.Num() > 1)
		{
			SetValueText(LOCTEXT("BehaviorRefMultipleValues", "Multiple Values"));
			return;
		}

		if (RawData.IsEmpty() || !RawData[0])
		{
			SetValueText(LOCTEXT("BehaviorRefInvalidSelection", "Invalid Selection"));
			return;
		}

		static const FSlateRoundedBoxBrush BadgeBrush(FStyleColors::Recessed, 4.0f);
		const FSlateFontInfo MonoFont = FCoreStyle::GetDefaultFontStyle(TEXT("Mono"), 10);
		const FActorBehaviorRef* StructPtr = static_cast<const FActorBehaviorRef*>(RawData[0]);

		FText StatusText;
		FSlateColor StatusColor;
		if (!IsValid(StructPtr->GetClass()))
		{
			StatusText = LOCTEXT("BehaviorRefNoClass", "Unknown Class");
			StatusColor = FStyleColors::AccentRed;
		}
		else if (!IsValid(StructPtr->GetOwner()))
		{
			StatusText = LOCTEXT("BehaviorRefNoOwner", "No Owner");
			StatusColor = FStyleColors::AccentRed;
		}
		else if (const UActorBehaviorComponent* Comp = StructPtr->Find())
		{
			StatusText = FText::FromString(Comp->GetName());
			StatusColor = FSlateColor::UseForeground();
		}
		else
		{
			StatusText = LOCTEXT("BehaviorRefNoReference", "No Reference");
			StatusColor = StructPtr->IsRequired() ? FStyleColors::AccentRed : FStyleColors::AccentYellow;
		}

		HeaderRow.ValueContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
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
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(5.0f, 0.0f)
			.AutoWidth()
			[
				SNew(SBorder)
				.BorderImage(&BadgeBrush)
				.Padding(FMargin(8.0f, 3.0f))
				[
					SNew(STextBlock)
					.Font(MonoFont)
					.Text(StatusText)
					.ColorAndOpacity(StatusColor)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				]
			]
		];

		if (StructPtr->IsRequired())
		{
			HeaderRow.ExtensionContent()
			[
				SNew(SBox)
				.Padding(5.0f, 0.0f)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFontBold())
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					.Text(LOCTEXT("BehaviorRefRequired", "(Required)"))
				]
			];
		}
	}
};
#undef STRUCT_NAME
#undef LOCTEXT_NAMESPACE