// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "GameplayTagContainer.h"
#include "WrappedTypes.generated.h"

/**
 * Boolean wrapped in a struct to allow usage with FInstancedStruct.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FWrappedBool final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wrapped)
	bool Value;

	FWrappedBool()
		: Value(false)
	{}

	FWrappedBool(const bool InValue)
		: Value(InValue)
	{}

	FORCEINLINE operator bool&() { return Value; }
	FORCEINLINE operator bool() const { return Value; }

	FORCEINLINE void operator=(const bool InValue) { Value = InValue; }

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

	[[nodiscard]] FORCEINLINE FString ToString() const
	{
		return LexToString(Value);
	}
};

/**
 * Float wrapped in a struct to allow usage with FInstancedStruct.
 * @note Internally uses a Double for better compatibility with UE5 standards.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FWrappedFloat final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wrapped)
	double Value;

	FWrappedFloat()
		: Value(0.0)
	{}

	FWrappedFloat(const double InValue)
		: Value(InValue)
	{}

	FORCEINLINE operator double&() { return Value; }
	FORCEINLINE operator double() const { return Value; }

	FORCEINLINE void operator=(const float InValue) { Value = InValue; }

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

	[[nodiscard]] FORCEINLINE FString ToString() const
	{
		return FString::SanitizeFloat(Value);
	}
};

/**
 * Byte (uint8) wrapped in a struct to allow usage with FInstancedStruct.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FWrappedByte final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wrapped)
	uint8 Value;

	FWrappedByte()
		: Value(0)
	{}

	FWrappedByte(const uint8 InValue)
		: Value(InValue)
	{}

	FORCEINLINE operator uint8&() { return Value; }
	FORCEINLINE operator uint8() const { return Value; }

	FORCEINLINE void operator=(const uint8 InValue) { Value = InValue; }

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

	[[nodiscard]] FORCEINLINE FString ToString() const
	{
		return LexToString(Value);
	}
};

/**
 * 32-bit Integer wrapped in a struct to allow usage with FInstancedStruct.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FWrappedInt32 final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wrapped)
	int32 Value;

	FWrappedInt32()
		: Value(0)
	{}

	FWrappedInt32(const int32 InValue)
		: Value(InValue)
	{}

	FORCEINLINE operator int32&() { return Value; }
	FORCEINLINE operator int32() const { return Value; }

	FORCEINLINE void operator=(const int32 InValue) { Value = InValue; }

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

	[[nodiscard]] FORCEINLINE FString ToString() const
	{
		return LexToString(Value);
	}
};

/**
 * 64-bit Integer wrapped in a struct to allow usage with FInstancedStruct.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FWrappedInt64 final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wrapped)
	int64 Value;

	FWrappedInt64()
		: Value(0)
	{}

	FWrappedInt64(const int64 InValue)
		: Value(InValue)
	{}

	FORCEINLINE operator int64&() { return Value; }
	FORCEINLINE operator int64() const { return Value; }

	FORCEINLINE void operator=(const int64 InValue) { Value = InValue; }

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

	[[nodiscard]] FORCEINLINE FString ToString() const
	{
		return LexToString(Value);
	}
};

/**
 * String wrapped in a struct to allow usage with FInstancedStruct.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FWrappedString final
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

	FORCEINLINE bool operator==(const FWrappedString& Other) const { return Value == Other.Value; }
	FORCEINLINE bool operator!=(const FWrappedString& Other) const { return Value != Other.Value; }

	[[nodiscard]] FORCEINLINE friend uint32 GetTypeHash(const FWrappedString& Wrapper)
	{
		return GetTypeHash(Wrapper.Value);
	}
};

/**
 * String wrapped in a struct to allow usage with FInstancedStruct.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FWrappedString final
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
};

