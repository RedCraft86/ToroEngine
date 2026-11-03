// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "SaveSystem/SaveGameContainer.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/NameAsStringProxyArchive.h"
#include "Libraries/ToroUtilityLibrary.h"
#include "Utilities/ToroLogging.h"

namespace
{
	FString GetSaveDirectory()
	{
		static const FString RootPath = FPaths::ProjectSavedDir() / TEXT("SaveGames");
		return RootPath;
	}

	bool ValidateSlot(uint8 InSlot)
	{
		if (InSlot == 0 || InSlot > FSlottedSaveContainer::MaxNumSlots)
		{
			TORO_REPORT_ERROR(TEXT("Save slot %d must be between 1 and %d."),
				InSlot, FSlottedSaveContainer::MaxNumSlots);
			return false;
		}

		return true;
	}

	FString GetSlotPath(uint8 InSlot)
	{
		return GetSaveDirectory() / FString::Printf(TEXT("Slot_%02d.tsav"), InSlot);
	}

	FString GetSlotKey(uint8 InSlot)
	{
		return FString::Printf(TEXT("Slot_%02d"), InSlot);
	}

	namespace JsonFields
	{
		const FString Schema = TEXT("Schema");
		const FString ActiveSlot = TEXT("ActiveSlot");
		const FString SaveTime = TEXT("SaveTime");
		const FString PlayTime = TEXT("PlayTime");
	}
}

bool FSaveGameContainer::LoadData()
{
	const FString FilePath = GetSavePath();
	if (!FPaths::FileExists(FilePath))
	{
		ResetData();
		return true;
	}

	TArray<uint8> CompressedData;
	if (!FFileHelper::LoadFileToArray(CompressedData, *FilePath))
	{
		TORO_REPORT_FATAL(TEXT("Failed to read data from %s"), *FilePath);
		return false;
	}

	TArray<uint8> BinaryData;
	if (!UToroUtilityLibrary::OodleDecompress(CompressedData, BinaryData))
	{
		TORO_REPORT_FATAL(TEXT("Failed to decompress binary data"));
		return false;
	}

	FMemoryReader Reader(BinaryData, true);
	FNameAsStringProxyArchive Ar(Reader);

	int32 Version = 0;
	Reader << Version;
	if (Reader.IsError())
	{
		TORO_REPORT_FATAL(TEXT("Failed to read save schema version"));
		return false;
	}

	if (Version != SchemaVersion)
	{
		TORO_REPORT_FATAL(TEXT("Unsupported save schema: expected %d, got %d."), SchemaVersion, Version);
		return false;
	}

	Serialize(Ar);

	if (Ar.IsError())
	{
		TORO_REPORT_FATAL(TEXT("Failed to deserialize binary to data"));
		return false;
	}

	return true;
}

bool FSaveGameContainer::SaveData()
{
	TArray<uint8> BinaryData;
	FMemoryWriter Writer(BinaryData, true);
	FNameAsStringProxyArchive Ar(Writer);

	int32 Version = SchemaVersion;
	Ar << Version;
	Serialize(Ar);

	if (Ar.IsError())
	{
		TORO_REPORT_FATAL(TEXT("Failed to serialize data to binary"));
		return false;
	}

	TArray<uint8> CompressedData;
	if (!UToroUtilityLibrary::OodleCompress(BinaryData, CompressedData))
	{
		TORO_REPORT_FATAL(TEXT("Failed to compress binary data"));
		return false;
	}

	const FString FilePath = GetSavePath();
	if (!FFileHelper::SaveArrayToFile(CompressedData, *FilePath))
	{
		TORO_REPORT_FATAL(TEXT("Failed to write data to %s"), *FilePath);
		return false;
	}

	return true;
}

bool FGlobalSaveContainer::LoadContainer()
{
	return LoadData();
}

bool FGlobalSaveContainer::SaveContainer()
{
	return SaveData();
}

void FGlobalSaveContainer::AddFlag(const FGameplayTag& InFlag)
{
	GlobalFlags.Add(InFlag);
}

