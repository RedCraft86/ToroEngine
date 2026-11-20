// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Inventory/ItemDefinition.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

uint8 UItemDefinition::GetItemLimit() const
{
	return (Fragment.IsValid() && Fragment.Get().EnforceLimit > 0)
		? Fragment.Get().EnforceLimit : ItemLimit;
}

float UItemDefinition::GetPriority() const
{
	if (Fragment.IsValid())
	{
		return Priority + Fragment.Get().PriorityOffset;
	}
	return Priority;
}

bool UItemDefinition::IsDefinitionValid() const
{
	return !DisplayName.IsEmptyOrWhitespace()
		&& (!Icon.IsNull() || Fragment.GetScriptStruct() == FItemTextFragment::StaticStruct());
}

#if WITH_EDITOR
bool UItemDefinition::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty && InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, ItemLimit))
	{
		if (Fragment.IsValid() && Fragment.Get().EnforceLimit > 0)
		{
			return false;
		}
	}

	return Super::CanEditChange(InProperty);
}

EDataValidationResult UItemDefinition::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult Parent = Super::IsDataValid(Context);

	bool bHasErrors = false;
	if (DisplayName.IsEmptyOrWhitespace())
	{
		Context.AddError(INVTEXT("DisplayName is empty or whitespace."));
		bHasErrors = true;
	}
	if (Icon.IsNull() && Fragment.GetScriptStruct() != FItemTextFragment::StaticStruct())
	{
		Context.AddError(INVTEXT("Item that aren't a text archive requires an icon."));
		bHasErrors = true;
	}
	if (Fragment.IsValid())
	{
		bHasErrors = !Fragment.Get().Validate(Context);
	}

	return bHasErrors ? EDataValidationResult::Invalid : Parent;
}
#endif
