// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "DataTypes/OodleEnums.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroUtilityLibrary.generated.h"

/**
 * Execution-mode, package-loading, and Unreal Oodle compressed-array helpers.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroUtilityLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Returns FApp::IsGame() in editor builds and true in non-editor builds; this is not a PIE-state query.
	 */
	UFUNCTION(BlueprintPure, Category = Utility)
	[[nodiscard]] static bool IsInGame();

	/**
	 * Returns whether the engine reports package loading through IsLoading(), including its global loading state.
	 */
	UFUNCTION(BlueprintPure, Category = Utility)
	[[nodiscard]] static bool IsAnyPackageLoading();

	/**
	 * Returns the engine's current global pending async-package count, not a per-world count.
	 */
	UFUNCTION(BlueprintPure, Category = Utility)
	[[nodiscard]] static int32 GetNumAsyncPackages();

	/**
	 * Compresses bytes into the Unreal FOodleCompressedArray format, including its size header.
	 * @param InData Bytes to encode; may be the same array as OutData.
	 * @param OutData Replaced with compressed bytes on success; emptied on failure.
	 * @param Compressor Named supported Oodle compressor.
	 * @param Level Named supported compression-effort level.
	 * @return True when compression succeeds; false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = Compression)
	[[nodiscard]] static bool OodleCompress(const TArray<uint8>& InData, TArray<uint8>& OutData,
		EOodleCompressor Compressor = EOodleCompressor::Kraken,
		EOodleCompressionLevel Level = EOodleCompressionLevel::SuperFast);

	/**
	 * Decodes Unreal FOodleCompressedArray data produced by OodleCompress.
	 * @param InData Compressed bytes; may be the same array as OutData. This is not a raw Oodle stream.
	 * @param OutData Replaced with decoded bytes on success; emptied on failure.
	 * @return True when decoding succeeds; false for empty, malformed, or undecodable input.
	 */
	UFUNCTION(BlueprintCallable, Category = Compression)
	[[nodiscard]] static bool OodleDecompress(const TArray<uint8>& InData, TArray<uint8>& OutData);
};
