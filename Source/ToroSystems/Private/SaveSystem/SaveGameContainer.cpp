// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "SaveSystem/SaveGameContainer.h"
#include "Libraries/ToroUtilityLibrary.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/NameAsStringProxyArchive.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Utilities/ToroLogging.h"
#include "HAL/FileManager.h"
#include "Misc/ScopeExit.h"
#include "Misc/Paths.h"

namespace
{
	FString SaveDirectory()
	{
		return FPaths::ProjectSavedDir() / TEXT("SaveGames");
	}

	FString GlobalPath()
	{
		return SaveDirectory() / TEXT("Global.tsav");
	}

	FString SlotPath(uint8 Slot)
	{
		return SaveDirectory() / FString::Printf(TEXT("Slot_%d.tsav"), Slot);
	}
}

ESaveContainerResult FSaveGameContainer::LoadData(const FString& FilePath)
{
	IFileManager& Files = IFileManager::Get();
	if (!Files.FileExists(*FilePath))
	{
		return ESaveContainerResult::MissingFile;
	}

	TUniquePtr<FArchive> Reader(Files.CreateFileReader(*FilePath));
	if (!Reader.IsValid() || Reader->IsError())
	{
		return ESaveContainerResult::ReadFailed;
	}

	uint8 SavedSchema;
	*Reader << SavedSchema;
	if (Reader->IsError() || SavedSchema != SchemaVersion)
	{
		return ESaveContainerResult::InvalidSchema;
	}

	SerializeHeader(*Reader);
	if (Reader->IsError())
	{
		return ESaveContainerResult::InvalidHeader;
	}

	TArray<uint8> Compressed;
	*Reader << Compressed;
	if (!Reader->Close() || Reader->IsError())
	{
		return ESaveContainerResult::ReadFailed;
	}

	TArray<uint8> Payload;
	if (!UToroUtilityLibrary::OodleDecompress(Compressed, Payload))
	{
		return ESaveContainerResult::CompressionFailed;
	}

	FMemoryReader PayloadReader(Payload, true);
	FNameAsStringProxyArchive Ar(PayloadReader);
	SerializePayload(Ar);

	return (Ar.IsError() || PayloadReader.Tell() != PayloadReader.TotalSize())
		? ESaveContainerResult::SerializationFailed : ESaveContainerResult::Success;
}

ESaveContainerResult FSaveGameContainer::SaveData(const FString& FilePath)
{
	TArray<uint8> Payload;
	{
		FMemoryWriter PayloadWriter(Payload, true);
		FNameAsStringProxyArchive Ar(PayloadWriter);
		SerializePayload(Ar);
		if (Ar.IsError())
		{
			return ESaveContainerResult::SerializationFailed;
		}
	}

	TArray<uint8> Compressed;
	if (!UToroUtilityLibrary::OodleCompress(Payload, Compressed))
	{
		return ESaveContainerResult::CompressionFailed;
	}

	IFileManager& Files = IFileManager::Get();
	if (!Files.MakeDirectory(*FPaths::GetPath(FilePath), true))
	{
		return ESaveContainerResult::WriteFailed;
	}

	const FString TempPath = FilePath + TEXT(".tmp");
	const FString BackupPath = FilePath + TEXT(".bak");

	TUniquePtr<FArchive> Writer(Files.CreateFileWriter(*TempPath));
	bool bPreserveTempFile = false;
	ON_SCOPE_EXIT
	{
		Writer.Reset();
		if (!bPreserveTempFile)
		{
			Files.Delete(*TempPath, false, true);
		}
	};

	if (!Writer.IsValid() || Writer->IsError())
	{
		return ESaveContainerResult::WriteFailed;
	}

	*Writer << SchemaVersion;
	if (Writer->IsError())
	{
		return ESaveContainerResult::WriteFailed;
	}

	SerializeHeader(*Writer);
	if (Writer->IsError())
	{
		return ESaveContainerResult::WriteFailed;
	}

	*Writer << Compressed;
	if (!Writer->Close())
	{
		return ESaveContainerResult::WriteFailed;
	}

	if (Files.FileExists(*FilePath) && Files.Copy(*BackupPath, *FilePath, true, true) != COPY_OK)
	{
		return ESaveContainerResult::WriteFailed;
	}

	bPreserveTempFile = true;
	if (!Files.Move(*FilePath, *TempPath, true, true))
	{
		// Preserve .bak and .tmp for explicit recovery if replacement failed.
		return ESaveContainerResult::WriteFailed;
	}

	return ESaveContainerResult::Success;
}

