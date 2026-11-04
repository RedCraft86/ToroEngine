// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SaveGameData.h"
#include "GameplayTagContainer.h"
#include "DataTypes/TaggedIdentity.h"
#include "Templates/SharedPointer.h"

class FJsonObject;

/**
 * Synchronous persistence for schema-versioned, Oodle-compressed files under Saved/SaveGames.
 * Names are serialized as strings. Unsupported schemas are rejected without migration.
 * Persistence failures report fatal errors; execution can return while PIE shutdown is pending.
 */
class TOROSYSTEMS_API FSaveGameContainer : public TSharedFromThis<FSaveGameContainer>
{
public:

	/** Binary schema version shared by all container types. */
	static constexpr int32 SchemaVersion = 1;

	FSaveGameContainer() = default;

	virtual ~FSaveGameContainer() = default;

protected:

	/**
	 * Reads and decompresses the selected file, validates its schema, and loads its payload.
	 * A missing file clears the payload and succeeds.
	 * @return False on read, decompression, schema, or archive failure if fatal reporting returns.
	 */
	bool LoadData();

	/**
	 * Serializes, compresses, and writes the payload to the selected file.
	 * @return False on archive, compression, or write failure if fatal reporting returns.
	 */
	bool SaveData();

	/** Returns the binary file path for the current selection. */
	virtual FString GetSavePath() const = 0;

	/** Serializes the savable data to/from the archive. */
	virtual void Serialize(FArchive& Ar) = 0;

	/** Clears the in-memory payload without deleting files. */
	virtual void ResetData() = 0;
};

/**
 * Exact gameplay flags shared across save slots and persisted in Global.tsav.
 * Loading a missing file initializes an empty flag set.
 */
class TOROSYSTEMS_API FGlobalSaveContainer final : public FSaveGameContainer
{
public:

	FGlobalSaveContainer() = default;

	/** Loads global flags, or an empty set for a missing file; returns false if fatal reporting returns after failure. */
	bool LoadContainer();

	/** Saves global flags; returns false if fatal reporting returns after a persistence failure. */
	bool SaveContainer();

	/** Adds an exact flag; adding an existing flag has no effect. */
	void AddFlag(const FGameplayTag& InFlag);

	/** Tests exact flag membership without implicit parent or child tag matching. */
	bool HasFlag(const FGameplayTag& InFlag) const;

protected:

	/** Flags persisted independently of slot-specific data. */
	TSet<FGameplayTag> GlobalFlags;

	virtual FString GetSavePath() const override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void ResetData() override;
};

/**
 * Slot-specific flags and actor records in Slot_NN.tsav, with shared JSON metadata in Metadata.tmeta.
 * Construction selects the remembered slot, or slot 1 by default, without loading its binary payload.
 * Binary and metadata writes are separate; later failure does not undo an earlier write or deletion.
 */
class TOROSYSTEMS_API FSlottedSaveContainer final : public FSaveGameContainer
{
public:

	/** Highest supported slot number; persistent slots start at 1. */
	static constexpr uint8 MaxNumSlots = 50;

	FSlottedSaveContainer();

	/** Returns the current selection, or zero after deleting the selected slot. */
	[[nodiscard]] FORCEINLINE uint8 GetActiveSlot() const { return ActiveSlotNum; }

	/** Whether the selected slot is in range; does not imply its payload has been loaded. */
	[[nodiscard]] FORCEINLINE bool IsValidSlot() const { return ActiveSlotNum != 0 && ActiveSlotNum <= MaxNumSlots; }

	/** Whether the selection is valid and loading, flag addition, or record access has marked the payload in use. */
	[[nodiscard]] FORCEINLINE bool IsValidContainer() const { return bContainsData && IsValidSlot(); }

	/**
	 * Reloads metadata and loads a slot's payload, initializing empty data if its binary file is missing.
	 * The first load attempt starts the playtime clock. Failure restores the selection but may leave partial data.
	 * @param InSlot Slot in 1 through MaxNumSlots, or zero to use the current selection.
	 * @return False for an invalid selection or persistence failure if fatal reporting returns.
	 */
	bool LoadContainer(uint8 InSlot);

