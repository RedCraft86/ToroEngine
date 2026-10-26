// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroPlatformLibrary.generated.h"

/**
 * Blueprint utilities for native clipboard access, Pak archive mounting, and synchronous text-file input and output.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroPlatformLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Copies text using the native platform clipboard API.
	 * Availability depends on the platform and application environment; no success result is exposed.
	 * @param InString Text to place on the clipboard.
	 */
	UFUNCTION(BlueprintCallable, Category = "Platform|Clipboard")
	static void CopyStringToClipboard(const FString& InString);

	/**
	 * Retrieves text using the native platform clipboard API.
	 * Availability depends on the platform and application environment.
	 * @return The retrieved text; an empty result does not distinguish empty content from an unavailable clipboard.
	 */
	UFUNCTION(BlueprintCallable, Category = "Platform|Clipboard")
	static FString GetStringFromClipboard();

	/**
	 * Requests Pak mounting through the initialized engine delegate with mount order zero.
	 * Requires a platform configuration that requires cooked data; unsupported configurations or unbound delegates log an error.
	 * Mounting exposes archive files but does not itself load assets.
	 * @param FilePath Pak archive path passed to the engine delegate.
	 * @return True if the delegate returns a mounted Pak object; false on rejection or native mount failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "Platform|Pak")
	static bool MountPakFile(const FString& FilePath);

	/**
	 * Requests Pak unmounting through the initialized engine delegate.
	 * Requires a platform configuration that requires cooked data; unsupported configurations or unbound delegates log an error.
	 * @param FilePath Pak archive path passed to the engine delegate.
	 * @return The native unmount result, or false when cooked-data support or the delegate is unavailable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Platform|Pak")
	static bool UnmountPakFile(const FString& FilePath);

	/**
	 * Collects archive filenames from the active Pak platform-file layer.
	 * @param FileNames Replaced with mounted Pak filenames; empty when the initialized layer is unavailable or no archives are mounted.
	 */
	UFUNCTION(BlueprintPure, Category = "Platform|Pak")
	static void GetMountedPakFileNames(TArray<FString>& FileNames);

	/**
	 * Synchronously loads a text file using native text decoding.
	 * @param FilePath Source filename interpreted by the native file system.
	 * @param OutContent Replaced with decoded text on success and cleared on failure; may also supply FilePath.
	 * @return True if native loading succeeds, including an empty file; false for missing or unreadable files or other load failures.
	 */
	UFUNCTION(BlueprintCallable, Category = "Platform|File")
	static bool ReadFileToString(const FString& FilePath, FString& OutContent);

	/**
	 * Synchronously creates or overwrites a text file using native automatic encoding and default write flags.
	 * Subject to native filesystem and platform restrictions; adds no custom directory-creation behavior.
	 * @param FilePath Destination filename interpreted by the native file system.
	 * @param InContent Text to write, replacing any existing file content.
	 * @return The native save result; false when the destination cannot be written.
	 */
	UFUNCTION(BlueprintCallable, Category = "Platform|File")
	static bool WriteStringToFile(const FString& FilePath, const FString& InContent);
};
