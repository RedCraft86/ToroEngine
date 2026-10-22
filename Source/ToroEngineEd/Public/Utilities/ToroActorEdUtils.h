// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Math/Vector2D.h"
#include "Math/Transform.h"
#include "Containers/Array.h"
#include "Templates/SubclassOf.h"

class AActor;
class UStaticMesh;
class UEditorActorSubsystem;
class UProceduralMeshComponent;

/** Editor-thread utilities for mesh conversion, actor selection, and grid layout. */
namespace ToroEngine::ActorEdUtils
{
	/** Returns the editor actor subsystem, or null with an ensure diagnostic when unavailable. */
	[[nodiscard]] TOROENGINEED_API UEditorActorSubsystem* GetSubsystem();

	/**
	 * Destroys actors through the editor subsystem inside a deletion undo transaction.
	 * @param Actors Actors passed to the editor subsystem for deletion.
	 * @return The subsystem deletion result, or false when the subsystem is unavailable.
	 */
	TOROENGINEED_API bool DestroyActors(const TArray<AActor*>& Actors);

	/**
	 * Returns the selected level actors, or an empty array if none or no subsystem is available.
	 * @param bWarnEmpty Shows an editor notification when the returned selection is empty.
	 */
	[[nodiscard]] TOROENGINEED_API TArray<AActor*> GetSelectedActors(bool bWarnEmpty);

	/**
	 * Spawns an editor actor and applies the supplied world location, rotation, and scale.
	 * @param Class Actor class to spawn.
	 * @param Transform Initial world transform.
	 * @return The new actor, or null when the class, subsystem, or spawn is unavailable.
	 */
	[[nodiscard]] TOROENGINEED_API AActor* CreateActor(const TSubclassOf<AActor> Class, const FTransform& Transform);

	/**
	 * Bakes supported mesh components into separate actors and updates editor selection.
	 * Supports static, instanced/hierarchical, spline, skeletal, and procedural meshes.
	 * Copies supported mesh/material/lighting and animation settings, not unrelated actor
	 * components or arbitrary gameplay state. Procedural meshes prompt for an unsaved asset.
	 * @param Actors Source actors; invalid entries are skipped. Duplicate inputs are not filtered.
	 * @param bDestroySource Deletes actors with at least one successful component bake.
	 *         This does not verify that every component or instance was converted.
	 * @return Created actors plus source entries whose component bake produced no output.
	 *         Entries may repeat, and selected source entries may subsequently be deleted.
	 *         Conversion and source deletion use separate transactions.
	 */
	TOROENGINEED_API TArray<AActor*> SplitActors(const TArray<AActor*>& Actors, bool bDestroySource);

	/**
	 * Groups static/instanced meshes by preset and bakes other supported mesh types separately.
	 * A single grouped transform produces a static actor; larger groups produce ISM/HISM.
	 * Uses world-space instance transforms. Unrelated actor components and arbitrary gameplay
	 * state are not copied; procedural meshes prompt for independent unsaved mesh assets.
	 * @param Actors Source actors; invalid entries are skipped during conversion.
	 * @param bUseHISM Uses hierarchical instancing for groups with multiple transforms.
	 * @param bDestroySource Requests deletion of all input actors after conversion, including
	 *         sources whose conversion failed or produced no output.
	 * @return Created actors, also selected in the editor; failed sources are not appended.
	 *         Conversion and source deletion use separate transactions.
	 */
	TOROENGINEED_API TArray<AActor*> InstanceActors(const TArray<AActor*>& Actors, bool bUseHISM, bool bDestroySource);

	/**
	 * Places valid actor entries on an XY grid using their bounds before applying the yaw.
	 * Sorts by descending XY bounds area and preserves scale. Duplicate inputs are not filtered.
	 * @param Actors Actors to arrange; invalid entries are skipped.
	 * @param MaxColumns Maximum actors per row; zero is rejected.
	 * @param YawOffset Absolute yaw in degrees; pitch and roll are set to zero.
	 * @param XYOffset Extra cell spacing; negative values permit overlap.
	 * @return True when layout was attempted for at least one valid actor; transform-setting
	 *         failures are not reflected in this result. No outer layout transaction is created.
	 */
	TOROENGINEED_API bool LayoutActors(const TArray<AActor*>& Actors, uint8 MaxColumns, float YawOffset, const FVector2D& XYOffset);

	/**
	 * Creates a static mesh asset from procedural geometry using a modal asset-path dialog.
	 * Preserves section material ordering and simple/complex collision intent. Builds the mesh
	 * synchronously, registers it, and marks its package dirty without saving it to disk.
	 * The asset remains independent of actor-conversion success or actor Undo/Redo.
	 * @param Source Valid procedural mesh component.
	 * @return The new unsaved asset, or null for an invalid source, cancellation, or no polygons.
	 */
	TOROENGINEED_API UStaticMesh* BakeProceduralMesh(const UProceduralMeshComponent* Source);
}
