// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"

class AActor;

/**
 * Stores SaveGame properties for restoration onto an existing object; does not create objects or perform file I/O.
 * Names and soft references are stored as strings without loading referenced assets; hard and weak object pointers are unsupported.
 * Optional save-interface events bracket each transfer. PostSerialize also runs on archive failure and does not indicate success.
 * Capture replaces existing data and may leave a partial payload on failure; use a temporary record to preserve an earlier snapshot.
 */
class TOROSYSTEMS_API FObjectSaveData final
{
	TArray<uint8> Binary;

public:

	/** Shared record containing no serialized properties. */
	static const FObjectSaveData Empty;

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FObjectSaveData& Data)
	{
		return Ar << Data.Binary;
	}

	[[nodiscard]] FORCEINLINE const TArray<uint8>& operator*() const
	{
		return Binary;
	}

	/** Clears the saved properties while retaining allocated byte storage. */
	void Reset()
	{
		Binary.Reset();
	}

	/**
	 * Captures the target's SaveGame properties, replacing the previously saved payload.
	 * Returns false for an invalid target, invalidation by PreSerialize, or an archive error.
	 */
	[[nodiscard]] bool Serialize(UObject* Target);

	/**
	 * Applies saved properties to an existing object, leaving properties absent from the payload unchanged.
	 * Returns false for an invalid target, invalidation by PreSerialize, or an archive error; failure may leave partial changes.
	 */
	[[nodiscard]] bool Deserialize(UObject* Target) const;
};

/**
 * Stores an actor's SaveGame properties and those of its eligible actor behaviors using FObjectSaveData.
 * Behaviors are matched by stable GUIDs, which must be valid and unique within the actor.
 * Restoration uses existing actors and behaviors; missing behavior records leave their targets unchanged and unmatched saved records are ignored.
 * Transfers process the actor before its behaviors, so the actor's PostSerialize event runs before behavior restoration.
 * Capture replaces the record; failed transfers may leave partial saved data or partially restored objects.
 */
class TOROSYSTEMS_API FActorSaveRecord final
{
	FObjectSaveData ActorBinary;
	TMap<FGuid, FObjectSaveData> BehaviorBinary;

public:

	/** Shared record containing no actor or behavior payloads. */
	static const FActorSaveRecord Empty;

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FActorSaveRecord& Data)
	{
		return Ar << Data.ActorBinary << Data.BehaviorBinary;
	}

	[[nodiscard]] FORCEINLINE const FObjectSaveData& operator*() const
	{
		return ActorBinary;
	}

	[[nodiscard]] FORCEINLINE const FObjectSaveData& operator[](const FGuid& BehaviorId) const
	{
		return BehaviorBinary.Contains(BehaviorId) ? BehaviorBinary[BehaviorId] : FObjectSaveData::Empty;
	}

	/** Clears the saved actor properties and all behavior records. */
	void Reset()
	{
		ActorBinary.Reset();
		BehaviorBinary.Empty();
	}

	/**
	 * Captures the actor and its behaviors currently eligible for saving, replacing the previous record.
	 * Returns false for an invalid actor, invalid or duplicate eligible behavior GUIDs, or a failed property transfer.
	 * Behavior failures do not prevent attempts to capture the remaining behaviors.
	 */
	[[nodiscard]] bool Serialize(AActor* Target);

	/**
	 * Restores the actor and matching behaviors currently eligible for saving.
	 * Returns false for an invalid actor, invalid or duplicate eligible behavior GUIDs, or a failed property transfer.
	 * Behavior failures do not prevent attempts to restore the remaining behaviors.
	 */
	[[nodiscard]] bool Deserialize(AActor* Target) const;
};
