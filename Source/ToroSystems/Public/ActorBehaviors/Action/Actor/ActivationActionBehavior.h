// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPtr.h"
#include "Interfaces/ActivatableObject.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "ActivationActionBehavior.generated.h"

/**
 * Sets or toggles activation through IActivatableObject, including Blueprint implementations.
 * Unavailable or non-implementing actors are ignored; targets own their activation state.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class UActivationActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

	/** Existing actors implementing the activation interface. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (MustImplement = "/Script/ToroEngine.ActivatableObject"))
	TSet<TSoftObjectPtr<AActor>> Targets;

	/** Whether to set the configured state or toggle each target's current state. */
	UPROPERTY(EditAnywhere, Category = Settings)
	EToroActionApplyType ApplyType = EToroActionApplyType::Set;

	/** Requested activation state when setting rather than toggling. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (EditCondition = "ApplyType == EToroActionApplyType::Set", EditConditionHides))
	bool bActive = false;

	virtual void OnTriggered() override
	{
		for (const TSoftObjectPtr<AActor>& Target : Targets)
		{
			if (AActor* Actor = Target.Get(); IsValid(Actor))
			{
				IActivatableObject::SetActiveState(Actor, ApplyType == EToroActionApplyType::Set
					? bActive : !IActivatableObject::GetActiveState(Actor));
			}
		}
	}
};
