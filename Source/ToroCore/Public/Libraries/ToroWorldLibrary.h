// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "UE5Coro.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroWorldLibrary.generated.h"

/**
 * Requested streaming state; transition completion may lag these flags.
 */
UENUM(BlueprintType)
enum class EToroLevelStreamState : uint8
{
	/** Request no loaded or visible streaming level. */
	Unloaded,

	/** Request residency without visibility. */
	Loaded,

	/** Request residency and visibility. */
	Visible
};

/**
 * World lookup, level reloading, remote events, and streamed-level state helpers.
 * Invoke on the game thread. FWorldGetter-based operations may use a fallback world.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroWorldLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Resolves a world through FWorldGetter; call only from the game thread.
	 * @param Context Optional context. Missing or unresolved contexts allow fallback to a cached, play, or global world, potentially from another PIE instance.
	 * @return Resolved or fallback world, or nullptr if none is available.
	 */
	UFUNCTION(BlueprintPure, Category = World, meta = (DefaultToSelf = Context, AdvancedDisplay = Context))
	[[nodiscard]] static UWorld* GetPossibleWorld(const UObject* Context);

	/**
	 * Reopens the current persistent map when world lookup succeeds.
	 * @param bAbsolute Reset existing travel options rather than carry them forward.
	 * @param Options Additional travel options passed to OpenLevel.
	 */
	UFUNCTION(BlueprintCallable, Category = World, meta = (WorldContext = ContextObject, AdvancedDisplay = "bAbsolute, Options"))
	static void ReloadLevel(const UObject* ContextObject, const bool bAbsolute = true, const FString& Options = FString());

	/**
	 * Invokes the named remote event through the world's level script actor.
	 * @param EventName Remote event name; NAME_None or an unavailable level script actor is ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = World, meta = (WorldContext = ContextObject))
	static void CallRemoteEvent(const UObject* ContextObject, const FName EventName);

	/**
	 * Returns requested streaming flags, not the actual resident or visible state during transitions.
	 * @param Level Existing streaming-level asset to query; missing entries report Unloaded.
	 */
	UFUNCTION(BlueprintCallable, Category = World, meta = (WorldContext = ContextObject))
	[[nodiscard]] static EToroLevelStreamState GetLevelStreamState(const UObject* ContextObject, const TSoftObjectPtr<UWorld>& Level);

	/**
	 * Requests a streaming state and awaits the corresponding load, hide, or unload operation.
	 * The level must already have a streaming entry; this does not create one.
	 * @param StreamedLevel Receives the found streaming entry, or nullptr when absent.
	 * @param Level Streaming-level asset to modify.
	 * @param State Desired state. Loaded requests hidden residency; Visible requests loaded visibility.
	 */
	UFUNCTION(BlueprintCallable, Category = World, meta = (Latent, LatentInfo = LatentInfo, WorldContext = ContextObject))
	static FVoidCoroutine SetLevelStreamState(FLatentActionInfo LatentInfo, const UObject* ContextObject,
		ULevelStreaming*& StreamedLevel, const TSoftObjectPtr<UWorld> Level, const EToroLevelStreamState State);
};
