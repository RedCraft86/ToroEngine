// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToroStructCustomization.h"

class TOROCOREED_API FToroWrapperCustomization final : public FToroStructCustomization
{
	TSharedPtr<IPropertyTypeCustomization> ChildCustomization;

	TSharedPtr<IPropertyHandle> GetInnerProperty() const;

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
};
