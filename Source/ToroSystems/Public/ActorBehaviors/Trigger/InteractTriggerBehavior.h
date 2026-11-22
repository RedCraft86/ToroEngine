// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

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

	/** Require at least one action bound to this trigger that can accept an activation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bRequireCallableAction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FText DisplayLabel = INVTEXT("Interact");

	/** Whether the player can interact with this trigger now. */
	[[nodiscard]] TOROSYSTEMS_API bool CanBeInteractedWith() const;
};
