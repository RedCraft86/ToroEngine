// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroDataAsset.h"
#include "Containers/Map.h"
#include "UObject/Class.h"
#include "UObject/ObjectPtr.h"
#include "GameplayTagContainer.h"
#include "Templates/Models.h"
#include "Templates/UnrealTypeTraits.h"
#include "Concepts/StaticStructProvider.h"
#include "StructUtils/InstancedStruct.h"
#include "ToroDatabase.generated.h"

/**
 * Polymorphic base for data stored in Toro database entries.
 * Native reflected subclasses provide payload fields and override IsValid() for editor validation.
 */
USTRUCT(BlueprintInternalUseOnly, meta = (Hidden))
struct TOROENGINE_API FToroDatabaseEntry
{
	GENERATED_BODY()

	virtual ~FToroDatabaseEntry() = default;

	/**
	 * Reports whether the payload satisfies its data-specific validation rules.
	 * The base implementation rejects every payload. Database lookups do not run this check.
	 */
	virtual bool IsValid() const
	{
		return false;
	}
};

/**
 * Gameplay-tag-keyed asset containing reflected struct payloads from a configured type hierarchy.
 * Configured native subclasses supply RootTag and RootStruct through the protected constructor.
 * Keys must be strict descendants of RootTag; values must use RootStruct or one of its descendants.
 * Remains concrete to support validation testing of unconfigured database instances.
 */
UCLASS(Abstract, NotBlueprintable, BlueprintType)
class TOROENGINE_API UToroDatabase : public UToroDataAsset
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class FToroDatabaseCustomization;
#endif

public:

	/** Shared immutable empty result used when a lookup cannot return a compatible stored payload. */
	static inline const FInstancedStruct EmptyValue = {};

	/**
	 * Exposes the stored entries without filtering or validating them.
	 * C++ callers receive a borrowed reference that must not outlive the asset or data mutation.
	 */
	UFUNCTION(BlueprintPure, Category = Database)
	[[nodiscard]] const TMap<FGameplayTag, FInstancedStruct>& GetEntries() const
	{
		return Entries;
	}

	/**
	 * Validates if a key is eligible for use in this database.
	 * Requires a valid RootTag and a valid strict descendant tag; the root itself is ineligible.
	 * <pre>
	 * e.g., if RootTag is 'Quest'
	 * 'None'              is invalid
	 * 'Character'         is invalid
	 * 'Quest'             is invalid
	 * 'Quest.WalkForward' is valid
	 * </pre>
	 * @param Key The tag to check.
	 * @return True if the key follows the database's tag hierarchy rules.
	 */
	UFUNCTION(BlueprintPure, Category = Database)
	bool IsValidKey(const FGameplayTag& Key) const;

	/**
	 * Checks whether an eligible key has an entry, regardless of payload validity.
	 * @param Key Gameplay tag to locate within the database hierarchy.
	 */
	UFUNCTION(BlueprintPure, Category = Database)
	bool DoesKeyExist(const FGameplayTag& Key) const;

	/**
	 * Retrieves a populated payload compatible with the current root tag and struct configuration.
	 * @param Key Eligible gameplay tag to look up.
	 * @return Stored payload, or EmptyValue for invalid roots, ineligible or missing keys, empty payloads, or incompatible stored struct types.
	 *         C++ callers must not retain the borrowed reference across data mutation or asset destruction.
	 * @note Payload-specific IsValid() is checked during editor validation, not retrieval.
	 */
	UFUNCTION(BlueprintPure, Category = Database)
	const FInstancedStruct& GetValue(const FGameplayTag& Key) const;

	/**
	 * Retrieves a payload as a compatible reflected entry type.
	 * @tparam StructType Reflected struct derived from FToroDatabaseEntry.
	 * @param Key Eligible gameplay tag to look up.
	 * @return Borrowed payload pointer, or null if retrieval fails or the stored type is incompatible.
	 *         Do not retain the pointer across data mutation or asset destruction.
	 * @note Payload-specific IsValid() is checked during editor validation, not retrieval.
	 */
	template <typename StructType>
	const StructType* Get(const FGameplayTag& Key) const
	{
		static_assert(TModels<CStaticStructProvider, StructType>::Value,
			"StructType must be a USTRUCT");

		static_assert(TIsDerivedFrom<StructType, FToroDatabaseEntry>::Value,
			"StructType must derive from FToroDatabaseEntry");

		const FInstancedStruct& FoundValue = GetValue(Key);
		return FoundValue.IsValid() ? FoundValue.GetPtr<StructType>() : nullptr;
	}

protected:

	/** Required root of the permitted key hierarchy; the root itself cannot be used as an entry key. */
	UPROPERTY(VisibleAnywhere, Category = Asset, meta = (DisplayPriority = 0))
	FGameplayTag RootTag;

	/** Required payload base type, which must derive from FToroDatabaseEntry. */
	UPROPERTY(VisibleAnywhere, Category = Asset, meta = (DisplayPriority = 0))
	TObjectPtr<const UScriptStruct> RootStruct;

	/**
	 * Stored tag-to-payload mapping; editor changes initialize empty payloads using a valid RootStruct.
	 * Invalid keys, incompatible types, and data-specific errors are reported by editor validation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Data, TextExportTransient, meta = (ForceInlineRow))
	TMap<FGameplayTag, FInstancedStruct> Entries;

	/**
	 * Creates an unconfigured database for validation testing.
	 * Lookups return no eligible entries until valid roots are supplied.
	 * Sub-classes should not call this super constructor.
	 */
	UToroDatabase()
		: RootTag(FGameplayTag::EmptyTag)
		, RootStruct(nullptr)
		, Entries({})
	{}

	/**
	 * Configures the tag and payload hierarchies for a native subclass.
	 * Editor builds report invalid constructor arguments; subsequent checks inspect current properties.
	 * @param TagType Valid root tag whose strict descendants are eligible keys.
	 * @param StructType Valid payload base struct derived from FToroDatabaseEntry.
	 */
	UToroDatabase(const FGameplayTag& TagType, const UScriptStruct* StructType)
		: RootTag(TagType)
		, RootStruct(StructType)
		, Entries({})
	{
#if WITH_EDITOR
		ensureMsgf(TagType.IsValid(), TEXT("RootTag for database %s is invalid."), *GetClass()->GetName());
		ensureMsgf(IsValid(StructType) && StructType->IsChildOf<FToroDatabaseEntry>(),
			TEXT("RootStruct for database %s is invalid. It must derive from FToroDatabaseEntry."), *GetClass()->GetName());
#endif
	}

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
