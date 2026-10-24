// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Misc/Guid.h"
#include "GameplayTagContainer.h"
#include "Serialization/Archive.h"
#include "Serialization/StructuredArchive.h"
#include "UObject/StructOpsTypeTraits.h"
#include "ToroIdentity.generated.h"

/**
 * Identifies an item by an exact gameplay-tag group and a GUID.
 *
 * Both fields must be valid for the identity to be valid. Equality and hashing use
 * both fields, so do not modify them while this identity is stored as a hashed key.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FToroIdentity final
{
	GENERATED_BODY()

	/** Group component of the identity; comparisons use exact tag equality. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Identity)
	FGameplayTag Group;

	/** GUID component of the identity; an all-zero value is invalid. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Identity)
	FGuid Guid;

	/** Empty identity with no group and an invalid, all-zero GUID. */
	static const FToroIdentity Empty;

	/** Constructs an empty identity with no group and an invalid GUID. */
	FToroIdentity()
		: Group(FGameplayTag::EmptyTag)
		, Guid()
	{}

	/**
	 * Constructs an identity with the supplied group and a newly generated GUID.
	 * @param InGroup Group to assign; an empty tag leaves the identity invalid.
	 */
	explicit FToroIdentity(const FGameplayTag& InGroup)
		: Group(InGroup)
		, Guid(FGuid::NewGuid())
	{}

	/**
	 * Constructs an identity from the supplied components without generating a GUID.
	 * @param InGroup Group to assign.
	 * @param InGuid GUID to assign. Both components must be valid for IsValid() to succeed.
	 */
	FToroIdentity(const FGameplayTag& InGroup, const FGuid& InGuid)
		: Group(InGroup)
		, Guid(InGuid)
	{}

	[[nodiscard]] FORCEINLINE operator bool() const
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

	FORCEINLINE bool Serialize(FArchive& Ar)
	{
		Ar << *this;
		return true;
	}

	FORCEINLINE bool Serialize(FStructuredArchive::FSlot Slot)
	{
		Slot << *this;
		return true;
	}

	/** Returns whether the group is valid and the GUID is nonzero. */
	[[nodiscard]] FORCEINLINE bool IsValid() const
	{
		return Group.IsValid() && Guid.IsValid();
	}

	/** Formats the identity as Group[Guid], using the default GUID string format. */
	[[nodiscard]] FString ToString() const;

	/** Clears the group and invalidates the GUID, restoring the empty identity. */
	void Invalidate();

	/**
	 * Generates a new GUID while preserving the group.
	 * @param bOnlyIfInvalid If true, preserves an existing valid GUID, regardless of
	 *        whether the group is valid. If false, always replaces the GUID.
	 */
	void RegenerateGuid(bool bOnlyIfInvalid = false);

#if WITH_EDITOR
	bool CanEditChange(const FEditPropertyChain& PropertyChain) const;
#endif
};

template<>
struct TStructOpsTypeTraits<FToroIdentity> : TStructOpsTypeTraitsBase2<FToroIdentity>
{
	enum
	{
		WithIdenticalViaEquality = true,
		WithSerializer = true,
		WithStructuredSerializer = true,
		WithCanEditChange = true
	};
};