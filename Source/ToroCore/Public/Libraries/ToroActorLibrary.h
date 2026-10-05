// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroActorLibrary.generated.h"

/**
 * Actor tag helpers and world-space axis-aligned bounding-box queries.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroActorLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Collects distinct corners of the actor's world-space axis-aligned bounds.
	 * @param OutVerts Replaced with scaled corners; empty for invalid targets. Degenerate corners collapse in the set.
	 * @param Origin Receives the unscaled bounds center, or zero for an invalid target.
	 * @param Extent Receives the unscaled bounds half-size, or zero for an invalid target.
	 * @param Target Actor whose component bounds are queried.
	 * @param Scale Per-axis corner scale about Origin; vector magnitude below 0.05 yields only Origin.
	 * @param bOnlyColliding Include only collision-enabled components.
	 * @param bChildActors Include bounds from child actors.
	 */
	UFUNCTION(BlueprintCallable, Category = Actor, meta = (DefaultToSelf = Target))
	static void GetBoundingBoxVertices(TSet<FVector>& OutVerts, FVector& Origin, FVector& Extent, const AActor* Target,
		const FVector Scale = FVector(1), const bool bOnlyColliding = false, const bool bChildActors = true);

	/**
	 * Adds an actor tag once; invalid targets are ignored.
	 * @param Target Actor to modify.
	 * @param InTag Tag to add, including NAME_None if supplied.
	 */
	UFUNCTION(BlueprintCallable, Category = Actor, DisplayName = "Add Tag", meta = (DefaultToSelf = Target))
	static void AddActorTag(AActor* Target, const FName InTag);

	/**
	 * Removes all occurrences of an actor tag; invalid targets are ignored.
	 * @param Target Actor to modify.
	 * @param InTag Tag to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = Actor, DisplayName = "Remove Tag", meta = (DefaultToSelf = Target))
	static void RemoveActorTag(AActor* Target, const FName InTag);
};
