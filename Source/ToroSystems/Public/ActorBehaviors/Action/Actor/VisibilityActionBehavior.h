// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPtr.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "VisibilityActionBehavior.generated.h"

/**
 * Sets or toggles visibility of existing target actors through their hidden-in-game state.
 * Unavailable targets are skipped; referenced actors are not loaded by this action.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class UVisibilityActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

	/** Existing actors whose visibility will change. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSet<TSoftObjectPtr<AActor>> Targets;

	/** Whether to set the configured state or toggle each target's current state. */
	UPROPERTY(EditAnywhere, Category = Settings)
	EToroActionApplyType ApplyType = EToroActionApplyType::Set;

	/** Whether targets should be visible when setting their state. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (EditCondition = "ApplyType == EToroActionApplyType::Set", EditConditionHides))
	bool bVisible = false;

	virtual void OnTriggered() override
	{
		for (const TSoftObjectPtr<AActor>& Target : Targets)
		{
			if (AActor* Actor = Target.Get(); IsValid(Actor))
			{
				Actor->SetActorHiddenInGame(ApplyType == EToroActionApplyType::Set
					? !bVisible : !Actor->IsHidden());
			}
		}
	}
};
