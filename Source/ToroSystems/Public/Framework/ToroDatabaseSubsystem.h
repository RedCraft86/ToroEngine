// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameplayTagContainer.h"
#include "Templates/Models.h"
#include "Templates/Function.h"
#include "Templates/UnrealTypeTraits.h"
#include "Utilities/ToroLogging.h"
#include "DataAssets/ToroDatabase.h"
#include "StructUtils/InstancedStruct.h"
#include "Concepts/StaticStructProvider.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ToroDatabaseSubsystem.generated.h"

/**
 * Resolves gameplay-tag-keyed database entries with game-instance-owned runtime overrides.
 * Registers configured databases at initialization and loads them synchronously on demand.
 * Overrides take priority; otherwise the first populated database result wins, with no defined
 * precedence between databases. Overrides survive map travel but are not saved by this subsystem.
 * Access and merge callbacks must occur on the game thread.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class UToroDatabaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/**
	 * Retrieves the database subsystem belonging to the context's game instance.
	 * @return The subsystem, or nullptr if the world, game instance, or subsystem is unavailable.
	 */
	TOROSYSTEMS_API static UToroDatabaseSubsystem* Get(const UObject* Context);

	/**
	 * Registers a database for subsequent lookups; invalid objects are ignored and duplicates have no effect.
	 * Stores a soft reference rather than keeping the database loaded. Registration lasts for this game instance.
	 * @param Database Database to include alongside the configured defaults.
	 */
	UFUNCTION(BlueprintCallable, Category = Database)
	TOROSYSTEMS_API void AddDatabase(const UToroDatabase* Database);

	/**
	 * Finds an override first, then searches registered databases, loading them synchronously as needed.
	 * @param Key Exact entry key; an invalid key returns an empty result.
	 * @return Borrowed stored payload, or a shared empty value if no populated entry is found.
	 * Do not retain the reference across registry mutation, database edits, asset collection, or teardown;
	 * copy the value when it must be retained. Payload-specific IsValid() is not evaluated here.
	 */
	UFUNCTION(BlueprintCallable, Category = Database)
	TOROSYSTEMS_API const FInstancedStruct& FindData(FGameplayTag Key);

	/**
	 * Finds an entry whose reflected struct type exactly matches the requested type.
	 * @tparam StructType Reflected struct derived from FToroDatabaseEntry; derived payload types are rejected.
	 * @param Key Exact entry key to resolve through overrides and registered databases.
	 * @return Borrowed payload pointer, or nullptr for missing data or a reported type mismatch.
	 * The pointer has the same lifetime constraints as the untyped FindData result.
	 */
	template <typename StructType>
	const StructType* FindData(const FGameplayTag& Key)
	{
		static_assert(TModels<CStaticStructProvider, StructType>::Value,
			"StructType must be a USTRUCT");

		static_assert(TIsDerivedFrom<StructType, FToroDatabaseEntry>::Value,
			"StructType must derive from FToroDatabaseEntry");

		const FInstancedStruct& FoundValue = FindData(Key);
		if (FoundValue.IsValid() && FoundValue.GetScriptStruct() != StructType::StaticStruct())
		{
			TORO_REPORT_ERROR(
				TEXT("Find type mismatch: expected %s, got %s."),
				*StructType::StaticStruct()->GetName(),
				*FoundValue.GetScriptStruct()->GetName()
			);
			return nullptr;
		}

		return FoundValue.GetPtr<StructType>();
	}

	/**
	 * Copies a payload into the runtime overrides without modifying its source database.
	 * @param Key Valid entry key; the key need not already exist in a database.
	 * @param Data Populated payload derived from FToroDatabaseEntry. If data already resolves for Key,
	 * its reflected struct type must match exactly. Payload-specific IsValid() is not evaluated.
	 * @return True if stored; false for an invalid key, empty payload, or reported type mismatch.
	 */
	UFUNCTION(BlueprintCallable, Category = Database)
	TOROSYSTEMS_API bool AddRuntimeOverride(FGameplayTag Key,
		const TInstancedStruct<FToroDatabaseEntry>& Data);

	/**
	 * Adds a typed override, optionally merging with the currently resolved override or database entry.
	 * @tparam StructType Reflected struct derived from FToroDatabaseEntry.
	 * @param Key Valid entry key to override.
	 * @param Data Incoming payload copied for storage; existing data must have the exact same struct type.
	 * @param Merge Optional synchronous callback that modifies a copy of existing data using Incoming.
	 * Without existing data, stores Data directly without invoking the callback. The callback is not retained
	 * and must not mutate this registry or invalidate its borrowed arguments while executing.
	 * @return True if the resulting override is stored; false for an invalid key or type mismatch.
	 */
	template <typename StructType>
	bool AddRuntimeOverride(const FGameplayTag& Key, const StructType& Data,
		TFunction<void(StructType& Result, const StructType& Incoming)> Merge = nullptr)
	{
		static_assert(TModels<CStaticStructProvider, StructType>::Value,
			"StructType must be a USTRUCT");

		static_assert(TIsDerivedFrom<StructType, FToroDatabaseEntry>::Value,
			"StructType must derive from FToroDatabaseEntry");

		if (!Key.IsValid())
		{
			return false;
		}

		auto Result = TInstancedStruct<FToroDatabaseEntry>::Make<StructType>(Data);
		if (Merge)
		{
			if (const StructType* Existing = FindData<StructType>(Key))
			{
				Result.template InitializeAs<StructType>(*Existing);
				Merge(Result.template GetMutable<StructType>(), Data);
			}
		}

		return AddRuntimeOverride(Key, Result);
	}

private:

	UPROPERTY(Transient)
	TSet<TSoftObjectPtr<const UToroDatabase>> Databases;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FInstancedStruct> RuntimeOverrides;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

};