bool FGlobalSaveContainer::HasFlag(const FGameplayTag& InFlag) const
{
	return GlobalFlags.Contains(InFlag);
}

FString FGlobalSaveContainer::GetSavePath() const
{
	return GetSaveDirectory() / TEXT("Global.tsav");
}

void FGlobalSaveContainer::Serialize(FArchive& Ar)
{
	Ar << GlobalFlags;
}

void FGlobalSaveContainer::ResetData()
{
	GlobalFlags.Empty();
}

FSlottedSaveContainer::FSlottedSaveContainer()
	: Metadata(MakeShared<FJsonObject>())
{
	LoadMetadata();
}

bool FSlottedSaveContainer::LoadContainer(uint8 InSlot)
{
	if (LastSaveTime.GetTicks() == 0)
	{
		LastSaveTime = FDateTime::UtcNow();
	}

	InSlot = InSlot == 0 ? ActiveSlotNum : InSlot;
	if (!ValidateSlot(InSlot))
	{
		return false;
	}

	const uint8 OldSlot = ActiveSlotNum;
	ActiveSlotNum = InSlot;

	if (!LoadMetadata() || !LoadData())
	{
		ActiveSlotNum = OldSlot;
		return false;
	}

	bContainsData = true;
	return true;
}

bool FSlottedSaveContainer::SaveContainer(uint8 InSlot, TFunctionRef<void(FJsonObject&)> ApplyMetadata)
{
	InSlot = InSlot == 0 ? ActiveSlotNum : InSlot;
	if (!ValidateSlot(InSlot))
	{
		return false;
	}

	const uint8 OldSlot = ActiveSlotNum;
	ActiveSlotNum = InSlot;

	if (!SaveData() || !SaveMetadata(ApplyMetadata))
	{
		ActiveSlotNum = OldSlot;
		return false;
	}

	return true;
}

bool FSlottedSaveContainer::DeleteContainer(uint8 InSlot)
{
	InSlot = InSlot == 0 ? ActiveSlotNum : InSlot;
	if (!ValidateSlot(InSlot))
	{
		return false;
	}

	if (!Metadata.IsValid())
	{
		TORO_REPORT_FATAL(TEXT("Metadata json object is null"));
		return false;
	}

	const FString FilePath = GetSlotPath(InSlot);

	if (FPaths::FileExists(FilePath) && !IFileManager::Get().Delete(*FilePath))
	{
		TORO_REPORT_FATAL(TEXT("Failed to delete save file %s"), *FilePath);
		return false;
	}

	Metadata->RemoveField(GetSlotKey(InSlot));

	if (ActiveSlotNum == InSlot)
	{
		ResetData();
		Metadata->SetNumberField(JsonFields::ActiveSlot, 0);
		bContainsData = false;
		ActiveSlotNum = 0;
	}

	return WriteMetadata();
}

void FSlottedSaveContainer::AddFlag(const FGameplayTag& InFlag)
{
	bContainsData = true;
	SlotFlags.Add(InFlag);
}

bool FSlottedSaveContainer::HasFlag(const FGameplayTag& InFlag) const
{
	return SlotFlags.Contains(InFlag);
}

FActorSaveRecord& FSlottedSaveContainer::GetActorRecord(const FTaggedIdentity& InIdentity)
{
	bContainsData = true;
	return ActorRecords.FindOrAdd(InIdentity);
}

TSharedPtr<FJsonObject> FSlottedSaveContainer::GetSlotMetadata(uint8 InSlot, bool bCreateIfNeeded) const
{
	if (!Metadata.IsValid())
	{
		TORO_REPORT_FATAL(TEXT("Metadata json object is null"));
		return nullptr;
	}

	if (!ValidateSlot(InSlot))
	{
		return nullptr;
	}

	const TSharedPtr<FJsonObject>* PossibleSlot = nullptr;
	const FString SlotKey = GetSlotKey(InSlot);
	if (Metadata->TryGetObjectField(SlotKey, PossibleSlot) && PossibleSlot->IsValid())
	{
		return *PossibleSlot;
	}
	if (!bCreateIfNeeded)
	{
		return nullptr;
	}

	const TSharedPtr<FJsonObject> SlotObject = MakeShared<FJsonObject>();
	Metadata->SetObjectField(SlotKey, SlotObject);
	return SlotObject;
}

