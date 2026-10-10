// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "DetailsPanel/ToroStructCustomization.h"
#include "ScopedTransaction.h"
#include "DataTypes/ToroIdentity.h"

#define STRUCT_NAME FToroIdentity
class FToroIdentityCustomization final : public FToroStructCustomization
{
	static void AccessStruct(TArray<FToroIdentity*>& Structs, TSharedRef<IPropertyHandle> StructHandle)
	{
		Structs.Empty();
		if (!StructHandle->IsValidHandle() || StructHandle->IsEditConst())
		{
			return;
		}

		TArray<void*> RawData;
		StructHandle->AccessRawData(RawData);
		if (RawData.IsEmpty())
		{
			return;
		}

		Structs.Reserve(RawData.Num());
		for (void* Data : RawData)
		{
			if (FToroIdentity* Identity = static_cast<FToroIdentity*>(Data))
			{
				Structs.Add(Identity);
			}
		}
	}

	static void OnClearClicked(TSharedRef<IPropertyHandle> StructHandle)
	{
		const FScopedTransaction Transaction(NSLOCTEXT("ToroCoreEd", "ClearIdentityTransaction", "Clear Identity"));

		TArray<FToroIdentity*> Structs;
		AccessStruct(Structs, StructHandle);

		StructHandle->NotifyPreChange();

		for (FToroIdentity* Identity : Structs)
		{
			Identity->Invalidate();
		}

		StructHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
		StructHandle->NotifyFinishedChangingProperties();
	}

	static bool IsResettable(TSharedRef<IPropertyHandle> StructHandle)
	{
		TArray<FToroIdentity*> Structs;
		AccessStruct(Structs, StructHandle);
		if (Structs.IsEmpty())
		{
			return false;
		}

		auto HasAnyData = [](const FToroIdentity* Identity) -> bool
		{
			return Identity->Group.IsValid() || Identity->Guid.IsValid();
		};

		bool bMultiple = false;
		const bool bAnyData = HasAnyData(Structs[0]);
		for (int32 i = 1; i < Structs.Num(); i++)
		{
			if (Structs[i] && HasAnyData(Structs[i]) != bAnyData)
			{
				bMultiple = true;
				break;
			}
		}

		return !bMultiple && bAnyData;
	}

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		FToroStructCustomization::CustomizeHeader(StructHandle, HeaderRow, CustomizationUtils);

		GET_STRUCT_PROPERTY_VAR_NS(Group, Group);
		ForwardMetadata(Group);
		HeaderRow.NameContent()
		[
			StructHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.AutoWidth()
			[
				Group->CreatePropertyValueWidgetWithCustomization(nullptr)
			]
			+SHorizontalBox::Slot()
			.Padding(1.0f, 0.0f, 0.0f, 0.0f)
			.HAlign(HAlign_Right)
			.AutoWidth()
			[
				PropertyCustomizationHelpers::MakeEmptyButton(
					FSimpleDelegate::CreateStatic(&FToroIdentityCustomization::OnClearClicked, StructHandle),
					NSLOCTEXT("ToroCoreEd", "ResetToroIdentity", "Reset Identity"),
					TAttribute<bool>::CreateStatic(&FToroIdentityCustomization::IsResettable, StructHandle)
				)
			]
		];

		const FText TooltipText = StructHandle->GetToolTipText();
		if (!TooltipText.IsEmptyOrWhitespace())
		{
			HeaderRow.NameWidget.Widget->SetToolTipText(TooltipText);
			HeaderRow.WholeRowWidget.Widget->SetToolTipText(TooltipText);
		}
	}

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle, IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		FToroStructCustomization::CustomizeChildren(StructHandle, StructBuilder, CustomizationUtils);

		GET_STRUCT_PROPERTY_VAR_NS(Guid, Guid);
		StructBuilder.AddProperty(Guid.ToSharedRef());
	}
};
#undef STRUCT_NAME