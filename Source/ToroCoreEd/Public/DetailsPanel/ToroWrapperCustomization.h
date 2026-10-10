// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToroStructCustomization.h"

/**
 * Flattens a struct with exactly one child property into the child's editor presentation.
 * Forwards wrapper metadata to the inner property before resolving its globally registered
 * customization. Delegates header and child rendering when one exists, while supplying the
 * wrapper's name widget and non-empty tooltip. Otherwise, uses the inner value widget and
 * exposes its valid immediate children as ordinary detail rows.
 */
class TOROCOREED_API FToroWrapperCustomization final : public FToroStructCustomization
{
	/**
	 * Inner customization resolved for the current header pass and reused for its child rows.
	 * Reset before each header pass; null when the inner property has no registered customization.
	 */
	TSharedPtr<IPropertyTypeCustomization> InnerCustomization;

	/**
	 * Resolves the wrapper's sole child and forwards reflected wrapper metadata to it.
	 * A valid wrapper with a child count other than one triggers an ensure diagnostic.
	 * @return Inner handle, or nullptr for invalid handles, missing reflected properties,
	 * a failed child-count query, or a child count other than one.
	 */
	TSharedPtr<IPropertyHandle> GetInnerProperty() const;

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
};
