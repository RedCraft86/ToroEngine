// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DataTypes/TaggedIdentity.h"
#include "ActorBehaviors/ActorBehaviorBase.h"
#include "IdentityBehavior.generated.h"

/**
 * Exposes a mutable identity for the owning actor with a manually configured group.
 * Construction derives the GUID from a nonempty determinant, or generates a random
 * GUID only when missing. The group is unchanged. Use the same group and determinant
 * when replacement actors must share the same persistent identity.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class UIdentityBehavior final : public UActorBehaviorBase
{
	GENERATED_BODY()

public:

	/**
	 * Caller-editable identity; consumers determine whether it meets their requirements.
	 * Construction updates the GUID according to Determinant without assigning a group.
	 * Assign distinct identities when actors of the same class require separate save records.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FTaggedIdentity Identity;

	/**
	 * Stable string used to derive the GUID during construction, replacing any existing GUID.
	 * Empty or whitespace-only values preserve valid GUIDs and generate random ones when missing.
	 * Nonempty values are hashed exactly as supplied, including surrounding whitespace.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings)
	FString Determinant;

	/**
	 * Causes the identity to be rebuilt.
	 * Used if the component is spawned and determinant is adjusted in cpp.
	 */
	void RebuildIdentity()
	{
		Identity.RegenerateGuid(false, Determinant);
	}

private:

	virtual void OnConstruction() override
	{
		Super::OnConstruction();
		Identity.RegenerateGuid(Determinant.TrimStartAndEnd().IsEmpty(), Determinant);
	}
};
