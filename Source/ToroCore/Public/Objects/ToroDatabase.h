// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToroDataAsset.h"
#include "GameplayTagContainer.h"
#include "DataTypes/WrappedTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "ToroDatabase.generated.h"

/**
 * Base for native reflected structs stored in a Toro database.
 * Native entry types override IsValid to define their data requirements; the base entry is invalid.
 */
USTRUCT(BlueprintInternalUseOnly, meta = (Hidden))
struct TOROCORE_API FToroDatabaseEntry
{
	GENERATED_BODY()

	virtual ~FToroDatabaseEntry() = default;

	/** Reports whether the entry's data is valid; the base implementation returns false. */
	virtual bool IsValid() const
	{
		return false;
	}
};

/**
 * Native data asset database mapping exact gameplay tags to reflected entry structs.
 * Native subclasses configure a root tag and an entry struct type through the parameterized constructor.
 * Eligible keys are strict descendants of RootTag; stored types must derive from RootStruct.
 * Lookups do not run key eligibility or entry data validation. Use editor validation to check database contents.
 */
UCLASS(Abstract, NotBlueprintable, BlueprintType)
class TOROCORE_API UToroDatabase : public UToroDataAsset
{
	GENERATED_BODY()

#if WITH_EDITOR
	// TODO: friend class FToroDatabaseDetails
#endif

public:

	/** Reports whether a nonempty exact key is stored, regardless of key eligibility or value validity. */
	UFUNCTION(BlueprintPure, Category = Database)
	bool DoesKeyExist(const FGameplayTag& Key) const;

	/**
	 * Copies the struct stored under the exact key, or returns an empty struct for an empty key or missing value.
	 * Does not check the stored type against RootStruct or call the entry's IsValid method.
	 */
	UFUNCTION(BlueprintPure, Category = Database)
	FInstancedStruct GetValue(const FGameplayTag& Key) const;

	/**
	 * Checks whether a key is a strict descendant of RootTag, without checking whether it is stored.
	 * For a root of Quest, Quest.WalkForward is eligible; Quest, Character, and an empty tag are not.
	 */
	virtual bool IsValidKey(const FGameplayTag& Key) const;

	/** Provides read-only access to the stored entries for this asset's lifetime. */
	const TMap<FWrappedGameplayTag, FInstancedStruct>& GetEntries() const;

	/**
	 * Borrows the value under the exact key as a native reflected entry type.
	 * StructType must be RootStruct or a descendant. Missing root configuration or an unsupported requested
	 * type triggers an ensure and returns nullptr; missing or incompatible stored values also return nullptr.
	 * The pointer is invalidated by relevant storage changes or asset destruction. Entry data is not validated.
	 */
	template <typename StructType>
	const StructType* GetValue(const FGameplayTag& Key) const
	{
		static_assert(TIsDerivedFrom<StructType, FToroDatabaseEntry>::Value,
			"StructType must derive from FToroDatabaseEntry and be a USTRUCT");

		const UScriptStruct* OutType = TBaseStructure<StructType>::Get();
		if (!ensureAlwaysMsgf(OutType, TEXT("T is not a USTRUCT type"))
			|| !ensureAlwaysMsgf(RootStruct, TEXT("RootStruct is null")))
		{
			return nullptr;
		}

		if (!ensureAlwaysMsgf(OutType == RootStruct || OutType->IsChildOf(RootStruct),
			TEXT("T (%s) is not RootStruct (%s) or a child of it"), *OutType->GetName(), *RootStruct->GetName()))
		{
			return nullptr;
		}

		if (!Key.IsValid())
		{
			return nullptr;
		}

		const FInstancedStruct* FoundValue = Entries.Find(Key);
		return FoundValue ? FoundValue->GetPtr<StructType>() : nullptr;
	}

protected:

	/**
	 * Creates an unconfigured database; editor validation reports its missing root configuration. 
	 * Subclass constructors should NOT call this overload and instead use the one with parameters.
	 */
	UToroDatabase();

	/**
	 * Configures the root tag and entry type for a native subclass.
	 * Subclass constructors should call this overload with a valid tag and a struct derived from FToroDatabaseEntry.
	 */
	UToroDatabase(const FGameplayTag& TagType, const UScriptStruct* StructType);

	/** Parent tag of eligible keys; the root itself is not an eligible key. */
	UPROPERTY(VisibleAnywhere, Category = Asset, meta = (DisplayPriority = 0))
	FGameplayTag RootTag;

	/** Required base type of stored values, configured by the native subclass constructor. */
	UPROPERTY(VisibleAnywhere, Category = Asset, meta = (DisplayPriority = 0))
	TObjectPtr<const UScriptStruct> RootStruct = nullptr;

	/**
	 * Stored entries keyed by exact gameplay tags. Empty keys remain unset until explicitly chosen.
	 * Editor changes initialize empty values to RootStruct when it is a supported entry type.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Data, TextExportTransient, meta = (ForceInlineRow))
	TMap<FWrappedGameplayTag, FInstancedStruct> Entries;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
