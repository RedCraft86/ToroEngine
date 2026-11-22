// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Inventory/EquipmentActor.h"
#include "Components/SceneComponent.h"
#include "Framework/ToroPlayerCharacter.h"

bool AEquipmentActor::PrepareEquipment_Implementation(AActor* InventoryOwner)
{
	if (!IsValid(InventoryOwner))
	{
		return false;
	}

	if (const AToroPlayerCharacter* Player = Cast<AToroPlayerCharacter>(InventoryOwner))
	{
		USceneComponent* Mount = Player->GetEquipmentRoot();
		USceneComponent* EquipmentRoot = GetRootComponent();
		if (!Mount || !EquipmentRoot
			|| !EquipmentRoot->AttachToComponent(Mount, FAttachmentTransformRules::KeepRelativeTransform))
		{
			return false;
		}

		EquipmentRoot->SetRelativeTransform(CameraOffset);
		if (Player->IsHidden() || Mount->bHiddenInGame)
		{
			EquipmentRoot->SetHiddenInGame(true, true);
		}
	}

	return true;
}
