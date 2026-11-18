// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ActorBehaviors/ActorBehaviorRef.h"
#include "ActorBehaviors/ActorBehaviorBase.h"
#include "SaveGameBehavior.generated.h"

class USaveGameSubsystem;

/**
 * Transfers the owner's SaveGame properties and eligible actor behaviors to and from the current in-memory playthrough.
 * Requires a sibling identity behavior with a stable, valid identity; eligible behaviors are matched by their GUIDs.
 * Registers with the save subsystem during play and schedules an initial restore after BeginPlay returns.
 * Initial restore ordering across actors is not guaranteed. Neither transfer performs file I/O or level travel.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class USaveGameBehavior final : public UActorBehaviorBase
{
	GENERATED_BODY()

public:

	USaveGameBehavior();

	/**
	 * Captures the owner and its eligible behaviors, replacing its in-memory record only on success.
	 * Returns false if the subsystem, container, owner, or identity is unavailable, or capture fails.
	 * Call the subsystem's slot save operation to persist the record to disk.
	 */
	UFUNCTION(BlueprintCallable, Category = ActorBehaviors)
	TOROSYSTEMS_API bool SaveActor();

	/**
	 * Restores the owner's existing record without reloading the save file.
	 * A missing record succeeds without changing the actor or creating an entry.
	 * Returns false for unavailable dependencies, an invalid identity, or failed restoration; restoration failure may leave partial changes.
	 */
	UFUNCTION(BlueprintCallable, Category = ActorBehaviors)
	TOROSYSTEMS_API bool LoadActor();

protected:

	/** Required sibling identity behavior selecting the owner's slot-wide save record. */
	UPROPERTY(EditAnywhere, Category = Dependencies, DisplayName = "Identity")
	FActorBehaviorRef IdentityRef;

	TWeakObjectPtr<USaveGameSubsystem> Subsystem;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