ESaveContainerResult FGlobalSaveContainer::Load()
{
	FGlobalSaveContainer Candidate;
	const ESaveContainerResult Result = Candidate.LoadData(GlobalPath());
	if (Result == ESaveContainerResult::Success)
	{
		ActiveSlot = Candidate.ActiveSlot;
		Flags = MoveTemp(Candidate.Flags);
	}

	return Result;
}

ESaveContainerResult FGlobalSaveContainer::Save()
{
	return SaveData(GlobalPath());
}

bool FGlobalSaveContainer::SetActiveSlot(uint8 Slot)
{
	if (FSlottedSaveContainer::IsValidSlot(Slot) && ActiveSlot != Slot)
	{
		ActiveSlot = Slot;
		return true;
	}
	return false;
}

uint8 FGlobalSaveContainer::GetActiveSlot() const
{
	return FSlottedSaveContainer::IsValidSlot(ActiveSlot)
		? ActiveSlot : FSlottedSaveContainer::DefaultSlot;
}

void FGlobalSaveContainer::AddFlag(const FGameplayTag& Flag)
{
	if (Flag.IsValid())
	{
		Flags.Add(Flag);
	}
}

void FGlobalSaveContainer::RemoveFlag(const FGameplayTag& Flag)
{
	Flags.Remove(Flag);
}

bool FGlobalSaveContainer::HasFlag(const FGameplayTag& Flag) const
{
	return Flag.IsValid() && Flags.Contains(Flag);
}

void FGlobalSaveContainer::SerializePayload(FArchive& Ar)
{
	Ar << ActiveSlot << Flags;
}

void FSlottedSaveContainer::FHeader::Reset()
{
	PlaySeconds = 0.0f;
	SaveTime = FDateTime(0);
	JsonData = MakeShared<FJsonObject>();
}

void FSlottedSaveContainer::FHeader::Serialize(FArchive& Ar)
{
	FString JsonString = TEXT("{}");
	if (Ar.IsSaving())
	{
		if (!JsonData.IsValid())
		{
			Ar.SetError();
			return;
		}

		const auto JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString, 0);
		if (!FJsonSerializer::Serialize(JsonData, JsonWriter, true))
		{
			Ar.SetError();
			return;
		}
	}

	Ar << PlaySeconds << SaveTime << JsonString;
	if (Ar.IsError())
	{
		return;
	}

	if (Ar.IsLoading())
	{
		const auto JsonReader = TJsonReaderFactory<>::Create(JsonString);

		TSharedPtr<FJsonObject> ParsedJson;
		if (!FJsonSerializer::Deserialize(JsonReader, ParsedJson) || !ParsedJson.IsValid())
		{
			Ar.SetError();
			return;
		}

		JsonData = MoveTemp(ParsedJson);
	}
}

ESaveContainerResult FSlottedSaveContainer::Load(uint8 Slot)
{
	if (!IsValidSlot(Slot))
	{
		TORO_REPORT_ERROR(TEXT("Slot %d is not between 1 and %d (both inclusive)"), Slot, MaxSaveSlots);
		return ESaveContainerResult::InvalidSlot;
	}

	FSlottedSaveContainer Candidate;
	const ESaveContainerResult Result = Candidate.LoadData(SlotPath(Slot));
	if (Result == ESaveContainerResult::Success
		|| Result == ESaveContainerResult::MissingFile) // Implies new game
	{
		SaveHeader = MoveTemp(Candidate.SaveHeader);
		TimeTracker = FDateTime::UtcNow();
		Flags = MoveTemp(Candidate.Flags);
		ActorRecords = MoveTemp(Candidate.ActorRecords);
	}

	return Result;
}

ESaveContainerResult FSlottedSaveContainer::Save(uint8 Slot, TFunctionRef<void(FJsonObject&)> ProcessJson)
{
	if (!IsValidSlot(Slot))
	{
		TORO_REPORT_ERROR(TEXT("Slot %d is not between 1 and %d (both inclusive)"), Slot, MaxSaveSlots);
		return ESaveContainerResult::InvalidSlot;
	}

	if (!SaveHeader.JsonData.IsValid())
	{
		return ESaveContainerResult::InvalidHeader;
	}

	FHeader PreviousHeader = SaveHeader;
	SaveHeader.JsonData = MakeShared<FJsonObject>();
	FJsonObject::Duplicate(PreviousHeader.JsonData, SaveHeader.JsonData);

	SaveHeader.SaveTime = FDateTime::UtcNow();
	FDateTime OldTimeTracker = SaveHeader.SaveTime;
	if (TimeTracker.IsSet())
	{
		OldTimeTracker = TimeTracker.GetValue();
		SaveHeader.PlaySeconds += (SaveHeader.SaveTime - OldTimeTracker).GetTotalSeconds();
	}

	ProcessJson(*SaveHeader.JsonData);
	const ESaveContainerResult Result = SaveData(SlotPath(Slot));
	if (Result == ESaveContainerResult::Success)
	{
		TimeTracker = SaveHeader.SaveTime;
	}
	else
	{
		SaveHeader = MoveTemp(PreviousHeader);
		TimeTracker = OldTimeTracker;
	}

	return Result;
}

