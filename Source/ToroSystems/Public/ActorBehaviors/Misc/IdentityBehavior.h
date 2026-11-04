// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DataTypes/TaggedIdentity.h"
#include "ActorBehaviors/ActorBehaviorBase.h"
#include "IdentityBehavior.generated.h"

/**
 * Exposes a mutable identity for the owning actor with a manually configured group.
 * Construction generates a random GUID only when missing, preserving existing valid
 * GUIDs and leaving the group unchanged. Callers can assign a deterministic identity
 * when replacement actors must share the same persistent identity.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class UIdentityBehavior final : public UActorBehaviorBase
{
	GENERATED_BODY()

public:

	/**
	 * Caller-editable identity; consumers determine whether it meets their requirements.
	 * Construction fills an invalid GUID with a random GUID without assigning a group.
	 * Assign distinct identities when actors of the same class require separate save records.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FTaggedIdentity Identity;

private:

	virtual void OnConstruction() override
	{
		Super::OnConstruction();
		Identity.RegenerateGuid();
	}
};
