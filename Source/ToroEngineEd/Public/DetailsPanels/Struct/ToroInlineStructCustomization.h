// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DetailsPanels/ToroStructCustomization.h"

/**
 * Displays a selected struct property's value inline in the struct's header.
 *
 * Selects a direct child using USTRUCT(meta = (HeaderProperty = "PropertyName")).
 * Falls back to the first child if the metadata is absent or the named child
 * cannot be resolved.
 *
 * Displays the selected property's children and the remaining struct properties
 * beneath the header, preserving nested struct customizations.
 */
class TOROENGINEED_API FToroInlineStructCustomization final : public FToroStructCustomization
{
	TSharedPtr<SBox> HeaderValueWidget;
	TSharedPtr<IPropertyHandle> HeaderProperty;

	TSharedPtr<IPropertyHandle> GetHeaderProperty();

	virtual bool ShouldInlineKey() const override { return true; }

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle, IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
};
