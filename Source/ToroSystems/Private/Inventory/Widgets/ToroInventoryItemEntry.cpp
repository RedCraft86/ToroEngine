// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Inventory/Widgets/ToroInventoryItemEntry.h"
#include "Inventory/InventoryBehavior.h"
#include "Inventory/ItemDefinition.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

UToroInventoryItemEntry::UToroInventoryItemEntry()
{
}

bool UToroInventoryItemEntry::InitEntry()
{
	if (!IsValid(IconImage) || !IsValid(AmountText) || !IsValid(EquipMarker))
	{
		RemoveFromParent();
		return false;
	}

	if (Super::InitEntry())
	{
		IconImage->SetBrushFromTexture(Item->Icon.LoadSynchronous());
		AmountText->SetText(FText::AsNumber(Inventory->GetQuantity(Item)));
		RefreshEquipmentMarker();

		return true;
	}

	return false;
}

void UToroInventoryItemEntry::RefreshEquipmentMarker() const
{
	if (IsValid(EquipMarker) && Inventory.IsValid())
	{
		EquipMarker->SetVisibility(Inventory->GetEquipment() == Item
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UToroInventoryItemEntry::NativeConstruct()
{
	Super::NativeConstruct();
	if (!IsValid(IconImage) || !IsValid(AmountText) || !IsValid(EquipMarker))
	{
		RemoveFromParent();
	}
}
