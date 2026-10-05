// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "HAL/IConsoleManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroCVarLibrary.generated.h"

/**
 * Console-variable lookup, typed reads, and writes within the configured priority range.
 * Setters report whether the variable was found; they do not verify the final value.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroCVarLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Lowest priority used when submitting writes. */
	static constexpr EConsoleVariableFlags MinPriority = ECVF_SetByScalability;

	/** Highest priority used when submitting writes. */
	static constexpr EConsoleVariableFlags MaxPriority = ECVF_SetByConsole;

	/**
	 * Sets a console variable through its current priority, clamped to MinPriority..MaxPriority.
	 * @param InName Console-variable name.
	 * @param InValue Boolean value to submit.
	 * @return True if the variable exists and a write was submitted; false if absent. The final value is not verified.
	 */
	UFUNCTION(BlueprintCallable, Category = ConsoleVariables, DisplayName = "Set Console Variable (Bool)")
	static bool SetCVarBool(const FString& InName, const bool InValue);

	/**
	 * Sets a console variable through its current priority, clamped to MinPriority..MaxPriority.
	 * @param InName Console-variable name.
	 * @param InValue Integer value to submit.
	 * @return True if the variable exists and a write was submitted; false if absent. The final value is not verified.
	 */
	UFUNCTION(BlueprintCallable, Category = ConsoleVariables, DisplayName = "Set Console Variable (Int)")
	static bool SetCVarInt(const FString& InName, const int32 InValue);

	/**
	 * Sets a console variable through its current priority, clamped to MinPriority..MaxPriority.
	 * @param InName Console-variable name.
	 * @param InValue Float value to submit.
	 * @return True if the variable exists and a write was submitted; false if absent. The final value is not verified.
	 */
	UFUNCTION(BlueprintCallable, Category = ConsoleVariables, DisplayName = "Set Console Variable (float)")
	static bool SetCVarFloat(const FString& InName, const float InValue);

	/**
	 * Sets a console variable through its current priority, clamped to MinPriority..MaxPriority.
	 * @param InName Console-variable name.
	 * @param InValue String value to submit.
	 * @return True if the variable exists and a write was submitted; false if absent. The final value is not verified.
	 */
	UFUNCTION(BlueprintCallable, Category = ConsoleVariables, DisplayName = "Set Console Variable (String)")
	static bool SetCVarString(const FString& InName, const FString& InValue);

	/**
	 * Reads a console variable through its boolean accessor.
	 * @param InName Console-variable name.
	 * @param bInDefault Value returned when the variable is absent.
	 */
	UFUNCTION(BlueprintPure, Category = ConsoleVariables, DisplayName = "Get Console Variable (Bool)")
	[[nodiscard]] static bool GetCVarBool(const FString& InName, const bool bInDefault = false);

	/**
	 * Reads a console variable through its integer accessor.
	 * @param InName Console-variable name.
	 * @param InDefault Value returned when the variable is absent.
	 */
	UFUNCTION(BlueprintPure, Category = ConsoleVariables, DisplayName = "Get Console Variable (Int)")
	[[nodiscard]] static int32 GetCVarInt(const FString& InName, const int32 InDefault = 0);

	/**
	 * Reads a console variable through its float accessor.
	 * @param InName Console-variable name.
	 * @param InDefault Value returned when the variable is absent.
	 */
	UFUNCTION(BlueprintPure, Category = ConsoleVariables, DisplayName = "Get Console Variable (float)")
	[[nodiscard]] static float GetCVarFloat(const FString& InName, const float InDefault = 0.0f);

	/**
	 * Reads a console variable through its string accessor.
	 * @param InName Console-variable name.
	 * @param InDefault Value returned when the variable is absent.
	 */
	UFUNCTION(BlueprintPure, Category = ConsoleVariables, DisplayName = "Get Console Variable (String)")
	[[nodiscard]] static FString GetCVarString(const FString& InName, const FString& InDefault = FString());

	/**
	 * Finds a console variable without creating it.
	 * @param InName Console-variable name.
	 * @return Console-manager-owned variable, or nullptr if absent; do not delete it.
	 */
	[[nodiscard]] static IConsoleVariable* FindCVar(const FString& InName);
};
