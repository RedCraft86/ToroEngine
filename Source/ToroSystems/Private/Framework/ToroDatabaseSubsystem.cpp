// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroDatabaseSubsystem.h"
#include "Utilities/ToroUtilities.h"
#include "DataAssets/ToroDatabase.h"
#include "Engine/GameInstance.h"
#include "ToroDevSettings.h"
#include "Engine/World.h"

UToroDatabaseSubsystem* UToroDatabaseSubsystem::Get(const UObject* Context)
{
	const UWorld* World = ToroEngine::GetWorld(Context);
	const UGameInstance* GameInstance = IsValid(World) ? World->GetGameInstance() : nullptr;
	return IsValid(GameInstance) ? GameInstance->GetSubsystem<UToroDatabaseSubsystem>() : nullptr;
}

void UToroDatabaseSubsystem::AddDatabase(const UToroDatabase* Database)
{
	if (IsValid(Database))
	{
		Databases.Add(Database);
	}
}

const FInstancedStruct& UToroDatabaseSubsystem::FindData(FGameplayTag Key)
{
	if (!Key.IsValid())
	{
		return UToroDatabase::EmptyValue;
	}

	const FInstancedStruct* OverrideData = RuntimeOverrides.Find(Key);
	if (OverrideData && OverrideData->IsValid())
	{
		return *OverrideData;
	}

	for (const TSoftObjectPtr<const UToroDatabase>& SoftDatabase : Databases)
	{
		if (const UToroDatabase* Database = SoftDatabase.LoadSynchronous())
		{
			if (Key.MatchesTag(Database->GetRootTag()))
			{
				const FInstancedStruct& Value = Database->GetValue(Key);
				if (Value.IsValid())
				{
					return Value;
				}
			}
		}
	}

	return UToroDatabase::EmptyValue;
}

bool UToroDatabaseSubsystem::AddRuntimeOverride(FGameplayTag Key,
	const TInstancedStruct<FToroDatabaseEntry>& Data)
{
	if (!Key.IsValid() || !Data.IsValid())
	{
		return false;
	}

	const FInstancedStruct& Existing = FindData(Key);
	if (Existing.IsValid() && Existing.GetScriptStruct() != Data.GetScriptStruct())
	{
		TORO_REPORT_ERROR(
			TEXT("Override type mismatch: expected %s, got %s."),
			*Existing.GetScriptStruct()->GetName(),
			*Data.GetScriptStruct()->GetName()
		);
		return false;
	}

	RuntimeOverrides.Add(Key, static_cast<const FInstancedStruct&>(Data));
	return true;
}

void UToroDatabaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const UToroDevSettings* Settings = UToroDevSettings::Get())
	{
		for (const TSoftObjectPtr<UToroDatabase>& Database : Settings->Databases)
		{
			Databases.Add(Database);
		}
	}
}

void UToroDatabaseSubsystem::Deinitialize()
{
	RuntimeOverrides.Reset();
	Super::Deinitialize();
}
