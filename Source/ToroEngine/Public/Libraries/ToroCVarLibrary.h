// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroCVarLibrary.generated.h"

/**
 * Blueprint utilities for reading registered console variables and requesting updates with bounded priority selection.
 * Typed access delegates conversion to the native console-variable API; setters must run on the game thread.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroCVarLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Minimum priority supplied to native setter priority selection, corresponding to scalability settings. */
	static constexpr EConsoleVariableFlags MinPriority = ECVF_SetByScalability;

	/** Maximum priority supplied to native setter priority selection, corresponding to the interactive console. */
	static constexpr EConsoleVariableFlags MaxPriority = ECVF_SetByConsole;

	/**
	 * Requests a boolean value for an existing console variable on the game thread, using native conversion.
	 * Uses SetWithCurrentPriority() with NAME_None and the MinPriority/MaxPriority bounds.
	 * With native history enabled, selects the highest recorded priority within those bounds, or MinPriority if none exists.
	 * Does not create missing variables or independently verify that the effective value changed.
	 * @param InName Registered console variable name.
	 * @param bInValue Value passed to the native console-variable setter.
	 * @return True if the variable was found and a set request issued; false if lookup failed.
	 */
	UFUNCTION(BlueprintCallable, Category = ConsoleVariables, DisplayName = "Set Console Variable (Bool)")
	static bool SetCVarBool(const FString& InName, bool bInValue);

	/**
	 * Requests a integer value for an existing console variable on the game thread, using native conversion.
	 * Uses SetWithCurrentPriority() with NAME_None and the MinPriority/MaxPriority bounds.
	 * With native history enabled, selects the highest recorded priority within those bounds, or MinPriority if none exists.
	 * Does not create missing variables or independently verify that the effective value changed.
	 * @param InName Registered console variable name.
	 * @param InValue Value passed to the native console-variable setter.
	 * @return True if the variable was found and a set request issued; false if lookup failed.
	 */
	UFUNCTION(BlueprintCallable, Category = ConsoleVariables, DisplayName = "Set Console Variable (Int)")
	static bool SetCVarInt(const FString& InName, int32 InValue);

	/**
	 * Requests a floating-point value for an existing console variable on the game thread, using native conversion.
	 * Uses SetWithCurrentPriority() with NAME_None and the MinPriority/MaxPriority bounds.
	 * With native history enabled, selects the highest recorded priority within those bounds, or MinPriority if none exists.
	 * Does not create missing variables or independently verify that the effective value changed.
	 * @param InName Registered console variable name.
	 * @param InValue Value passed to the native console-variable setter.
	 * @return True if the variable was found and a set request issued; false if lookup failed.
	 */
	UFUNCTION(BlueprintCallable, Category = ConsoleVariables, DisplayName = "Set Console Variable (float)")
	static bool SetCVarFloat(const FString& InName, float InValue);

	/**
	 * Requests a string value for an existing console variable on the game thread, using native conversion.
	 * Uses SetWithCurrentPriority() with NAME_None and the MinPriority/MaxPriority bounds.
	 * With native history enabled, selects the highest recorded priority within those bounds, or MinPriority if none exists.
	 * Does not create missing variables or independently verify that the effective value changed.
	 * @param InName Registered console variable name.
	 * @param InValue Value passed to the native console-variable setter.
	 * @return True if the variable was found and a set request issued; false if lookup failed.
	 */
	UFUNCTION(BlueprintCallable, Category = ConsoleVariables, DisplayName = "Set Console Variable (String)")
	static bool SetCVarString(const FString& InName, const FString& InValue);

	/**
	 * Reads a console variable through the native boolean getter and its conversion rules.
	 * @param InName Registered console variable name.
	 * @param bInDefault Value returned when lookup fails; not a fallback for conversion errors.
	 * @return The native getter's value, or bInDefault if no registered variable is found.
	 */
	UFUNCTION(BlueprintPure, Category = ConsoleVariables, DisplayName = "Get Console Variable (Bool)")
	[[nodiscard]] static bool GetCVarBool(const FString& InName, bool bInDefault = false);

	/**
	 * Reads a console variable through the native integer getter and its conversion rules.
	 * @param InName Registered console variable name.
	 * @param InDefault Value returned when lookup fails; not a fallback for conversion errors.
	 * @return The native getter's value, or InDefault if no registered variable is found.
	 */
	UFUNCTION(BlueprintPure, Category = ConsoleVariables, DisplayName = "Get Console Variable (Int)")
	[[nodiscard]] static int32 GetCVarInt(const FString& InName, int32 InDefault = 0);

	/**
	 * Reads a console variable through the native floating-point getter and its conversion rules.
	 * @param InName Registered console variable name.
	 * @param InDefault Value returned when lookup fails; not a fallback for conversion errors.
	 * @return The native getter's value, or InDefault if no registered variable is found.
	 */
	UFUNCTION(BlueprintPure, Category = ConsoleVariables, DisplayName = "Get Console Variable (float)")
	[[nodiscard]] static float GetCVarFloat(const FString& InName, float InDefault = 0.0f);

	/**
	 * Reads a console variable through the native string getter and its conversion rules.
	 * @param InName Registered console variable name.
	 * @param InDefault Value returned when lookup fails; not a fallback for conversion errors.
	 * @return The native getter's value, or InDefault if no registered variable is found.
	 */
	UFUNCTION(BlueprintPure, Category = ConsoleVariables, DisplayName = "Get Console Variable (String)")
	[[nodiscard]] static FString GetCVarString(const FString& InName, const FString& InDefault = FString());

	/**
	 * Finds a registered console variable without creating it.
	 * @param InName Console variable name; command-only names do not resolve to variables.
	 * @return A borrowed console-manager-owned pointer, or nullptr for missing, unregistered, or command-only names.
	 * @note Do not delete the pointer or assume it remains valid after unregistration.
	 */
	[[nodiscard]] static IConsoleVariable* FindCVar(const FString& InName);
};
