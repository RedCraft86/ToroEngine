// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Serialization/Archive.h"
#include "Serialization/StructuredArchive.h"
#include "UObject/StructOpsTypeTraits.h"
#include "WrappedTypes.generated.h"

/**
 * Boolean wrapped in a struct to allow usage with FInstancedStruct.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWrappedBool final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wrapped)
	bool Value;

	FWrappedBool()
		: Value(false)
	{}

	FWrappedBool(bool InValue)
		: Value(InValue)
	{}

	FORCEINLINE operator bool&() { return Value; }
	FORCEINLINE operator bool() const { return Value; }

	FORCEINLINE void operator=(bool InValue) { Value = InValue; }

	[[nodiscard]] FORCEINLINE bool operator==(const FWrappedBool& Other) const { return Value == Other.Value; }
	[[nodiscard]] FORCEINLINE bool operator!=(const FWrappedBool& Other) const { return Value != Other.Value; }

	[[nodiscard]] FORCEINLINE friend uint32 GetTypeHash(const FWrappedBool& Wrapper)
	{
		return GetTypeHash(Wrapper.Value);
	}

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FWrappedBool& Wrapper)
	{
		return Ar << Wrapper.Value;
	}

	FORCEINLINE friend void operator<<(FStructuredArchive::FSlot Slot, FWrappedBool& Wrapper)
	{
		Slot << Wrapper.Value;
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

	[[nodiscard]] FORCEINLINE FString ToString() const
	{
		return LexToString(Value);
	}
};

template<>
struct TStructOpsTypeTraits<FWrappedBool> : TStructOpsTypeTraitsBase2<FWrappedBool>
{
	enum
	{
		WithZeroConstructor = true,
		WithIdenticalViaEquality = true,
		WithSerializer = true,
		WithStructuredSerializer = true
	};
};

/**
 * Float wrapped in a struct to allow usage with FInstancedStruct.
 * @note Internally uses a Double for better compatibility with UE5 standards.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWrappedFloat final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wrapped)
	double Value;

	FWrappedFloat()
		: Value(0.0)
	{}

	FWrappedFloat(double InValue)
		: Value(InValue)
	{}

	/** @note This constructor may cause precision loss. */
	FWrappedFloat(float InValue)
		: Value(InValue)
	{}

	FORCEINLINE operator double&() { return Value; }
	FORCEINLINE operator double() const { return Value; }
	FORCEINLINE operator bool() const { return !FMath::IsNearlyZero(Value); }

	/** @note This assignment may cause precision loss. */
	FORCEINLINE void operator=(float InValue) { Value = InValue; }

	FORCEINLINE void operator=(double InValue) { Value = InValue; }

	[[nodiscard]] FORCEINLINE bool operator==(const FWrappedFloat& Other) const { return Value == Other.Value; }
	[[nodiscard]] FORCEINLINE bool operator!=(const FWrappedFloat& Other) const { return Value != Other.Value; }

	[[nodiscard]] FORCEINLINE friend uint32 GetTypeHash(const FWrappedFloat& Wrapper)
	{
		// Signed zeros compare equal, so they must also hash equally.
		return GetTypeHash(Wrapper.Value == 0.0 ? 0.0 : Wrapper.Value);
	}

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FWrappedFloat& Wrapper)
	{
		return Ar << Wrapper.Value;
	}

	FORCEINLINE friend void operator<<(FStructuredArchive::FSlot Slot, FWrappedFloat& Wrapper)
	{
		Slot << Wrapper.Value;
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

	[[nodiscard]] FORCEINLINE FString ToString() const
	{
		return FString::SanitizeFloat(Value);
	}
};

template<>
struct TStructOpsTypeTraits<FWrappedFloat> : TStructOpsTypeTraitsBase2<FWrappedFloat>
{
	enum
	{
		WithZeroConstructor = true,
		WithIdenticalViaEquality = true,
		WithSerializer = true,
		WithStructuredSerializer = true
	};
};

