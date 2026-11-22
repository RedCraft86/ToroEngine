// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UObject/SoftObjectPtr.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "WorldPickupActionBehavior.generated.h"

class UItemDefinition;

/**
 * Transfers items from the owning world actor to the local player's inventory.
 * Leave One Shot disabled so the player can return for any quantity that did not fit.
 */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Inventory] World Pickup")
class TOROSYSTEMS_API UWorldPickupActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	/** Item granted by this pickup. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSoftObjectPtr<UItemDefinition> Item;

	/** Number of items available from this actor. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 1))
	uint8 Quantity = 1;

	uint8 CollectedQuantity = 0;
	bool bCollecting = false;

	virtual bool CanTrigger() const override;
	virtual void OnTriggered() override;
};
