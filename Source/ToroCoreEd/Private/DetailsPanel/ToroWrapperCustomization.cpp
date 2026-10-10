// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "DetailsPanel/ToroWrapperCustomization.h"
#include "DetailWidgetRow.h"

TSharedPtr<IPropertyHandle> FToroWrapperCustomization::GetInnerProperty() const
{
	if (WeakStructHandle.IsValid())
	{
		const TSharedPtr<IPropertyHandle> StructHandle = WeakStructHandle.Pin();

		uint32 NumChildren = 0;
		StructHandle->GetNumChildren(NumChildren);
		if (ensureMsgf(NumChildren == 1,
			TEXT("Structs using FToroWrapperCustomization must have exactly 1 child property. %s does not."),
			*StructHandle->GetProperty()->GetName()))
		{
			const TSharedPtr<IPropertyHandle> Property = StructHandle->GetChildHandle(0);
			ForwardMetadata(Property);
			return Property;
		}
	}

	return nullptr;
}

void FToroWrapperCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FToroStructCustomization::CustomizeHeader(PropertyHandle, HeaderRow, CustomizationUtils);
	if (const TSharedPtr<IPropertyHandle> Property = GetInnerProperty())
	{
		FPropertyEditorModule& Module = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		const FPropertyTypeLayoutCallback Callback = Module.GetPropertyTypeCustomization(
			Property->GetProperty(), *Property, FCustomPropertyTypeLayoutMap());

		if (Callback.IsValid())
		{
			ChildCustomization = Callback.GetCustomizationInstance();
		}

		if (ChildCustomization.IsValid())
		{
			ChildCustomization->CustomizeHeader(Property.ToSharedRef(), HeaderRow, CustomizationUtils);
		}
		else
		{
			HeaderRow.ValueContent()
			[
				Property->CreatePropertyValueWidgetWithCustomization(nullptr)
			];
		}

		HeaderRow.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		];

		const FText TooltipText = PropertyHandle->GetToolTipText();
		if (!TooltipText.IsEmptyOrWhitespace())
		{
			HeaderRow.NameWidget.Widget->SetToolTipText(TooltipText);
			HeaderRow.ValueWidget.Widget->SetToolTipText(TooltipText);
			HeaderRow.WholeRowWidget.Widget->SetToolTipText(TooltipText);
		}
	}
}

void FToroWrapperCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FToroStructCustomization::CustomizeChildren(PropertyHandle, ChildBuilder, CustomizationUtils);
	if (const TSharedPtr<IPropertyHandle> Property = GetInnerProperty())
	{
		if (ChildCustomization.IsValid())
		{
			ChildCustomization->CustomizeChildren(Property.ToSharedRef(), ChildBuilder, CustomizationUtils);
		}
	}
}
