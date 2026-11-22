// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Inventory/InventoryBehavior.h"
#include "Inventory/Widgets/ToroInventoryWidget.h"
#include "Inventory/EquipmentActor.h"
#include "Inventory/ItemDefinition.h"
#include "UObject/SoftObjectPath.h"
#include "Engine/World.h"

UInventoryBehavior::UInventoryBehavior()
{
	PrimaryComponentTick.bCanEverTick = false;
}

uint8 UInventoryBehavior::AddItem(const UItemDefinition* Item, uint8 Quantity)
{
	if (bRestoring || !IsValid(Item) || !Item->IsDefinitionValid() || Quantity == 0)
	{
		return Quantity;
	}

	const TSoftObjectPtr<UItemDefinition> ItemKey{FSoftObjectPath(Item)};
	if (!Items.Contains(ItemKey))
	{
		Items.Add(ItemKey, 0);
	}

	uint8 Overflow = Quantity;
	const uint8 OldAmount = Items[ItemKey];
	const uint8 NewAmount = static_cast<uint8>(FMath::Min(
		OldAmount + Quantity, static_cast<int32>(Item->GetItemLimit()))
	);
	Overflow -= NewAmount - OldAmount;
	Items[ItemKey] = NewAmount;

	if (OldAmount == 0 && NewAmount != 0)
	{
		NewItems.Add(ItemKey);
	}

	OnItemChangedEvent.Broadcast(Item, OldAmount, NewAmount);
	return Overflow;
}

bool UInventoryBehavior::AddArchive(const UItemDefinition* Item, bool bOpenUI)
{
	if (bRestoring || !IsValid(Item) || !Item->IsDefinitionValid()
		|| Item->Fragment.GetPtr<FItemTextFragment>() == nullptr)
	{
		return false;
	}

	if (GetQuantity(Item) != 0 || AddItem(Item) == 0)
	{
		if (bOpenUI)
		{
			OpenArchive = TSoftObjectPtr<UItemDefinition>(FSoftObjectPath(Item));
			UToroInventoryWidget::Push(this);
		}
		return true;
	}

	return false;
}

bool UInventoryBehavior::AddEquipment(const UItemDefinition* Item, bool bEquip)
{
	if (bRestoring || !IsValid(Item) || !Item->IsDefinitionValid()
		|| Item->Fragment.GetPtr<FItemEquipmentFragment>() == nullptr)
	{
		return false;
	}

	if (GetQuantity(Item) != 0 || AddItem(Item) == 0)
	{
		if (bEquip)
		{
			EquipItem(Item);
		}
		return true;
	}

	return false;
}

bool UInventoryBehavior::RemoveItem(const UItemDefinition* Item, uint8 Quantity)
{
	if (bRestoring || Quantity == 0 || !HasItem(Item, Quantity))
	{
		return false;
	}

	const TSoftObjectPtr<UItemDefinition> ItemKey{FSoftObjectPath(Item)};
	const uint8 OldAmount = Items[ItemKey];
	const uint8 NewAmount = static_cast<uint8>(FMath::Max(static_cast<int32>(OldAmount) - Quantity, 0));
	if (NewAmount == 0)
	{
		Items.Remove(ItemKey);
		if (Item == EquipmentItem)
		{
			UnequipItem();
		}
	}
	else
	{
		Items[ItemKey] = NewAmount;
	}

	OnItemChangedEvent.Broadcast(Item, OldAmount, NewAmount);
	return true;
}

uint8 UInventoryBehavior::GetQuantity(const UItemDefinition* Item) const
{
	if (!IsValid(Item))
	{
		return 0;
	}

	return Items.FindRef(TSoftObjectPtr<UItemDefinition>(FSoftObjectPath(Item)), 0);
}

bool UInventoryBehavior::HasItem(const UItemDefinition* Item, uint8 Quantity) const
{
	if (!IsValid(Item))
	{
		return false;
	}

	return GetQuantity(Item) >= Quantity;
}

