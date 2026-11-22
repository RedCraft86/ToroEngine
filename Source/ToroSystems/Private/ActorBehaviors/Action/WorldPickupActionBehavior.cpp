// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Action/WorldPickupActionBehavior.h"
#include "Framework/ToroPlayerCharacter.h"
#include "GameFramework/Actor.h"
#include "Inventory/InventoryBehavior.h"
#include "Inventory/ItemDefinition.h"
#include "Templates/UnrealTemplate.h"
#include "CoreTypes.h"

bool UWorldPickupActionBehavior::CanTrigger() const
{
	return !bCollecting && CollectedQuantity < Quantity && !Item.IsNull();
}

void UWorldPickupActionBehavior::OnTriggered()
{
	if (!CanTrigger())
	{
		return;
	}

	AToroPlayerCharacter* Player = AToroPlayerCharacter::Get(this);
	UInventoryBehavior* Inventory = Player ? Player->GetInventory() : nullptr;
	if (!IsValid(Inventory))
	{
		return;
	}

	const TGuardValue<bool> CollectingGuard(bCollecting, true);
	const uint8 Remaining = Quantity - CollectedQuantity;
	const uint8 Overflow = Inventory->AddItem(Item.LoadSynchronous(), Remaining);
	CollectedQuantity += Remaining - Overflow;
}
