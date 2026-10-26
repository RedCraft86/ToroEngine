// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DataTypes/OodleEnums.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroUtilityLibrary.generated.h"

/**
 * Blueprint utilities for game-state queries, global async-loading activity, and synchronous Oodle byte-array compression.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroUtilityLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Reports whether the application is currently acting as a game.
	 * Editor builds use FApp::IsGame(), including PIE game execution and dedicated servers;
	 * this depends on current game-thread editor state and is not a reliable worker-thread query.
	 * Non-editor builds always return true.
	 */
	UFUNCTION(BlueprintPure, Category = Utility)
	[[nodiscard]] static bool IsInGame();

	/**
	 * Reports global async package-loading activity through the native engine query.
	 * This does not report whether individual requests succeeded or provide a stable progress percentage.
	 */
	UFUNCTION(BlueprintPure, Category = Utility)
	[[nodiscard]] static bool IsAsyncLoading();

	/**
	 * Gets the current global count of active async package-load requests.
	 * The count can change as requests arrive or complete; it is not a total, success count, or stable progress percentage.
	 */
	UFUNCTION(BlueprintPure, Category = Utility)
	[[nodiscard]] static int32 GetNumAsyncPackages();

	/**
	 * Pauses global shader pipeline-cache PSO precompilation batching by incrementing the native pause count.
	 * Nested pauses require matching resume calls before batching is enabled again.
	 * Has no effect without an initialized cache; does not wait for or cancel already-issued compilation work.
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering | Pipeline Cache")
	static void PausePrecompilingPSOs();

	/**
	 * Decrements the global shader pipeline-cache pause count and enables batching when the count reaches zero.
	 * Additional outstanding pauses keep batching paused; the native count is clamped to zero on resumption.
	 * Has no effect without an initialized cache and does not wait for compilation to complete.
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering | Pipeline Cache")
	static void ResumePrecompilingPSOs();

	/**
	 * Synchronously compresses bytes into the native size-prefixed FOodleCompressedArray format.
	 * The output includes size metadata and is intended for OodleDecompress(), rather than raw Oodle-stream decoding.
	 * @param InData Bytes to compress; native empty-input behavior is preserved.
	 * @param OutData Replaced with encoded bytes on success and cleared on failure; may be the same array as InData.
	 * @param Compressor Declared compressor choice from Selkie through Leviathan.
	 * @param Level Declared compression level from HyperFast4 through Optimal5.
	 * @return True if native compression succeeds; false for invalid settings or native compression failure.
	 */
	UFUNCTION(BlueprintCallable, Category = Compression)
	[[nodiscard]] static bool OodleCompress(const TArray<uint8>& InData, TArray<uint8>& OutData,
		TEnumAsByte<EOodleCompressor::Type> Compressor = EOodleCompressor::Kraken,
		TEnumAsByte<EOodleCompressionLevel::Type> Level = EOodleCompressionLevel::SuperFast);

	/**
	 * Synchronously decompresses compatible size-prefixed FOodleCompressedArray data, not a raw Oodle stream.
	 * Validates the 32-bit header and payload bounds before allocation; unsupported 64-bit headers are rejected.
	 * Valid headers determine output allocation size; this helper does not impose a separate decompressed-size cap.
	 * @param InData Size-prefixed data produced by compatible FOodleCompressedArray compression.
	 * @param OutData Replaced with decoded bytes on success and cleared on failure; may be the same array as InData.
	 * @return True if decoding succeeds; false for empty input, an invalid or unsupported header, truncated data, or native decode failure.
	 */
	UFUNCTION(BlueprintCallable, Category = Compression)
	[[nodiscard]] static bool OodleDecompress(const TArray<uint8>& InData, TArray<uint8>& OutData);
};
