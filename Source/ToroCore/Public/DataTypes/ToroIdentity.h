// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Misc/Guid.h"
#include "GameplayTagContainer.h"
#include "Serialization/StructuredArchive.h"
#include "ToroIdentity.generated.h"

/**
 * An identity consisting of a gameplay tag group and a GUID.
 * Both must be valid for the identity to be valid.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FToroIdentity
{
	GENERATED_BODY()

	/** Gameplay tag identifying the group to which this identity belongs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Identity)
	FGameplayTag Group;

	/** GUID identifying this entry within its group. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Identity)
	FGuid Guid;

	/** Empty identity with an empty group tag and an invalid GUID. */
	static const FToroIdentity Empty;

	/** Constructs an empty, invalid identity. */
	FToroIdentity()
		: Group(FGameplayTag::EmptyTag), Guid(FGuid())
	{}

	/** Constructs an identity with the supplied group and a newly generated GUID. */
	explicit FToroIdentity(const FGameplayTag& InGroup)
		: Group(InGroup), Guid(FGuid::NewGuid())
	{}

	/** Constructs an identity using the supplied group and GUID without validating them. */
	FToroIdentity(const FGameplayTag& InGroup, const FGuid& InGuid)
		: Group(InGroup), Guid(InGuid)
	{}

	[[nodiscard]] FORCEINLINE explicit operator bool() const
	{
		return IsValid();
	}

	[[nodiscard]] FORCEINLINE bool operator==(const FToroIdentity& Other) const
	{
		return Group == Other.Group && Guid == Other.Guid;
	}

	[[nodiscard]] FORCEINLINE bool operator!=(const FToroIdentity& Other) const
	{
		return Group != Other.Group || Guid != Other.Guid;
	}

	[[nodiscard]] FORCEINLINE friend uint32 GetTypeHash(const FToroIdentity& Identity)
	{
		return HashCombine(GetTypeHash(Identity.Group), GetTypeHash(Identity.Guid));
	}

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FToroIdentity& Identity)
	{
		return Ar << Identity.Group << Identity.Guid;
	}

	FORCEINLINE friend void operator<<(FStructuredArchive::FSlot Slot, FToroIdentity& Identity)
	{
		FStructuredArchive::FRecord Record = Slot.EnterRecord();
		Record << SA_VALUE(TEXT("Group"), Identity.Group) << SA_VALUE(TEXT("Guid"), Identity.Guid);
	}

	/** Returns true only when both the group tag and GUID are valid. */
	[[nodiscard]] FORCEINLINE bool IsValid() const
	{
		return Group.IsValid() && Guid.IsValid();
	}

	/** Returns the identity formatted as Group[Guid]. */
	[[nodiscard]] FString ToString() const;

	/**
	 * Generates a new GUID while preserving the group tag.
	 * @param bOnlyIfInvalid If true, replaces the GUID only when the GUID itself is invalid,
	 * regardless of group validity.
	 */
	void RegenerateGuid(const bool bOnlyIfInvalid = false);

	/** Clears the group tag and invalidates the GUID, restoring the empty identity state. */
	void Invalidate();
};
