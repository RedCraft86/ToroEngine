// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DetailsPanels/Struct/ToroInlineStructCustomization.h"
#include "IDetailChildrenBuilder.h"

TSharedPtr<IPropertyHandle> FToroInlineStructCustomization::GetHeaderProperty()
{
	if (HeaderProperty.IsValid() && HeaderProperty->IsValidHandle() && HeaderProperty->GetProperty())
	{
		return HeaderProperty;
	}

	const TSharedPtr<IPropertyHandle> StructHandle = WeakStructHandle.Pin();
	if (!StructHandle.IsValid() || !StructHandle->IsValidHandle())
	{
		return nullptr;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(StructHandle->GetProperty());
	if (!StructProperty || !StructProperty->Struct)
	{
		return nullptr;
	}

	static const FName META_HeaderProperty(TEXT("HeaderProperty"));
	const UScriptStruct* StructType = StructProperty->Struct;

	// Try getting from name provided via metadata first
	if (StructType->HasMetaData(META_HeaderProperty))
	{
		const FName PropertyName(*StructType->GetMetaData(META_HeaderProperty));
		HeaderProperty = StructHandle->GetChildHandle(PropertyName, false);
	}

	// Get the first child property if no metadata present or it was not found
	if (!HeaderProperty.IsValid() || !HeaderProperty->IsValidHandle())
	{
		uint32 NumChildren = 0;
		if (StructHandle->GetNumChildren(NumChildren) != FPropertyAccess::Success || NumChildren == 0)
		{
			return nullptr;
		}

		HeaderProperty = StructHandle->GetChildHandle(0);
	}

	if (!HeaderProperty.IsValid() || !HeaderProperty->IsValidHandle() || !HeaderProperty->GetProperty())
	{
		HeaderProperty.Reset();
		return nullptr;
	}

	ForwardMetadata(HeaderProperty);
	return HeaderProperty;
}

void FToroInlineStructCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderProperty.Reset();
	HeaderValueWidget.Reset();

	FToroStructCustomization::CustomizeHeader(StructHandle, HeaderRow, CustomizationUtils);
	if (const TSharedPtr<IPropertyHandle> Property = GetHeaderProperty())
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

void FToroInlineStructCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FToroStructCustomization::CustomizeChildren(StructHandle, ChildBuilder, CustomizationUtils);

	const TSharedPtr<IPropertyHandle> Property = GetHeaderProperty();
	if (!Property.IsValid() || !Property->IsValidHandle() || !Property->GetProperty() || !HeaderValueWidget.IsValid())
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
	}
	else
	{
		uint32 NumHeaderChildren = 0;
		if (Property->GetNumChildren(NumHeaderChildren) != FPropertyAccess::Success)
		{
			return;
		}

		// Ordinary compound properties expose their children directly. Single property types like scalars add nothing.
		for (uint32 i = 0; i < NumHeaderChildren; i++)
		{
			const TSharedPtr<IPropertyHandle> Child = Property->GetChildHandle(i);
			if (Child.IsValid() && Child->IsValidHandle() && Child->GetProperty())
			{
				ChildBuilder.AddProperty(Child.ToSharedRef());
			}
		}
	}

	uint32 NumChildren = 0;
	if (StructHandle->GetNumChildren(NumChildren) != FPropertyAccess::Success)
	{
		return;
	}

	// Any remaining properties of this struct (if any) expose their children directly.
	for (uint32 i = 0; i < NumChildren; i++)
	{
		const TSharedPtr<IPropertyHandle> Child = StructHandle->GetChildHandle(i);
		if (Child.IsValid() && Child->IsValidHandle() && Child->GetProperty()
			&& Child->GetProperty() != Property->GetProperty()) // Avoid re-adding the header
		{
			ChildBuilder.AddProperty(Child.ToSharedRef());
		}
	}
}
