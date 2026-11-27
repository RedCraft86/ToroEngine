// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFlags/GameFlagManager.h"
#include "GameplayTagContainer.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "GameFlagActionBehavior.generated.h"

/** Sets a game flag in its configured scope. */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Misc] Set Game Flag")
class TOROSYSTEMS_API UGameFlagActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, Category = Settings)
	FGameplayTag Flag;

	UPROPERTY(EditAnywhere, Category = Settings)
	EGameFlagScope Scope = EGameFlagScope::SlotSave;

	virtual bool CanTrigger() const override;
	virtual void OnTriggered() override;
};
