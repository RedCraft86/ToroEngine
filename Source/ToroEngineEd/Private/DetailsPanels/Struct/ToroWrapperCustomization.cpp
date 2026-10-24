// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DetailsPanels/Struct/ToroWrapperCustomization.h"
#include "IDetailChildrenBuilder.h"

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
	FToroStructCustomization::CustomizeHeader(StructHandle, HeaderRow, CustomizationUtils);
	if (const TSharedPtr<IPropertyHandle> Property = GetInnerProperty())
	{
		HeaderRow.NameContent()
		[
			StructHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SAssignNew(HeaderValueWidget, SBox)
			[
				Property->CreatePropertyValueWidgetWithCustomization(nullptr)
			]
		];

		const FText TooltipText = StructHandle->GetToolTipText();
		if (!TooltipText.IsEmptyOrWhitespace())
		{
			HeaderRow.NameWidget.Widget->SetToolTipText(TooltipText);
		}
	}
}

void FToroWrapperCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FToroStructCustomization::CustomizeChildren(StructHandle, ChildBuilder, CustomizationUtils);

	const TSharedPtr<IPropertyHandle> Property = GetInnerProperty();
	if (!Property.IsValid() || !HeaderValueWidget.IsValid())
	{
		return;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(Property->GetProperty());
	const FPropertyEditorModule& Module = FModuleManager::GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	if (StructProperty && Module.IsCustomizedStruct(StructProperty->Struct, FCustomPropertyTypeLayoutMap()))
	{
		// For structs with customization, add the property as a normal row to generate its header and expanded children.
		// Move its value widget into the wrapper header, then hide the inner row's header so only its children appear when expanded.

		IDetailPropertyRow& Row = ChildBuilder.AddProperty(Property.ToSharedRef());

		TSharedPtr<SWidget> NameWidget, ValueWidget;
		Row.GetDefaultWidgets(NameWidget, ValueWidget);
		if (ValueWidget.IsValid())
		{
			HeaderValueWidget->SetContent(ValueWidget.ToSharedRef());
		}

		Row.CustomWidget(true);
		return;
	}

	uint32 NumChildren = 0;
	if (Property->GetNumChildren(NumChildren) != FPropertyAccess::Success)
	{
		return;
	}

	// Ordinary compound properties expose their children directly. Single property types like scalars add nothing.
	for (uint32 i = 0; i < NumChildren; i++)
	{
		const TSharedPtr<IPropertyHandle> Child = Property->GetChildHandle(i);
		if (Child.IsValid() && Child->IsValidHandle() && Child->GetProperty())
		{
			ChildBuilder.AddProperty(Child.ToSharedRef());
		}
	}
}
