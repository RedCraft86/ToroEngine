// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Math/Transform.h"
#include "GameFramework/Actor.h"
#include "EquipmentActor.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API AEquipmentActor : public AActor
{
	GENERATED_BODY()

public:

	/** Transform relative to the player's first-person equipment mount. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Equipment)
	FTransform CameraOffset = FTransform::Identity;

	/**
	 * Prepares this actor for equipping. The default implementation attaches it to a Toro player's
	 * equipment mount and applies CameraOffset. Call the parent implementation to retain that setup.
	 * Returns false if preparation fails.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = Equipment)
	bool PrepareEquipment(AActor* InventoryOwner);
	virtual bool PrepareEquipment_Implementation(AActor* InventoryOwner);

	UFUNCTION(BlueprintNativeEvent, Category = Equipment)
	void Equipped();
	virtual void Equipped_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = Equipment)
	void Unequipped();
	virtual void Unequipped_Implementation()
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Destroy();
	}

	UFUNCTION(BlueprintNativeEvent, Category = Equipment)
	bool Use();
	virtual bool Use_Implementation() { return false; }
};
