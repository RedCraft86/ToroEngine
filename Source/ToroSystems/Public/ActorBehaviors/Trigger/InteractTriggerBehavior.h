// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Interfaces/ActivatableObject.h"
#include "ActorBehaviors/Trigger/TriggerBehaviorBase.h"
#include "InteractTriggerBehavior.generated.h"

UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class UInteractTriggerBehavior final : public UTriggerBehaviorBase
{
	GENERATED_BODY()

	friend class AToroPlayerCharacter;

public:

	UInteractTriggerBehavior() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bInteractable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FText DisplayLabel = INVTEXT("Interact");

	[[nodiscard]] bool CanBeInteractedWith() const
	{
		return bInteractable && IsActive() && IActivatableObject::GetActiveState(GetOwner());
	}
};
