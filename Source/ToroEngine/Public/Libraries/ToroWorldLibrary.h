// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UE5Coro.h"
#include "DataTypes/MiscEnums.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroWorldLibrary.generated.h"

/**
 * Blueprint utilities for world resolution, map travel, level-script events, and level streaming.
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
	 * Reports requested loading and visibility flags for an existing streaming level.
	 * This is not a completed-state query: loading, unloading, or visibility transitions may still be in progress.
	 * @param Level Soft world reference identifying the streaming level by package name.
	 * @return Unloaded for a missing streaming object or when loading is not requested;
	 *         otherwise Visible when visibility is requested, or Loaded when it is not.
	 */
	UFUNCTION(BlueprintCallable, Category = World, meta = (WorldContext = ContextObject))
	[[nodiscard]] static EToroLevelStreamState GetLevelStreamState(const UObject* ContextObject, const TSoftObjectPtr<UWorld>& Level);

	/**
	 * Requests a nonblocking streaming transition and waits for the native action to complete.
	 * Loaded explicitly requests a loaded, hidden level and additionally waits for visibility to clear,
	 * ending that visibility wait if the streaming object becomes invalid.
	 * Uses an existing streaming object rather than creating a streaming instance.
	 * Completion provides no separate success result or exclusive control over concurrent streaming requests.
	 * @param StreamedLevel Receives the streaming object found before requesting the transition, or nullptr if none is found.
	 *        A found object is also returned for unsupported states; it may become invalid while waiting.
	 * @param Level Soft world reference identifying the existing streaming level by package name.
	 * @param State Unloaded, Loaded (hidden), or Visible; unsupported values cause no streaming changes.
	 */
	UFUNCTION(BlueprintCallable, Category = World, meta = (Latent, LatentInfo = LatentInfo, WorldContext = ContextObject))
	static FVoidCoroutine SetLevelStreamState(FLatentActionInfo LatentInfo, const UObject* ContextObject,
		ULevelStreaming*& StreamedLevel, TSoftObjectPtr<UWorld> Level, EToroLevelStreamState State);

	/**
	 * Tests whether a level is its owning world's persistent level.
	 * @param Target Level to test.
	 * @return True if the valid level equals its valid owning world's persistent level; false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = Level, meta = (DefaultToSelf = Target))
	[[nodiscard]] static bool IsPersistentLevel(const ULevel* Target);
};
