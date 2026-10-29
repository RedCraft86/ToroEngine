// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "DataTypes/ToroInputConfig.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroInputLibrary.generated.h"

class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;

/**
 * Blueprint utilities for applying player input configurations and managing Enhanced Input mapping contexts.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroInputLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Clears cached native input modes and the focus widget without applying changes to a player controller.
	 * Preserves input routing, cursor visibility, mouse locking, and capture settings.
	 * @param Config Configuration whose cached modes and focus target are reset.
	 */
	UFUNCTION(BlueprintCallable, Category = "Utilities|Input")
	static void ResetInputConfig(UPARAM(ref) FToroInputConfig& Config);

	/**
	 * Applies cursor visibility and the prepared native input mode, then flushes the selected controller's pressed keys.
	 * Updates the configuration's internal mode cache; an unavailable player controller is ignored.
	 * @param Config Input configuration to prepare and apply.
	 * @param PlayerIdx Player controller index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player|Input", meta = (WorldContext = ContextObject))
	static void SetPlayerInputConfig(const UObject* ContextObject, UPARAM(ref) FToroInputConfig& Config, int32 PlayerIdx = 0);

	/**
	 * Resolves the selected player controller's Enhanced Input local-player subsystem.
	 * @param PlayerIdx Player controller index.
	 * @return The subsystem, or nullptr if the controller, local player, or subsystem is unavailable.
	 */
	UFUNCTION(BlueprintPure, Category = "Game|Player|Input", meta = (WorldContext = ContextObject))
	static UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputPlayerSubsystem(const UObject* ContextObject, int32 PlayerIdx = 0);

	/**
	 * Adds a mapping context using the engine's default modification options and requests a native mapping rebuild when needed.
	 * Higher priorities are evaluated first and can block lower-priority mappings when they consume input.
	 * Repeated additions follow the context's registration-tracking mode; counted registrations require matching removals.
	 * @param Mappings Context to apply; a null context is passed to the engine, which logs a warning when the subsystem is available.
	 * @param Priority Mapping priority relative to other contexts.
	 * @param PlayerIdx Player controller index used to resolve the local-player subsystem; unavailable subsystems are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player|Input", meta = (WorldContext = ContextObject))
	static void AddInputMappingContext(const UObject* ContextObject, const UInputMappingContext* Mappings, int32 Priority = 0, int32 PlayerIdx = 0);

	/**
	 * Removes a mapping context using the engine's default modification options and requests a native mapping rebuild when needed.
	 * Missing or null contexts have no effect. For counted registrations, the context remains until its registration count reaches zero.
	 * @param Mappings Context whose registration is removed.
	 * @param PlayerIdx Player controller index used to resolve the local-player subsystem; unavailable subsystems are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player|Input", meta = (WorldContext = ContextObject))
	static void RemoveInputMappingContext(const UObject* ContextObject, const UInputMappingContext* Mappings, int32 PlayerIdx = 0);

	/**
	 * Clears all applied mapping contexts from the selected local player's input and requests a native mapping rebuild.
	 * Includes contexts added elsewhere; has no effect when the subsystem or its player input is unavailable.
	 * @param PlayerIdx Player controller index used to resolve the local-player subsystem.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player|Input", meta = (WorldContext = ContextObject))
	static void ClearInputMappingContexts(const UObject* ContextObject, int32 PlayerIdx = 0);
};
