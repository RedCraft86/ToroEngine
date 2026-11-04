// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Misc/Guid.h"
#include "CoreMinimal.h"
#include "Serialization/Archive.h"

class AActor;

/**
 * Binary SaveGame property payload for an existing object.
 * Uses string-based names and soft references; weak and hard object pointers are unsupported.
 * Optional save-interface notifications bracket property serialization and do not imply success.
 */
struct TOROSYSTEMS_API FObjectSaveData final
{
	/** Serialized SaveGame properties; may contain partial data after a failed save. */
	TArray<uint8> Binary;

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FObjectSaveData& Data)
	{
		return Ar << Data.Binary;
	}

	/** Clears the payload while retaining its allocated storage. */
	void Reset() { Binary.Reset(); }

	/**
	 * Replaces the payload with the target's SaveGame properties.
	 * Sends optional pre/post notifications; the post notification runs even if the archive fails.
	 * Existing data is cleared before target validation, and failure may leave a partial payload.
	 * @param Target Existing object to save; must remain valid after its pre-serialization notification.
	 * @return True if the target remains valid through preparation and serialization reports no archive error.
	 */
	[[nodiscard]] bool Serialize(UObject* Target);

	/**
	 * Applies the payload to an existing object's SaveGame properties.
	 * Sends optional pre/post notifications; the post notification runs even if the archive fails.
	 * Failure may leave the target partially modified; changes are not rolled back.
	 * @param Target Existing object to restore; must remain valid after its pre-serialization notification.
	 * @return True if the target remains valid through preparation and deserialization reports no archive error.
	 */
	[[nodiscard]] bool Deserialize(UObject* Target) const;
};

/**
 * SaveGame property payloads for an actor and its eligible behavior components.
 * Component payloads are matched to existing components by valid, unique behavior GUIDs.
 * Loading does not create components and skips eligible components without saved records.
 * Operations are not transactional: failures may leave partial records or partially restored objects.
 */
struct TOROSYSTEMS_API FActorSaveRecord final
{
	/** SaveGame property payload for the actor itself. */
	FObjectSaveData ActorData;

	/** Eligible behavior component payloads keyed by their behavior GUIDs. */
	TMap<FGuid, FObjectSaveData> ComponentData;

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FActorSaveRecord& Record)
	{
		return Ar << Record.ActorData << Record.ComponentData;
	}

	/**
	 * Clears the record and saves the actor followed by its eligible behavior components.
	 * Eligibility is determined by UActorBehaviorBase::EligibleForSaving.
	 * @param Target Existing actor to save.
	 * @return False for an invalid target, invalid or duplicate eligible behavior GUID, or failed object save.
	 */
	[[nodiscard]] bool Serialize(AActor* Target);

	/**
	 * Restores the actor followed by matching eligible behavior components.
	 * Missing component records and saved records without an eligible target component are ignored.
	 * Valid, unique GUIDs are required for all eligible components, even those without saved records.
	 * @param Target Existing actor to restore.
	 * @return False for an invalid target, invalid or duplicate eligible behavior GUID, or failed object load.
	 */
	[[nodiscard]] bool Deserialize(AActor* Target) const;
};
