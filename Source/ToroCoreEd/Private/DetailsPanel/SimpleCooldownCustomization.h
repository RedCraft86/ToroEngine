// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "DetailsPanel/ToroStructCustomization.h"
#include "DataTypes/SimpleCooldown.h"

#define STRUCT_NAME FSimpleCooldown
class FSimpleCooldownCustomization final : public FToroStructCustomization
{
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		FToroStructCustomization::CustomizeHeader(StructHandle, HeaderRow, CustomizationUtils);

		GET_STRUCT_PROPERTY_VAR_NS(Interval, Interval);
		ForwardMetadata(Interval);
		HeaderRow.NameContent()
		[
			StructHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			Interval->CreatePropertyValueWidget()
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

		GET_STRUCT_PROPERTY_VAR_NS(Cooldown, Cooldown);
		StructBuilder.AddProperty(Cooldown.ToSharedRef()).IsEnabled(false);
	}
};
#undef STRUCT_NAME