bool FSlottedSaveContainer::LoadMetadata()
{
	if (!Metadata.IsValid())
	{
		TORO_REPORT_FATAL(TEXT("Metadata json object is null"));
		return false;
	}

	const FString FilePath = GetMetadataPath();
	if (FPaths::FileExists(FilePath))
	{
		FString JsonStr;
		if (!FFileHelper::LoadFileToString(JsonStr, *FilePath))
		{
			TORO_REPORT_FATAL(TEXT("Failed to read metadata from %s"), *FilePath);
			return false;
		}

		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonStr);
		if (!FJsonSerializer::Deserialize(JsonReader, Metadata) || !Metadata.IsValid())
		{
			TORO_REPORT_FATAL(TEXT("Failed to deserialize metadata json"));
			return false;
		}
	}

	if (ActiveSlotNum == 0)
	{
		if (!Metadata->TryGetNumberField(JsonFields::ActiveSlot, ActiveSlotNum)
			|| ActiveSlotNum == 0 || ActiveSlotNum > MaxNumSlots)
		{
			ActiveSlotNum = 1;
		}
	}
	else
	{
		Metadata->SetNumberField(JsonFields::ActiveSlot, ActiveSlotNum);
	}

	return true;
}

bool FSlottedSaveContainer::WriteMetadata() const
{
	if (!Metadata.IsValid())
	{
		TORO_REPORT_FATAL(TEXT("Metadata json object is null"));
		return false;
	}

	FString JsonStr;
	const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonStr, 0);
	if (!FJsonSerializer::Serialize(Metadata.ToSharedRef(), JsonWriter, true))
	{
		TORO_REPORT_FATAL(TEXT("Failed to serialize metadata json"));
		return false;
	}

	const FString FilePath = GetMetadataPath();
	if (!FFileHelper::SaveStringToFile(JsonStr, *FilePath))
	{
		TORO_REPORT_FATAL(TEXT("Failed to write metadata to %s"), *FilePath);
		return false;
	}

	return true;
}

bool FSlottedSaveContainer::SaveMetadata(TFunctionRef<void(FJsonObject&)> ModifySlotInfo)
{
	const TSharedPtr<FJsonObject> SlotObject = GetSlotMetadata(ActiveSlotNum, true);
	if (!SlotObject.IsValid())
	{
		return false;
	}

	Metadata->SetNumberField(JsonFields::ActiveSlot, ActiveSlotNum);

	const FDateTime TimeNow = FDateTime::UtcNow();
	SlotObject->SetNumberField(JsonFields::Schema, SchemaVersion);
	SlotObject->SetStringField(JsonFields::SaveTime, TimeNow.ToIso8601());

	float PlaySeconds = 0.0f;
	SlotObject->TryGetNumberField(JsonFields::PlayTime, PlaySeconds);
	if (LastSaveTime.GetTicks() != 0)
	{
		PlaySeconds += (TimeNow - LastSaveTime).GetTotalSeconds();
		SlotObject->SetNumberField(JsonFields::PlayTime, PlaySeconds);
	}

	ModifySlotInfo(*SlotObject);
	if (!WriteMetadata())
	{
		return false;
	}

	LastSaveTime = TimeNow;
	return true;
}

FString FSlottedSaveContainer::GetMetadataPath()
{
	return GetSaveDirectory() / TEXT("Metadata.tmeta");
}

FString FSlottedSaveContainer::GetSavePath() const
{
	return GetSlotPath(ActiveSlotNum);
}

void FSlottedSaveContainer::Serialize(FArchive& Ar)
{
	Ar << SlotFlags;
	Ar << ActorRecords;
}

void FSlottedSaveContainer::ResetData()
{
	SlotFlags.Empty();
	ActorRecords.Empty();
}
