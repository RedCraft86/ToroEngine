// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SaveGameData.h"
#include "GameplayTagContainer.h"
#include "DataTypes/TaggedIdentity.h"
#include "Templates/SharedPointer.h"

class FJsonObject;

class TOROSYSTEMS_API FSaveGameContainer : public TSharedFromThis<FSaveGameContainer>
{
public:

	static constexpr int32 SchemaVersion = 1;

	FSaveGameContainer() = default;

	virtual ~FSaveGameContainer() = default;

protected:

	bool LoadData();
	bool SaveData();
	virtual FString GetSavePath() const = 0;
	virtual void Serialize(FArchive& Ar) = 0;
	virtual void ResetData() = 0;
};

class TOROSYSTEMS_API FGlobalSaveContainer final : public FSaveGameContainer
{
public:

	FGlobalSaveContainer() = default;

	bool LoadContainer();
	bool SaveContainer();

	void AddFlag(const FGameplayTag& InFlag);
	bool HasFlag(const FGameplayTag& InFlag) const;

protected:

	TSet<FGameplayTag> GlobalFlags;

	virtual FString GetSavePath() const override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void ResetData() override;
};

class TOROSYSTEMS_API FSlottedSaveContainer final : public FSaveGameContainer
{
public:

	static constexpr uint8 MaxNumSlots = 50;

	FSlottedSaveContainer();

	[[nodiscard]] FORCEINLINE uint8 GetActiveSlot() const { return ActiveSlotNum; }
	[[nodiscard]] FORCEINLINE bool IsValidSlot() const { return ActiveSlotNum != 0 && ActiveSlotNum <= MaxNumSlots; }
	[[nodiscard]] FORCEINLINE bool IsValidContainer() const { return bContainsData && IsValidSlot(); }

	bool LoadContainer(uint8 InSlot);
	bool SaveContainer(uint8 InSlot, TFunctionRef<void(FJsonObject&)> ApplyMetadata);

	bool DeleteContainer(uint8 InSlot);

	void AddFlag(const FGameplayTag& InFlag);
	bool HasFlag(const FGameplayTag& InFlag) const;

	FActorSaveRecord& GetActorRecord(const FTaggedIdentity& InIdentity);

	TSharedPtr<FJsonObject> GetSlotMetadata(uint8 InSlot, bool bCreateIfNeeded) const;

protected:

	bool bContainsData = false;
	uint8 ActiveSlotNum = 0;
	FDateTime LastSaveTime;
	TSharedPtr<FJsonObject> Metadata;

	TSet<FGameplayTag> SlotFlags;
	TMap<FTaggedIdentity, FActorSaveRecord> ActorRecords;

	bool LoadMetadata();
	bool WriteMetadata() const;
	bool SaveMetadata(TFunctionRef<void(FJsonObject&)> ModifySlotInfo);

	static FString GetMetadataPath();
	virtual FString GetSavePath() const override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void ResetData() override;
};
