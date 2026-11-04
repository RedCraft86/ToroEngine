// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPtr.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "ActorTagsActionBehavior.generated.h"

/**
 * Adds unique FName actor tags, then removes configured tags from existing target actors.
 * Removal wins when a tag appears in both sets. These are ordinary actor tags, not gameplay tags.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class UActorTagsActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

	/** Existing actors whose tag arrays will change. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSet<TSoftObjectPtr<AActor>> Targets;

	/** Tags to add without creating duplicate entries. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSet<FName> AddTags;

	/** Tags to remove after additions. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSet<FName> RemoveTags;

	virtual void OnTriggered() override
	{
		for (const TSoftObjectPtr<AActor>& Target : Targets)
		{
			if (AActor* Actor = Target.Get(); IsValid(Actor))
			{
				for (const FName& Tag : AddTags)
				{
					Actor->Tags.AddUnique(Tag);
				}
				for (const FName& Tag : RemoveTags)
				{
					Actor->Tags.Remove(Tag);
				}
			}
		}
	}
};
