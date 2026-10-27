// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/StructOpsTypeTraits.h"
#include "Serialization/StructuredArchive.h"
#include "ToroImageData.generated.h"

/**
 * Stores an image's dimensions and contiguous row-major FColor pixels.
 * A valid image has positive dimensions and exactly Size.X * Size.Y pixels.
 * Construction and serialization do not validate this relationship; use IsValid() before consuming the data.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FToroImageData
{
	GENERATED_BODY()

	/** Image width (X) and height (Y), in pixels. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImageData)
	FIntPoint Size;

	/** Contiguous row-major pixels; a valid image contains one entry per pixel in Size. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImageData)
	TArray<FColor> Pixels;

	FToroImageData()
		: Size(FIntPoint::ZeroValue)
		, Pixels({})
	{}

	/**
	 * Copies image dimensions and pixels and validates them with an ensure.
	 * @param Size Image width and height in pixels.
	 * @param InPixels Row-major pixels to copy; validity requires Size.X * Size.Y entries and positive dimensions.
	 */
	FToroImageData(const FIntPoint& Size, const TArray<FColor>& InPixels)
		: Size(Size)
		, Pixels(InPixels)
	{
		ensureMsgf(IsValid(),
			TEXT("Constructing an image with %s dimensions and %d pixels. The data is invalid."),
			*Size.ToString(), InPixels.Num()
		);
	}

	/**
	 * Copies pixels and sets image dimensions and validates them with an ensure.
	 * @param SizeX Image width in pixels.
	 * @param SizeY Image height in pixels.
	 * @param InPixels Row-major pixels to copy; validity requires SizeX * SizeY entries and positive dimensions.
	 */
	FToroImageData(int32 SizeX, int32 SizeY, const TArray<FColor>& InPixels)
		: Size(SizeX, SizeY)
		, Pixels(InPixels)
	{
		ensureMsgf(IsValid(),
			TEXT("Constructing an image with %s dimensions and %d pixels. The data is invalid."),
			*Size.ToString(), InPixels.Num()
		);
	}

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FToroImageData& ImageData)
	{
		Ar << ImageData.Size;
		Ar << ImageData.Pixels;
		return Ar;
	}

	FORCEINLINE friend void operator<<(FStructuredArchive::FSlot Slot, FToroImageData& ImageData)
	{
		FStructuredArchive::FRecord Record = Slot.EnterRecord();
		Record << SA_VALUE(TEXT("Size"), ImageData.Size) << SA_VALUE(TEXT("Pixels"), ImageData.Pixels);
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

	/** Returns whether both dimensions are positive and the pixel count matches their product. */
	[[nodiscard]] FORCEINLINE bool IsValid() const
	{
		return Size.X > 0 && Size.Y > 0 
			&& static_cast<int64>(Pixels.Num()) == static_cast<int64>(Size.X) * static_cast<int64>(Size.Y);
	}

	/**
	 * Encodes a valid image as PNG data.
	 * @param OutData Receives the encoded bytes; cleared before validation, including when the image is invalid.
	 * @return True if encoding produces nonempty output; false if the image is invalid or encoding produces no output.
	 */
	[[nodiscard]] bool CompressPNG(TArray64<uint8>& OutData) const;

	/** Resets dimensions to zero and removes all pixels, leaving an invalid empty image. */
	void Clear();
};

template<>
struct TStructOpsTypeTraits<FToroImageData> : TStructOpsTypeTraitsBase2<FToroImageData>
{
	enum
	{
		WithSerializer = true,
		WithStructuredSerializer = true
	};
};