void UInventoryBehavior::UseEquipment() const
{
	if (EquipmentActor)
	{
		EquipmentActor->Use();
	}
}

const UItemDefinition* UInventoryBehavior::GetEquipment() const
{
	return EquipmentItem.LoadSynchronous();
}

bool UInventoryBehavior::EquipItem(const UItemDefinition* Item)
{
	if (bRestoring || !IsValid(Item) || !HasItem(Item)
		|| EquipmentItem == Item)
	{
		return false;
	}

	const FItemEquipmentFragment* Fragment = Item->Fragment.GetPtr<FItemEquipmentFragment>();
	if (!Fragment || Fragment->EquipmentClass.IsNull())
	{
		return false;
	}

	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.Instigator = GetOwner()->GetInstigator();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AEquipmentActor* SpawnedActor = GetWorld()->SpawnActor<AEquipmentActor>(
		Fragment->EquipmentClass.LoadSynchronous(), GetOwner()->GetActorTransform(), Params);
	if (!IsValid(SpawnedActor))
	{
		return false;
	}

	if (!SpawnedActor->PrepareEquipment(GetOwner()) || !IsValid(SpawnedActor))
	{
		if (IsValid(SpawnedActor))
		{
			SpawnedActor->Destroy();
		}
		return false;
	}

	UnequipItem();

	EquipmentItem = TSoftObjectPtr<UItemDefinition>(FSoftObjectPath(Item));
	EquipmentActor = SpawnedActor;
	EquipmentActor->Equipped();
	return true;
}

bool UInventoryBehavior::UnequipItem()
{
	if (bRestoring || (EquipmentItem.IsNull() && !EquipmentActor))
	{
		return false;
	}

	AEquipmentActor* PreviousActor = EquipmentActor;
	EquipmentItem.Reset();
	EquipmentActor = nullptr;

	if (IsValid(PreviousActor))
	{
		PreviousActor->Unequipped();
	}

	return true;
}

bool UInventoryBehavior::IsNewItem(const UItemDefinition* Item) const
{
	return IsValid(Item) && NewItems.Contains(TSoftObjectPtr<UItemDefinition>(FSoftObjectPath(Item)));
}

void UInventoryBehavior::MarkItemSeen(const UItemDefinition* Item)
{
	if (!bRestoring && IsValid(Item))
	{
		NewItems.Remove(TSoftObjectPtr<UItemDefinition>(FSoftObjectPath(Item)));
	}
}

void UInventoryBehavior::GetItems(TArray<const UItemDefinition*>& OutItems) const
{
	OutItems.Empty();
	OutItems.Reserve(Items.Num());
	for (const TPair<TSoftObjectPtr<UItemDefinition>, uint8>& Pair : Items)
	{
		if (Pair.Key.LoadSynchronous() && Pair.Value != 0)
		{
			OutItems.Emplace(Pair.Key.LoadSynchronous());
		}
	}

	OutItems.Sort([](const UItemDefinition& A, const UItemDefinition& B) -> bool
	{
		const float PriorityA = A.GetPriority();
		const float PriorityB = B.GetPriority();
		return (PriorityA != PriorityB) ? (PriorityA < PriorityB) : (A.DisplayName.CompareTo(B.DisplayName) < 0);
	});
}

void UInventoryBehavior::PreSerialize_Implementation(bool bLoading)
{
	if (bLoading)
	{
		bRestoring = true;
	}
}

void UInventoryBehavior::PostSerialize_Implementation(bool bLoading)
{
	if (bLoading)
	{
		bRestoring = false;

		const UItemDefinition* SavedEquipment = EquipmentItem.LoadSynchronous();

		UnequipItem();
		EquipItem(SavedEquipment);
	}
}

void UInventoryBehavior::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnequipItem();
	OnItemChangedEvent.Clear();
	Super::EndPlay(EndPlayReason);
}
