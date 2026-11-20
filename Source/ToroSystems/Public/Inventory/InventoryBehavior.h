// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ActorBehaviors/ActorBehaviorBase.h"
#include "SaveSystem/SaveSerializeInterface.h"
#include "InventoryBehavior.generated.h"

class AEquipmentActor;
class UItemDefinition;

UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class TOROSYSTEMS_API UInventoryBehavior final : public UActorBehaviorBase, public ISaveSerializeInterface
{
	GENERATED_BODY()

	friend class UToroInventoryWidget;

public:

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnInventoryChanged, const UItemDefinition*, uint8, uint8);

	UInventoryBehavior();

	uint8 AddItem(const UItemDefinition* Item, uint8 Quantity = 1);
	bool AddArchive(const UItemDefinition* Item, bool bOpenUI = false);
	bool AddEquipment(const UItemDefinition* Item, bool bEquip = true);
	bool RemoveItem(const UItemDefinition* Item, uint8 Quantity = 1);

	uint8 GetQuantity(const UItemDefinition* Item) const;
	bool HasItem(const UItemDefinition* Item, uint8 Quantity = 1) const;

	void UseEquipment() const;
	const UItemDefinition* GetEquipment() const;
	bool EquipItem(const UItemDefinition* Item);
	bool UnequipItem();

	bool IsNewItem(const UItemDefinition* Item) const;
	void MarkItemSeen(const UItemDefinition* Item);

	void GetItems(TArray<const UItemDefinition*>& OutItems) const;

	FOnInventoryChanged::RegistrationType& OnItemChanged()
	{
		return OnItemChangedEvent;
	}

	virtual bool EligibleForSaving_Implementation() const override
	{
		return true;
	}

protected:

	UPROPERTY(SaveGame)
	TMap<TSoftObjectPtr<UItemDefinition>, uint8> Items;

	UPROPERTY(SaveGame)
	TSet<TSoftObjectPtr<UItemDefinition>> NewItems;

	UPROPERTY(SaveGame)
	TSoftObjectPtr<UItemDefinition> EquipmentItem;

	UPROPERTY(Transient)
	TObjectPtr<AEquipmentActor> EquipmentActor;

	bool bRestoring = false;
	TSoftObjectPtr<UItemDefinition> OpenArchive;
	FOnInventoryChanged OnItemChangedEvent;

	virtual void PreSerialize_Implementation(bool bLoading) override;
	virtual void PostSerialize_Implementation(bool bLoading) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