ESaveContainerResult FSlottedSaveContainer::LoadSlotHeader(uint8 Slot, FHeader& OutHeader)
{
	if (!IsValidSlot(Slot))
	{
		TORO_REPORT_ERROR(TEXT("Slot %d is not between 1 and %d (both inclusive)"), Slot, MaxSaveSlots);
		return ESaveContainerResult::InvalidSlot;
	}

	const FString FilePath = SlotPath(Slot);
	IFileManager& Files = IFileManager::Get();
	if (!Files.FileExists(*FilePath))
	{
		return ESaveContainerResult::MissingFile;
	}

	const TUniquePtr<FArchive> Reader(Files.CreateFileReader(*FilePath));
	if (!Reader.IsValid() || Reader->IsError())
	{
		return ESaveContainerResult::ReadFailed;
	}

	uint8 SavedSchema;
	*Reader << SavedSchema;
	if (Reader->IsError() || SavedSchema != CurrentSchema)
	{
		return ESaveContainerResult::InvalidSchema;
	}

	FHeader Candidate;
	Candidate.Serialize(*Reader);
	if (Reader->IsError())
	{
		return ESaveContainerResult::InvalidHeader;
	}
	if (!Reader->Close())
	{
		return ESaveContainerResult::ReadFailed;
	}

	OutHeader = MoveTemp(Candidate);
	return ESaveContainerResult::Success;
}

ESaveContainerResult FSlottedSaveContainer::Delete(uint8 Slot)
{
	if (Slot == 0 || Slot > MaxSaveSlots)
	{
		TORO_REPORT_ERROR(TEXT("Slot %d is not between 1 and %d (both inclusive)"), Slot, MaxSaveSlots);
		return ESaveContainerResult::InvalidSlot;
	}

	const FString FilePath = SlotPath(Slot);
	IFileManager& Files = IFileManager::Get();
	const bool bExisted = Files.FileExists(*FilePath);
	if (!Files.Delete(*(FilePath + TEXT(".tmp")), false, true)
		|| !Files.Delete(*(FilePath + TEXT(".bak")), false, true)
		|| !Files.Delete(*FilePath, false, true))
	{
		return ESaveContainerResult::DeleteFailed;
	}

	return bExisted ? ESaveContainerResult::Success : ESaveContainerResult::MissingFile;
}

void FSlottedSaveContainer::AddFlag(const FGameplayTag& Flag)
{
	if (Flag.IsValid())
	{
		Flags.Add(Flag);
	}
}

void FSlottedSaveContainer::RemoveFlag(const FGameplayTag& Flag)
{
	Flags.Remove(Flag);
}

bool FSlottedSaveContainer::HasFlag(const FGameplayTag& Flag) const
{
	return Flag.IsValid() && Flags.Contains(Flag);
}

FObjectSaveData* FSlottedSaveContainer::GetObjectRecord(const FTaggedIdentity& Identity)
{
	if (!Identity.IsValid())
	{
		return nullptr;
	}

	return &ObjectRecords.FindOrAdd(Identity);
}

const FObjectSaveData* FSlottedSaveContainer::FindObjectRecord(const FTaggedIdentity& Identity) const
{
	return Identity.IsValid() ? ObjectRecords.Find(Identity) : nullptr;
}

FActorSaveRecord* FSlottedSaveContainer::GetActorRecord(const FTaggedIdentity& Identity)
{
	if (!Identity.IsValid())
	{
		return nullptr;
	}

	return &ActorRecords.FindOrAdd(Identity);
}

const FActorSaveRecord* FSlottedSaveContainer::FindActorRecord(const FTaggedIdentity& Identity) const
{
	return Identity.IsValid() ? ActorRecords.Find(Identity) : nullptr;
}

void FSlottedSaveContainer::SerializeHeader(FArchive& Ar)
{
	SaveHeader.Serialize(Ar);
}

void FSlottedSaveContainer::SerializePayload(FArchive& Ar)
{
	Ar << Flags << ObjectRecords << ActorRecords;
}
