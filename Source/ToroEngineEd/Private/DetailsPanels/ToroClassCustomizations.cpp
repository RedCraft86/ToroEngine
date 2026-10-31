// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DetailsPanels/ToroClassCustomization.h"
#include "UObject/UnrealType.h"

bool FToroClassCustomization::IsPrimaryCustomization() const
{
	for (const UClass* Class = CustomizingClass; Class; Class = Class->GetSuperClass())
	{
		const FName ClassName = Class->GetFName();
		if (RegisteredEntries.Contains(ClassName))
		{
			return RegisteredClassName == ClassName;
		}
	}

	return false;
}

void FToroClassCustomization::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder)
{
	WeakBuilder.Reset();
	CustomizingClass = nullptr;
	bIsTemplateObject = false;
	if (!DetailBuilder.IsValid())
	{
		return;
	}

	UClass* SelectedClass = nullptr;
	bool bSelectedTemplate = false;
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder->GetObjectsBeingCustomized(Objects);
	for (const TWeakObjectPtr<UObject>& Object : Objects)
	{
		// Check validity and whether template customization is enabled
		if (!Object.IsValid() || (!bCustomizeTemplate && Object->IsTemplate()))
		{
			return;
		}

		if (!SelectedClass)
		{
			SelectedClass = Object->GetClass();
			bSelectedTemplate = Object->IsTemplate();
		}
		else if (SelectedClass != Object->GetClass() || bSelectedTemplate != Object->IsTemplate())
		{
			// Disallow mixed classes or a mix of templates and instances.
			return;
		}
	}

	if (!SelectedClass)
	{
		return;
	}

	WeakBuilder = DetailBuilder;
	CustomizingClass = SelectedClass;
	bIsTemplateObject = bSelectedTemplate;

	IDetailCustomization::CustomizeDetails(DetailBuilder);
}
