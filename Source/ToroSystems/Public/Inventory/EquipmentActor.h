// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/Actor.h"
#include "EquipmentActor.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API AEquipmentActor : public AActor
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, Category = Equipment)
	bool PrepareEquipment(AActor* InventoryOwner);
	virtual bool PrepareEquipment_Implementation(AActor* InventoryOwner)
	{
		return IsValid(InventoryOwner);
	}

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
