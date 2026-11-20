// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Inventory/Widgets/ToroInventoryArchiveEntry.h"
#include "Inventory/ItemDefinition.h"
#include "Components/TextBlock.h"

UToroInventoryArchiveEntry::UToroInventoryArchiveEntry()
{
}

bool UToroInventoryArchiveEntry::InitEntry()
{
	if (!IsValid(NameText))
	{
		RemoveFromParent();
		return false;
	}

	if (Super::InitEntry())
	{
		NameText->SetText(Item->DisplayName);
		return true;
	}

	return false;
}

void UToroInventoryArchiveEntry::NativeConstruct()
{
	Super::NativeConstruct();
	if (!IsValid(NameText))
	{
		RemoveFromParent();
	}
}
