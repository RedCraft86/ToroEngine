// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "DetailsPanel/ToroWrapperCustomization.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"

TSharedPtr<IPropertyHandle> FToroWrapperCustomization::GetInnerProperty() const
{
	const TSharedPtr<IPropertyHandle> StructHandle = WeakStructHandle.Pin();
	if (!StructHandle.IsValid() || !StructHandle->IsValidHandle())
	{
		return nullptr;
	}

	const FProperty* StructProperty = StructHandle->GetProperty();
	if (!StructProperty)
	{
		return nullptr;
	}

	uint32 NumChildren = 0;
	if (StructHandle->GetNumChildren(NumChildren) != FPropertyAccess::Success)
	{
		return nullptr;
	}

	if (!ensureMsgf(NumChildren == 1,
		TEXT("Structs using FToroWrapperCustomization must have exactly 1 child property. %s does not."),
		*StructProperty->GetName()))
	{
		return nullptr;
	}

	const TSharedPtr<IPropertyHandle> Property = StructHandle->GetChildHandle(0);
	if (!Property.IsValid() || !Property->IsValidHandle() || !Property->GetProperty())
	{
		return nullptr;
	}

	ForwardMetadata(Property);
	return Property;
}

void FToroWrapperCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	InnerCustomization.Reset();
	FToroStructCustomization::CustomizeHeader(StructHandle, HeaderRow, CustomizationUtils);

	if (const TSharedPtr<IPropertyHandle> Property = GetInnerProperty())
	{
		FPropertyEditorModule& Module = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		const FPropertyTypeLayoutCallback Callback = Module.GetPropertyTypeCustomization(
			Property->GetProperty(), *Property, FCustomPropertyTypeLayoutMap());

		if (Callback.IsValid())
		{
			InnerCustomization = Callback.GetCustomizationInstance();
		}

		if (InnerCustomization.IsValid())
		{
			InnerCustomization->CustomizeHeader(Property.ToSharedRef(), HeaderRow, CustomizationUtils);
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
			StructHandle->CreatePropertyNameWidget()
		];

		const FText TooltipText = StructHandle->GetToolTipText();
		if (!TooltipText.IsEmptyOrWhitespace())
		{
			HeaderRow.NameWidget.Widget->SetToolTipText(TooltipText);
			HeaderRow.ValueWidget.Widget->SetToolTipText(TooltipText);
			HeaderRow.WholeRowWidget.Widget->SetToolTipText(TooltipText);
		}
	}
}

void FToroWrapperCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FToroStructCustomization::CustomizeChildren(StructHandle, ChildBuilder, CustomizationUtils);

	if (const TSharedPtr<IPropertyHandle> Property = GetInnerProperty())
	{
		if (InnerCustomization.IsValid())
		{
			InnerCustomization->CustomizeChildren(Property.ToSharedRef(), ChildBuilder, CustomizationUtils);
		}
		else
		{
			uint32 NumChildren = 0;
			if (Property->GetNumChildren(NumChildren) != FPropertyAccess::Success)
			{
				return;
			}

			for (uint32 i = 0; i < NumChildren; i++)
			{
				const TSharedPtr<IPropertyHandle> Child = Property->GetChildHandle(i);
				if (Child.IsValid() && Child->IsValidHandle() && Child->GetProperty())
				{
					ChildBuilder.AddProperty(Child.ToSharedRef());
				}
			}
		}
	}
}
