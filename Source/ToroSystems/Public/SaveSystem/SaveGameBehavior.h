// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DataTypes/TaggedIdentity.h"
#include "ActorBehaviors/ActorBehaviorBase.h"
#include "ActorBehaviors/ActorBehaviorRef.h"
#include "SaveGameBehavior.generated.h"

class USaveGameSubsystem;

/**
 * Transfers the owning actor's SaveGame properties and eligible behaviors to and from the current slot's in-memory record.
 * Requires a sibling UIdentityBehavior with a valid tagged identity identifying that record.
 * Schedules an initial load after BeginPlay and reloads when the subsystem successfully loads a different slot number.
 * Saving is explicit; this component does not perform file I/O or save automatically during EndPlay.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class USaveGameBehavior final : public UActorBehaviorBase
{
	GENERATED_BODY()

public:

	USaveGameBehavior();

	/**
	 * Writes the owner's SaveGame data into its record in the current in-memory slot payload.
	 * Does not write the container to disk.
	 * @return False if the cached subsystem or identity is unavailable, the identity is invalid, or actor serialization fails.
	 */
	UFUNCTION(BlueprintCallable, Category = ActorBehaviors)
	TOROSYSTEMS_API bool SaveActor();

	/**
	 * Applies the owner's record from the current in-memory slot payload without reloading files.
	 * Record lookup creates an empty entry when none exists; failure may leave the actor partially restored.
	 * @return False if the cached subsystem or identity is unavailable, the identity is invalid, or actor deserialization fails.
	 */
	UFUNCTION(BlueprintCallable, Category = ActorBehaviors)
	TOROSYSTEMS_API bool LoadActor();

protected:

	/** Required sibling identity behavior used to select the owner's save record. */
	UPROPERTY(EditAnywhere, Category = Dependencies, DisplayName = "Identity")
	FActorBehaviorRef IdentityRef;

	/** Subsystem cached during BeginPlay and released during EndPlay; not kept alive by this component. */
	TWeakObjectPtr<USaveGameSubsystem> Subsystem;

	/** Reloads the owner after the subsystem successfully loads a different slot number. */
	void OnLoadedDifferentSlot();

	/** Returns the sibling's valid identity, or an empty optional if the component or identity is invalid. */
	TOptional<FTaggedIdentity> GetIdentity();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
