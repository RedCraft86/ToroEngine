// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WorldPartition/DataLayer/DataLayerInstance.h"
#include "ToroWorldLibrary.generated.h"

class UWorld;
class ULevel;
class UDataLayerAsset;

/**
 * Blueprint utilities for world resolution, map travel, level-script events, and runtime Data Layers.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroWorldLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Resolves a valid world from an object on the game thread.
	 * @param Target Object supplying the world; may be null when fallback selection is enabled.
	 * @param bTryFindPossible Whether to try the current play world, then GWorld, if context resolution fails.
	 *        Fallback selection is best-effort; supply an appropriate target when world identity matters.
	 * @return The resolved world, or nullptr if no valid world is found.
	 */
	UFUNCTION(BlueprintPure, Category = World, meta = (DefaultToSelf = Target))
	[[nodiscard]] static UWorld* GetWorldFromObject(const UObject* Target, bool bTryFindPossible = false);

	/**
	 * Requests travel to the current map, using its name without the streaming prefix.
	 * Invalid contexts or contexts without a resolvable world do not initiate travel.
	 * @param bAbsolute Whether to discard existing travel options; false retains them before applying Options.
	 * @param Options Additional travel options passed to the native OpenLevel request.
	 */
	UFUNCTION(BlueprintCallable, Category = World, meta = (WorldContext = ContextObject, AdvancedDisplay = "bAbsolute, Options"))
	static void ReloadLevel(const UObject* ContextObject, bool bAbsolute = true, const FString& Options = FString());

	/**
	 * Invokes matching parameterless events on visible levels' script actors in the context's world.
	 * Dispatches locally through the world's level script actor; this does not send a network RPC.
	 * Empty names or an unavailable context world or dispatching script actor produce no action.
	 * @param EventName Name of the parameterless event to invoke.
	 */
	UFUNCTION(BlueprintCallable, Category = World, meta = (WorldContext = ContextObject))
	static void CallRemoteEvent(const UObject* ContextObject, FName EventName);

	/**
	 * Tests whether a level is its owning world's persistent level.
	 * @param Target Level to test.
	 * @return True if the valid level equals its valid owning world's persistent level; false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = World, meta = (DefaultToSelf = Target))
	[[nodiscard]] static bool IsPersistentLevel(const ULevel* Target);

	/**
	 * Returns the requested state of a runtime Data Layer in the current world.
	 * Returns Unloaded if the layer is unavailable. Use bEffective to account for parent layers.
	 * The state does not indicate whether cell streaming has finished.
	 */
	UFUNCTION(BlueprintPure, Category = World, meta = (WorldContext = ContextObject, AdvancedDisplay = "bEffective"))
	[[nodiscard]] static EDataLayerRuntimeState GetDataLayerState(const UObject* ContextObject,
		const UDataLayerAsset* DataLayer, bool bEffective = false);

	/**
	 * Requests a runtime state for a Data Layer in the current world.
	 * Returns whether the engine accepted the request. Cell streaming completes asynchronously.
	 */
	UFUNCTION(BlueprintCallable, Category = World, meta = (WorldContext = ContextObject))
	static bool SetDataLayerState(const UObject* ContextObject,
		const UDataLayerAsset* DataLayer, EDataLayerRuntimeState State);
};
