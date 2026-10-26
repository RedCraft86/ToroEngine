// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroActorLibrary.generated.h"

/**
 * Blueprint utilities for actor bounding-box vertices and actor name tags.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroActorLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Collects unique world-space vertices of the actor's axis-aligned bounds, scaled per axis about their center.
	 * Outputs are cleared first; an invalid target leaves an empty vertex set and zero origin and extent.
	 * @param OutVerts Unordered set of at most eight unique vertices; collapsed axes can reduce the count.
	 * @param Origin Center of the original, unscaled world-space bounds.
	 * @param Extent Half-size of the original bounds; unaffected by Scale.
	 * @param Target Actor whose component bounds are queried.
	 * @param Scale Per-axis factors applied to vertex offsets from Origin; vector length below 0.05 yields only Origin.
	 * @param bOnlyColliding Whether to include only components with collision enabled.
	 * @param bChildActors Whether to recursively include bounds from child actor components.
	 */
	UFUNCTION(BlueprintCallable, Category = Actor, meta = (DefaultToSelf = Target))
	static void GetActorBoundingBoxVertices(TSet<FVector>& OutVerts, FVector& Origin, FVector& Extent,
		const AActor* Target, FVector Scale = FVector(1), bool bOnlyColliding = false, bool bChildActors = true);

	/**
	 * Adds an actor name tag without creating a duplicate; invalid targets are ignored.
	 * These entries belong to the actor's Tags array and are distinct from Gameplay Tags.
	 * @param Target Actor to update.
	 * @param InTag Name to add if it is not already present.
	 */
	UFUNCTION(BlueprintCallable, Category = Actor, meta = (DefaultToSelf = Target))
	static void AddActorTag(AActor* Target, FName InTag);

	/**
	 * Removes all matching actor name tags; missing tags and invalid targets have no effect.
	 * These entries belong to the actor's Tags array and are distinct from Gameplay Tags.
	 * @param Target Actor to update.
	 * @param InTag Name whose matching entries are removed.
	 */
	UFUNCTION(BlueprintCallable, Category = Actor, meta = (DefaultToSelf = Target))
	static void RemoveActorTag(AActor* Target, FName InTag);
};
