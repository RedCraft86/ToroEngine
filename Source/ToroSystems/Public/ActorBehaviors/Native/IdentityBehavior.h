// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DataTypes/ToroIdentity.h"
#include "ActorBehaviors/ActorBehaviorBase.h"
#include "IdentityBehavior.generated.h"

/**
 * Exposes an identity for the owning actor with a manually configured group.
 * Construction generates a GUID only when missing, preserving existing valid GUIDs.
 * Uniqueness across duplicated actors is not enforced.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class UIdentityBehavior final : public UActorBehaviorBase
{
	GENERATED_BODY()

public:

	UIdentityBehavior() {}

	/** Returns the stored identity by const reference; it may be invalid. */
	UFUNCTION(BlueprintPure, Category = ActorBehaviors)
	[[nodiscard]] TOROSYSTEMS_API const FToroIdentity& GetIdentity() const
	{
		return Identity;
	}

private:

	/**
	 * Identity requiring a manually assigned valid group; construction generates a missing GUID.
	 * Existing valid GUIDs are preserved, so the author must ensure uniqueness across duplicated actors.
	 */
	UPROPERTY(EditAnywhere, Category = Settings)
	FToroIdentity Identity;

	virtual void OnConstruction() override
	{
		Super::OnConstruction();
		Identity.RegenerateGuid(true);
	}
};
