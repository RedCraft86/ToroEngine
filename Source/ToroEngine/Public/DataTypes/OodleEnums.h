// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Compression/OodleDataCompression.h"
#include "OodleEnums.generated.h"

/**
 * Blueprint-compatible Oodle compressor choices, ordered from fastest to slowest decompression.
 * Slower choices generally provide higher compression ratios; compression level controls encoding effort.
 * Values are offset from the native enum to omit its NotSet sentinel.
 */
UENUM(BlueprintType)
namespace EOodleCompressor
{
	enum Type : uint8
	{
		/** Fastest decompression with the lowest compression ratio of these compressors. */
		Selkie = 0,		// Native: 1

		/** Favors decompression speed over the higher compression ratios of Kraken and Leviathan. */
		Mermaid = 1,	// Native: 2

		/** Balances decompression speed and compression ratio; a general starting choice. */
		Kraken = 2,		// Native: 3

		/** Highest compression ratio with the slowest decompression of these compressors. */
		Leviathan = 3	// Native: 4
	};
}
namespace EOodleCompressor
{
	/**
	 * Converts a reflected compressor choice to its corresponding native compressor.
	 * @param InCompressor A declared EOodleCompressor::Type value; arbitrary values are not validated.
	 */
	inline FOodleDataCompression::ECompressor ToNative(Type InCompressor)
	{
		return static_cast<FOodleDataCompression::ECompressor>(static_cast<uint8>(InCompressor) + 1);
	}

	/**
	 * Converts a native compressor to its reflected choice, mapping NotSet to Selkie.
	 * @param InCompressor A declared native compressor value; arbitrary values are not validated.
	 * @note NotSet cannot round-trip because the reflected enum has no corresponding sentinel.
	 */
	inline Type FromNative(FOodleDataCompression::ECompressor InCompressor)
	{
		return static_cast<Type>(FMath::Max(static_cast<int8>(InCompressor) - 1, 0));
	}
}

/**
 * Blueprint-compatible Oodle compression levels, trading encoding speed for compression ratio.
 * HyperFast levels favor real-time encoding, while Optimal levels favor slower offline encoding.
 * Values are offset by four to represent the native negative levels in an unsigned enum.
 */
UENUM(BlueprintType)
namespace EOodleCompressionLevel
{
	enum Type : uint8
	{
		/** Fastest HyperFast encoding setting, favoring speed over compression ratio. */
		HyperFast4 = 0,	// Native: -4

		/** HyperFast encoding with more compression effort than HyperFast4. */
		HyperFast3 = 1,	// Native: -3

		/** HyperFast encoding with more compression effort than HyperFast3. */
		HyperFast2 = 2,	// Native: -2

		/** Highest compression effort within the HyperFast range. */
		HyperFast1 = 3,	// Native: -1

		/** Disables compression. */
		None = 4,		// Native:  0

		/** Fastest standard compression level. */
		SuperFast = 5,	// Native:  1

		/** Standard compression with more encoding effort than SuperFast. */
		VeryFast = 6,	// Native:  2

		/** Standard compression with more encoding effort than VeryFast. */
		Fast = 7,		// Native:  3

		/** Highest standard compression level; a general starting choice. */
		Normal = 8,		// Native:  4

		/** Lowest encoding effort within the Optimal range, intended for offline compression. */
		Optimal1 = 9,	// Native:  5

		/** Offline compression with more encoding effort than Optimal1. */
		Optimal2 = 10,	// Native:  6

		/** Offline compression with more encoding effort than Optimal2. */
		Optimal3 = 11,	// Native:  7

		/** Offline compression with more encoding effort than Optimal3. */
		Optimal4 = 12,	// Native:  8

		/** Highest encoding effort within the Optimal range. */
		Optimal5 = 13	// Native:  9
	};
}
namespace EOodleCompressionLevel
{
	/**
	 * Converts a reflected compression level to its corresponding native level.
	 * @param InLevel A declared EOodleCompressionLevel::Type value; arbitrary values are not validated.
	 */
	inline FOodleDataCompression::ECompressionLevel ToNative(Type InLevel)
	{
		return static_cast<FOodleDataCompression::ECompressionLevel>(static_cast<int8>(InLevel) - 4);
	}

	/**
	 * Converts a native compression level to its corresponding reflected level.
	 * @param InLevel A declared native compression level; arbitrary values are not validated.
	 */
	inline Type FromNative(FOodleDataCompression::ECompressionLevel InLevel)
	{
		return static_cast<Type>(static_cast<int8>(InLevel) + 4);
	}
}
