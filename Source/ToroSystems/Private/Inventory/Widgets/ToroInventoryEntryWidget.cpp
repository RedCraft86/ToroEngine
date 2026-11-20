// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Inventory/Widgets/ToroInventoryEntryWidget.h"
#include "Inventory/InventoryBehavior.h"
#include "Inventory/ItemDefinition.h"
#include "CommonButtonBase.h"

UToroInventoryEntryWidget::UToroInventoryEntryWidget()
{
}

void UToroInventoryEntryWidget::Init(UInventoryBehavior* InInventory, const UItemDefinition* InItem)
{
	if ((Inventory.IsValid() && IsValid(Item))
		|| (!IsValid(InInventory) || !IsValid(InItem)))
	{
		return;
	}

	Inventory = InInventory;
	Item = InItem;

	if (!InitEntry())
	{
		Item = nullptr;
		Inventory.Reset();
	}
}

void UToroInventoryEntryWidget::SetIsSelected(bool bSelected) const
{
	if (!IsValid(SelectButton) || !IsValid(NewMarker) || !Inventory.IsValid())
	{
		return;
	}

	SelectButton->SetIsSelected(bSelected, false);
	if (bSelected)
	{
		Inventory->MarkItemSeen(Item);
		NewMarker->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UToroInventoryEntryWidget::OnButtonSelected(bool bSelected) const
{
	if (bSelected)
	{
		OnSelectedEvent.Broadcast(this);
	}
}

bool UToroInventoryEntryWidget::InitEntry()
{
	if (!IsValid(SelectButton)
		|| !IsValid(NewMarker)
		|| !Inventory.IsValid()
		|| !IsValid(Item)
		|| !Inventory->HasItem(Item))
	{
		RemoveFromParent();
		return false;
	}

	NewMarker->SetVisibility(
		Inventory->IsNewItem(Item)
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed
	);

	return true;
}

void UToroInventoryEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!IsValid(SelectButton) || !IsValid(NewMarker))
	{
		RemoveFromParent();
		return;
	}

	SelectButton->SetIsSelectable(true);
	SelectButton->SetIsInteractableWhenSelected(false);
	SelectButton->OnIsSelectedChanged().RemoveAll(this);
	SelectButton->OnIsSelectedChanged().AddUObject(this, &UToroInventoryEntryWidget::OnButtonSelected);
}

void UToroInventoryEntryWidget::NativeDestruct()
{
	OnSelectedEvent.Clear();
	if (IsValid(SelectButton))
	{
		SelectButton->OnIsSelectedChanged().RemoveAll(this);
	}
	Super::NativeDestruct();
}
