// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DetailsPanels/ToroStructCustomization.h"

class TOROENGINEED_API FToroWrapperCustomization final : public FToroStructCustomization
{
	TSharedPtr<SBox> HeaderValueWidget;

	TSharedPtr<IPropertyHandle> GetInnerProperty() const;

	virtual bool ShouldInlineKey() const override { return true; }

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle, IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
};