	/**
	 * Writes the current payload to a slot, then updates its metadata.
	 * Failure restores the previous selection; files and in-memory metadata already changed are not rolled back.
	 * @param InSlot Destination in 1 through MaxNumSlots, or zero to use the current selection.
	 * @param ApplyMetadata Synchronous callback editing slot JSON after standard fields are populated; must not reenter container operations.
	 * @return True only if both binary and metadata persistence succeed.
	 */
	bool SaveContainer(uint8 InSlot, TFunctionRef<void(FJsonObject&)> ApplyMetadata);

	/**
	 * Deletes a slot's binary file and persists removal of its metadata; an already-missing binary is accepted.
	 * Deleting the selected slot clears its payload, selection, and payload-use flag before metadata writing.
	 * @param InSlot Slot in 1 through MaxNumSlots, or zero to use the current selection.
	 * @return False for an invalid selection or persistence failure; metadata failure does not undo binary deletion.
	 */
	bool DeleteContainer(uint8 InSlot);

	/** Adds an exact flag and marks the current payload in use. */
	void AddFlag(const FGameplayTag& InFlag);

	/** Tests exact membership in the current payload without parent or child tag matching. */
	bool HasFlag(const FGameplayTag& InFlag) const;

	/**
	 * Finds or creates an actor record and marks the current payload in use.
	 * @param InIdentity Record key; identity validity is not checked.
	 * @return Mutable record reference, which can be invalidated when payload data is reset or loaded.
	 */
	FActorSaveRecord& GetActorRecord(const FTaggedIdentity& InIdentity);

	/**
	 * Obtains mutable slot JSON from the current metadata root without file I/O.
	 * @param InSlot Explicit slot in 1 through MaxNumSlots; zero is not a current-slot shorthand here.
	 * @param bCreateIfNeeded Creates or replaces a missing or non-object entry when true, even on a const container.
	 * @return Null for invalid slots, unavailable metadata, or absent entries when creation is disabled.
	 * Changes require subsequent metadata persistence; returned objects can become detached after a metadata reload.
	 */
	TSharedPtr<FJsonObject> GetSlotMetadata(uint8 InSlot, bool bCreateIfNeeded) const;

protected:

	/** Payload-use flag set by successful loading, flag addition, or record access; cleared on selected-slot deletion. */
	bool bContainsData = false;

	/** Current or remembered slot; zero denotes no selection. */
	uint8 ActiveSlotNum = 0;

	/** UTC playtime baseline initialized on first load attempt and advanced after successful metadata saves. */
	FDateTime LastSaveTime;

	/** Shared JSON root containing the remembered selection and per-slot metadata. */
	TSharedPtr<FJsonObject> Metadata;

	/** Exact gameplay flags for the current in-memory slot payload. */
	TSet<FGameplayTag> SlotFlags;

	/** Actor payloads keyed by tagged identity. */
	TMap<FTaggedIdentity, FActorSaveRecord> ActorRecords;

	/**
	 * Reloads metadata when present, restoring the remembered selection only if none is selected.
	 * Missing or unusable remembered selections fall back to slot 1, including when the file is missing.
	 * @return False on JSON or read failure if fatal reporting returns.
	 */
	bool LoadMetadata();

	/**
	 * Writes the current JSON root without creating slot entries or changing the playtime baseline.
	 * @return False on JSON or write failure if fatal reporting returns.
	 */
	bool WriteMetadata() const;

	/**
	 * Updates standard slot fields, invokes the callback, and writes metadata before advancing the playtime baseline.
	 * Playtime accumulates from the existing baseline; a save with no baseline establishes one without adding elapsed time.
	 * @param ModifySlotInfo Synchronous callback editing the active slot JSON; must not reenter container operations.
	 * @return False if slot metadata cannot be obtained or written.
	 */
	bool SaveMetadata(TFunctionRef<void(FJsonObject&)> ModifySlotInfo);

	static FString GetMetadataPath();
	virtual FString GetSavePath() const override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void ResetData() override;
};
