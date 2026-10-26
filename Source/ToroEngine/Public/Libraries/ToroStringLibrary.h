// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroStringLibrary.generated.h"

/**
 * Blueprint helpers for names, mutable strings, path combination, whitespace, and missing-entry placeholder checks.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroStringLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Tests whether the name is NAME_None.
	 * @param InName Name to test.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Name", DisplayName = "Is None")
	static bool IsNameNone(FName InName);

	/**
	 * Formats a name using the engine's display-name word splitting.
	 * @param InName Name to convert to a display string.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Name", DisplayName = "To Display String")
	static FString NameToDisplayString(FName InName);

	/**
	 * Tests whether the string is empty or contains only characters recognized by native trimming as whitespace.
	 * @param InString String to inspect without modifying it.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|String", DisplayName = "Is Empty or Whitespace")
	static bool IsStringEmptyOrWhitespace(const FString& InString);

	/**
	 * Appends text in place without inserting a separator.
	 * @param InString Destination string to extend.
	 * @param Appendage Text to append; empty text leaves the destination unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "Utilities|String", DisplayName = "Append To")
	static void StringAppendTo(UPARAM(ref) FString& InString, const FString& Appendage);

	/**
	 * Appends each string in array order without inserting separators.
	 * @param InString Destination string to extend in place.
	 * @param Appendages Text fragments to append; an empty array leaves the destination unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "Utilities|String", DisplayName = "Append To (Multiple)")
	static void StringAppendToMultiple(UPARAM(ref) FString& InString, const TArray<FString>& Appendages);

	/**
	 * Combines path A followed by path B using FPaths::Combine and native path-separator handling.
	 * Order matters; this does not check filesystem existence or validate the resulting path.
	 * @param A First path segment.
	 * @param B Following path segment.
	 */
	UFUNCTION(BlueprintCallable, Category = "Utilities|Path", DisplayName = "Combine Paths",
		meta = (CommutativeAssociativeBinaryOperator = true, CompactNodeTitle = "/"))
	static FString CombinePaths(const FString& A, const FString& B);

	/**
	 * Tests whether text is empty or whitespace according to the native FText predicate.
	 * @param InText Text to inspect without modifying it.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Text", DisplayName = "Is Empty or Whitespace")
	static bool IsTextEmptyOrWhitespace(const FText& InText);

	/**
	 * Tests whether the displayed text is the engine's missing string-table-entry placeholder.
	 * Does not verify string-table membership or table/key validity; ordinary text, including empty text, can fail.
	 * @param InText Text whose current displayed string is compared with the native placeholder.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Text", DisplayName = "Is String Table Placeholder")
	static bool IsTextStringTablePlaceholder(const FText& InText);
};
