// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Math/UnrealMathUtility.h"
#include "Compression/OodleDataCompression.h"
#include "OodleEnums.generated.h"

/**
 * Blueprint-compatible Oodle compression algorithm selection.
 * Values are one less than the corresponding native ECompressor values.
 * Native NotSet has no corresponding enumerator.
 */
UENUM(BlueprintType)
enum class EOodleCompressor : uint8
{
	Selkie		= 0,  // Native: 1
	Mermaid		= 1,  // Native: 2
	Kraken		= 2,  // Native: 3
	Leviathan	= 3   // Native: 4
};

/**
 * Converts a Blueprint-compatible compressor to its native Oodle counterpart.
 * @param InCompressor A named EOodleCompressor value. Out-of-range values are not validated.
 */
inline FOodleDataCompression::ECompressor OodleCompressorToNative(const EOodleCompressor InCompressor)
{
	return static_cast<FOodleDataCompression::ECompressor>(static_cast<uint8>(InCompressor) + 1);
}

/**
 * Converts a native Oodle compressor to its Blueprint-compatible counterpart.
 * Native NotSet maps to Selkie, so converting back does not preserve the unset state.
 * @param InCompressor A named native ECompressor value, including NotSet.
 *        Other values are not validated and may produce unnamed enum values.
 */
inline EOodleCompressor NativeToOodleCompressor(const FOodleDataCompression::ECompressor InCompressor)
{
	return static_cast<EOodleCompressor>(FMath::Max(static_cast<int8>(InCompressor) - 1, 0));
}

/**
 * Blueprint-compatible Oodle compression effort level.
 * Values are four greater than native ECompressionLevel values, allowing native
 * levels from -4 through 9 to be represented by an unsigned Blueprint enum.
 * Lower levels favor compression speed; higher levels favor compression ratio.
 */
UENUM(BlueprintType)
enum class EOodleCompressionLevel : uint8
{
	HyperFast4	= 0,  // Native: -4
	HyperFast3	= 1,  // Native: -3
	HyperFast2	= 2,  // Native: -2
	HyperFast1	= 3,  // Native: -1
	None		= 4,  // Native:  0
	SuperFast	= 5,  // Native:  1
	VeryFast	= 6,  // Native:  2
	Fast		= 7,  // Native:  3
	Normal		= 8,  // Native:  4
	Optimal1	= 9,  // Native:  5
	Optimal2	= 10, // Native:  6
	Optimal3	= 11, // Native:  7
	Optimal4	= 12, // Native:  8
	Optimal5	= 13  // Native:  9
};

/**
 * Converts a Blueprint-compatible compression level to its native Oodle counterpart.
 * @param InLevel A named EOodleCompressionLevel value. Out-of-range values are not validated.
 */
inline FOodleDataCompression::ECompressionLevel OodleCompressionLevelToNative(const EOodleCompressionLevel InLevel)
{
	return static_cast<FOodleDataCompression::ECompressionLevel>(static_cast<int8>(InLevel) - 4);
}

/**
 * Converts a native Oodle compression level to its Blueprint-compatible counterpart.
 * @param InLevel A named native ECompressionLevel value from HyperFast4 (-4) through
 *        Optimal5 (9). Out-of-range values are not validated and may produce unnamed enum values.
 */
inline EOodleCompressionLevel NativeToOodleCompressionLevel(const FOodleDataCompression::ECompressionLevel InLevel)
{
	return static_cast<EOodleCompressionLevel>(static_cast<int8>(InLevel) + 4);
}