/**
 * Byte (uint8) wrapped in a struct to allow usage with FInstancedStruct.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWrappedByte final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wrapped)
	uint8 Value;

	FWrappedByte()
		: Value(0)
	{}

	FWrappedByte(uint8 InValue)
		: Value(InValue)
	{}

	FORCEINLINE operator uint8&() { return Value; }
	FORCEINLINE operator uint8() const { return Value; }
	FORCEINLINE operator bool() const { return Value != 0; }

	FORCEINLINE void operator=(uint8 InValue) { Value = InValue; }

	[[nodiscard]] FORCEINLINE bool operator==(const FWrappedByte& Other) const { return Value == Other.Value; }
	[[nodiscard]] FORCEINLINE bool operator!=(const FWrappedByte& Other) const { return Value != Other.Value; }

	[[nodiscard]] FORCEINLINE friend uint32 GetTypeHash(const FWrappedByte& Wrapper)
	{
		return GetTypeHash(Wrapper.Value);
	}

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FWrappedByte& Wrapper)
	{
		return Ar << Wrapper.Value;
	}

	FORCEINLINE friend void operator<<(FStructuredArchive::FSlot Slot, FWrappedByte& Wrapper)
	{
		Slot << Wrapper.Value;
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

	[[nodiscard]] FORCEINLINE FString ToString() const
	{
		return LexToString(Value);
	}
};

template<>
struct TStructOpsTypeTraits<FWrappedByte> : TStructOpsTypeTraitsBase2<FWrappedByte>
{
	enum
	{
		WithZeroConstructor = true,
		WithIdenticalViaEquality = true,
		WithSerializer = true,
		WithStructuredSerializer = true
	};
};

/**
 * 32-bit Integer wrapped in a struct to allow usage with FInstancedStruct.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWrappedInt32 final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wrapped)
	int32 Value;

	FWrappedInt32()
		: Value(0)
	{}

	FWrappedInt32(int32 InValue)
		: Value(InValue)
	{}

	FORCEINLINE operator int32&() { return Value; }
	FORCEINLINE operator int32() const { return Value; }
	FORCEINLINE operator bool() const { return Value != 0; }

	FORCEINLINE void operator=(int32 InValue) { Value = InValue; }

	[[nodiscard]] FORCEINLINE bool operator==(const FWrappedInt32& Other) const { return Value == Other.Value; }
	[[nodiscard]] FORCEINLINE bool operator!=(const FWrappedInt32& Other) const { return Value != Other.Value; }

	[[nodiscard]] FORCEINLINE friend uint32 GetTypeHash(const FWrappedInt32& Wrapper)
	{
		return GetTypeHash(Wrapper.Value);
	}

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FWrappedInt32& Wrapper)
	{
		return Ar << Wrapper.Value;
	}

	FORCEINLINE friend void operator<<(FStructuredArchive::FSlot Slot, FWrappedInt32& Wrapper)
	{
		Slot << Wrapper.Value;
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

	[[nodiscard]] FORCEINLINE FString ToString() const
	{
		return LexToString(Value);
	}
};

template<>
struct TStructOpsTypeTraits<FWrappedInt32> : TStructOpsTypeTraitsBase2<FWrappedInt32>
{
	enum
	{
		WithZeroConstructor = true,
		WithIdenticalViaEquality = true,
		WithSerializer = true,
		WithStructuredSerializer = true
	};
};

/**
 * 64-bit Integer wrapped in a struct to allow usage with FInstancedStruct.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWrappedInt64 final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wrapped)
	int64 Value;

	FWrappedInt64()
		: Value(0)
	{}

	FWrappedInt64(int64 InValue)
		: Value(InValue)
	{}

	FORCEINLINE operator int64&() { return Value; }
	FORCEINLINE operator int64() const { return Value; }
	FORCEINLINE operator bool() const { return Value != 0; }

	FORCEINLINE void operator=(int64 InValue) { Value = InValue; }

	[[nodiscard]] FORCEINLINE bool operator==(const FWrappedInt64& Other) const { return Value == Other.Value; }
	[[nodiscard]] FORCEINLINE bool operator!=(const FWrappedInt64& Other) const { return Value != Other.Value; }

	[[nodiscard]] FORCEINLINE friend uint32 GetTypeHash(const FWrappedInt64& Wrapper)
	{
		return GetTypeHash(Wrapper.Value);
	}

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FWrappedInt64& Wrapper)
	{
		return Ar << Wrapper.Value;
	}

	FORCEINLINE friend void operator<<(FStructuredArchive::FSlot Slot, FWrappedInt64& Wrapper)
	{
		Slot << Wrapper.Value;
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

	[[nodiscard]] FORCEINLINE FString ToString() const
	{
		return LexToString(Value);
	}
};

template<>
struct TStructOpsTypeTraits<FWrappedInt64> : TStructOpsTypeTraitsBase2<FWrappedInt64>
{
	enum
	{
		WithZeroConstructor = true,
		WithIdenticalViaEquality = true,
		WithSerializer = true,
		WithStructuredSerializer = true
	};
};

/**
 * Name wrapped in a struct to allow usage with FInstancedStruct.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWrappedName final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wrapped)
	FName Value;

	FWrappedName()
		: Value(NAME_None)
	{}

	FWrappedName(const FName& InValue)
		: Value(InValue)
	{}

	FORCEINLINE operator FName&() { return Value; }
	FORCEINLINE operator const FName&() const { return Value; }
	FORCEINLINE operator bool() const { return !Value.IsNone(); }

	FORCEINLINE void operator=(const FName& InValue) { Value = InValue; }
	FORCEINLINE void operator=(const FString& InValue) { Value = *InValue; }

	[[nodiscard]] FORCEINLINE bool operator==(const FWrappedName& Other) const { return Value == Other.Value; }
	[[nodiscard]] FORCEINLINE bool operator!=(const FWrappedName& Other) const { return Value != Other.Value; }

	[[nodiscard]] FORCEINLINE friend uint32 GetTypeHash(const FWrappedName& Wrapper)
	{
		return GetTypeHash(Wrapper.Value);
	}

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FWrappedName& Wrapper)
	{
		return Ar << Wrapper.Value;
	}

	FORCEINLINE friend void operator<<(FStructuredArchive::FSlot Slot, FWrappedName& Wrapper)
	{
		Slot << Wrapper.Value;
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

	[[nodiscard]] FORCEINLINE FString ToString() const
	{
		return Value.ToString();
	}
};

template<>
struct TStructOpsTypeTraits<FWrappedName> : TStructOpsTypeTraitsBase2<FWrappedName>
{
	enum
	{
		WithIdenticalViaEquality = true,
		WithSerializer = true,
		WithStructuredSerializer = true
	};
};

/**
 * String wrapped in a struct to allow usage with FInstancedStruct.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWrappedString final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wrapped)
	FString Value;

	FWrappedString()
		: Value(FString())
	{}

	FWrappedString(const FString& InValue)
		: Value(InValue)
	{}

	FORCEINLINE operator FString&() { return Value; }
	FORCEINLINE operator const FString&() const { return Value; }
	FORCEINLINE operator bool() const { return !Value.TrimStartAndEnd().IsEmpty(); }

	FORCEINLINE void operator=(const FString& InValue) { Value = InValue; }
	FORCEINLINE void operator=(const FName& InValue) { Value = InValue.ToString(); }

	[[nodiscard]] FORCEINLINE bool operator==(const FWrappedString& Other) const { return Value == Other.Value; }
	[[nodiscard]] FORCEINLINE bool operator!=(const FWrappedString& Other) const { return Value != Other.Value; }

	[[nodiscard]] FORCEINLINE friend uint32 GetTypeHash(const FWrappedString& Wrapper)
	{
		return GetTypeHash(Wrapper.Value);
	}

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FWrappedString& Wrapper)
	{
		return Ar << Wrapper.Value;
	}

	FORCEINLINE friend void operator<<(FStructuredArchive::FSlot Slot, FWrappedString& Wrapper)
	{
		Slot << Wrapper.Value;
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
};

template<>
struct TStructOpsTypeTraits<FWrappedString> : TStructOpsTypeTraitsBase2<FWrappedString>
{
	enum
	{
		WithIdenticalViaEquality = true,
		WithSerializer = true,
		WithStructuredSerializer = true
	};
};