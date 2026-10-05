// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/StructuredArchive.h"
#include "ToroImageData.generated.h"

/**
 * Blueprint-accessible image dimensions and uncompressed 8-bit color pixels.
 * Pixels are stored in row-major order, with pixel (X, Y) at Y * Size.X + X.
 * Constructors and serialization do not validate dimensions or pixel count;
 * use IsValid() before consuming data that may be incomplete or inconsistent.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FToroImageData
{
	GENERATED_BODY()

	/** Image width (X) and height (Y) in pixels; both must be positive for valid data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImageData)
	FIntPoint Size;

	/** Row-major color pixels; valid data contains exactly Size.X * Size.Y elements. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImageData)
	TArray<FColor> Pixels;

	/** Creates empty, invalid image data with zero dimensions and no pixels. */
	FToroImageData()
		: Size(FIntPoint::ZeroValue), Pixels({})
	{}

	/**
	 * Copies the supplied dimensions and pixels without validating their consistency.
	 * @param Size Image width (X) and height (Y) in pixels.
	 * @param InPixels Row-major color pixels to copy.
	 */
	FToroImageData(const FIntPoint& Size, const TArray<FColor>& InPixels)
		: Size(Size), Pixels(InPixels)
	{}

	/**
	 * Copies the supplied dimensions and pixels without validating their consistency.
	 * @param SizeX Image width in pixels.
	 * @param SizeY Image height in pixels.
	 * @param InPixels Row-major color pixels to copy.
	 */
	FToroImageData(int32 SizeX, int32 SizeY, const TArray<FColor>& InPixels)
		: Size(SizeX, SizeY), Pixels(InPixels)
	{}

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

	/**
	 * Checks that both dimensions are positive and the pixel count equals their product.
	 * The dimension product is calculated using 64-bit arithmetic to avoid int32 overflow.
	 */
	[[nodiscard]] FORCEINLINE bool IsValid() const
	{
		return Size.X > 0 && Size.Y > 0 && static_cast<int64>(Pixels.Num()) == static_cast<int64>(Size.X) * static_cast<int64>(Size.Y);
	}

	/**
	 * Encodes valid image data as PNG, replacing any existing output bytes.
	 * @param OutData Receives the encoded PNG; cleared when the image data is invalid.
	 * @return True when encoding produces nonempty output; false for invalid data or empty output.
	 */
	[[nodiscard]] bool CompressPNG(TArray64<uint8>& OutData) const;

	/** Resets dimensions to zero and removes all pixels, leaving invalid image data. */
	void Empty();
};
