// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "SaveGameSerializer.h"
#include "GameplayTagContainer.h"
#include "DataTypes/TaggedIdentity.h"
#include "Dom/JsonObject.h"

/** Outcome of a synchronous persistence operation. */
enum class ESaveContainerResult : uint8
{
	Success,

	/** Schema could not be read or differs from the supported version. */
	InvalidSchema,

	/** The requested primary save file does not exist. */
	MissingFile,

	/** Opening, reading, or closing the input file failed. */
	ReadFailed,

	/** Creating, writing, closing, backing up, or replacing the output file failed. */
	WriteFailed,

	/** Removing the primary save or its recovery files failed. */
	DeleteFailed,

	/** Slot metadata is unavailable or could not be decoded. */
	InvalidHeader,

	/** Payload serialization failed or the decoded payload did not match its expected layout. */
	SerializationFailed,

	/** Oodle compression or decompression failed. */
	CompressionFailed,

	/** The slot is outside the supported inclusive range. */
	InvalidSlot,

	Unknown
};

/**
 * Shared synchronous persistence for schema-versioned, Oodle-compressed save payloads.
 * Derived containers supply optional uncompressed metadata and payload serialization.
 * Use containers on the game thread. Save failures may leave recovery files; backups are not loaded automatically.
 */
class TOROSYSTEMS_API FSaveGameContainer
{
public:

	FSaveGameContainer() = delete;
	FSaveGameContainer(uint8 Schema)
		: SchemaVersion(Schema)
	{}

	virtual ~FSaveGameContainer() = default;

protected:

	typedef FSaveGameContainer Super;

	uint8 SchemaVersion;

	[[nodiscard]] ESaveContainerResult LoadData(const FString& FilePath);
	[[nodiscard]] ESaveContainerResult SaveData(const FString& FilePath);

	virtual void SerializeHeader(FArchive& Ar) {}
	virtual void SerializePayload(FArchive& Ar) = 0;
};

/**
 * Cross-playthrough flags and remembered slot selection persisted in Saved/SaveGames/Global.tsav.
 * Mutations affect memory until Save is called. Flags use exact tag membership.
 */
class TOROSYSTEMS_API FGlobalSaveContainer final : public FSaveGameContainer
{
public:

	FGlobalSaveContainer()
		: Super(0)
	{}

	/** Replaces global state on success; missing files and other failures leave current state unchanged. */
	ESaveContainerResult Load();

	/** Persists the current flags and remembered slot selection. */
	ESaveContainerResult Save();

	/** Updates the remembered selection for a valid slot; invalid values, including zero, are ignored. */
	bool SetActiveSlot(uint8 Slot);

	/** Returns the remembered slot, or DefaultSlot when no valid selection is stored. */
	[[nodiscard]] uint8 GetActiveSlot() const;

	/** Adds a valid exact flag; invalid or existing flags have no effect. */
	void AddFlag(const FGameplayTag& Flag);

	/** Tests exact membership; invalid flags return false. */
	[[nodiscard]] bool HasFlag(const FGameplayTag& Flag) const;

private:

	uint8 ActiveSlot = 0;
	TSet<FGameplayTag> Flags;

	virtual void SerializePayload(FArchive& Ar) override;
};

/**
 * One in-memory playthrough with flags and actor records shared across worlds.
 * Explicit slots range from 1 through MaxSaveSlots; zero has no current-slot meaning here.
 * Persistence does not capture or restore live actors; callers transfer actor state through the records.
 * Playtime includes elapsed wall-clock time from construction or successful load and between saves, including pauses.
 * Slot operations do not change the global remembered selection.
 */
class TOROSYSTEMS_API FSlottedSaveContainer final : public FSaveGameContainer
{
public:

	/**
	 * Uncompressed slot information for save-list presentation, independent of the gameplay payload.
	 * Copies share JsonData until explicitly duplicated or replaced.
	 */
	struct FHeader final
	{
		/** Accumulated playtime in seconds at the saved snapshot. */
		double PlaySeconds = 0.0;

		/** UTC timestamp of the saved snapshot; zero denotes an unsaved header. */
		FDateTime SaveTime = FDateTime(0);

		/** Game-specific metadata, serialized as a JSON object string. Must be valid when saving. */
		TSharedPtr<FJsonObject> JsonData = MakeShared<FJsonObject>();

		/** Clears playtime and timestamp and replaces metadata with an empty object. */
		void Reset();
		void Serialize(FArchive& Ar);
	};

	/** Highest valid slot number. */
	static constexpr uint8 MaxSaveSlots = 50;

	/** Fallback selection used by global slot access. */
	static constexpr uint8 DefaultSlot = 1;

	/** Supported schema for slotted saves, including header-only queries. */
	static constexpr uint8 CurrentSchema = 0;

	FSlottedSaveContainer()
		: Super(CurrentSchema)
	{}

	[[nodiscard]] static FORCEINLINE bool IsValidSlot(uint8 Slot)
	{
		return Slot > 0 && Slot <= MaxSaveSlots;
	}

	/** Replaces the playthrough and resets its local timing baseline on success; failure preserves current state. */
	ESaveContainerResult Load(uint8 Slot);

	/**
	 * Saves the current playthrough, accumulating elapsed seconds and updating the saved UTC timestamp.
	 * Failure restores the prior header and existing baseline; an absent baseline is initialized even on failure.
	 * @param ProcessJson Synchronous callback editing an independent metadata copy before persistence; do not reenter container operations.
	 */
	ESaveContainerResult Save(uint8 Slot, TFunctionRef<void(FJsonObject&)> ProcessJson);

	/**
	 * Reads slot metadata without decompressing or validating the gameplay payload.
	 * OutHeader is replaced only on success; no live playthrough state is changed.
	 */
	[[nodiscard]] static ESaveContainerResult LoadSlotHeader(uint8 Slot, FHeader& OutHeader);

	/** Deletes a slot and its recovery files without changing live state; MissingFile means the primary file was absent. */
	static ESaveContainerResult Delete(uint8 Slot);

	/** Adds a valid exact flag; invalid or existing flags have no effect. */
	void AddFlag(const FGameplayTag& Flag);

	/** Tests exact membership; invalid flags return false. */
	[[nodiscard]] bool HasFlag(const FGameplayTag& Flag) const;

	/**
	 * Finds or creates a mutable actor record; returns nullptr for an invalid identity.
	 * Returned pointers may be invalidated by record insertion or successful playthrough loading.
	 */
	[[nodiscard]] FActorSaveRecord* GetActorRecord(const FTaggedIdentity& Identity);

	/** Finds an existing record without creating one; nullptr for absent or invalid identities. Pointer lifetime matches GetActorRecord. */
	[[nodiscard]] const FActorSaveRecord* FindActorRecord(const FTaggedIdentity& Identity) const;

private:

	FHeader SaveHeader;
	TOptional<FDateTime> TimeTracker;

	TSet<FGameplayTag> Flags;
    TMap<FTaggedIdentity, FActorSaveRecord> ActorRecords;

	virtual void SerializeHeader(FArchive& Ar) override;
	virtual void SerializePayload(FArchive& Ar) override;
};
