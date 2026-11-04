// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPtr.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "CollisionActionBehavior.generated.h"

/**
 * Sets or toggles actor collision enablement for existing target actors.
 * Unavailable targets are skipped; component collision settings are otherwise preserved.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class UCollisionActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

	/** Existing actors whose collision enablement will change. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSet<TSoftObjectPtr<AActor>> Targets;

	/** Whether to set the configured state or toggle each target's current state. */
	UPROPERTY(EditAnywhere, Category = Settings)
	EToroActionApplyType ApplyType = EToroActionApplyType::Set;

	/** Whether actor collision should be enabled when setting the state. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (EditCondition = "ApplyType == EToroActionApplyType::Set", EditConditionHides))
	bool bCollisionEnabled = false;

	virtual void OnTriggered() override
	{
		for (const TSoftObjectPtr<AActor>& Target : Targets)
		{
			if (AActor* Actor = Target.Get(); IsValid(Actor))
			{
				Actor->SetActorEnableCollision(ApplyType == EToroActionApplyType::Set
					? bCollisionEnabled : !Actor->GetActorEnableCollision());
			}
		}
	}
};
