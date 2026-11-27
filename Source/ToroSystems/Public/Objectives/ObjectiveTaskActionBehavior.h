// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameplayTagContainer.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "ObjectiveTaskActionBehavior.generated.h"

/** Adds progress to an objective count task. */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Misc] Advance Count Task")
class TOROSYSTEMS_API UObjectiveTaskActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, Category = Settings, meta = (Categories = "Objective"))
	FGameplayTag Task;

	/** Progress added per trigger for a count task. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 1, UIMin = 1))
	uint8 Amount = 1;

	virtual bool CanTrigger() const override;
	virtual void OnTriggered() override;
};
