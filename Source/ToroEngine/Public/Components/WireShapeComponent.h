// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DataTypes/WireShapesData.h"
#include "Components/ActorComponent.h"
#include "WireShapeComponent.generated.h"

/**
 * Stores named shapes and labels for editor component visualization.
 * Shape positions and rotations are relative to the owner, with owner scale ignored;
 * generated navigation path points are cached in world space.
 * While selected in the editor outside game mode, refreshes navigation paths on a 0.1-second tick interval.
 * In editor builds, disables ticking at BeginPlay and defers self-destruction through the core ticker.
 * Shape collections are editor-only data; viewport drawing is supplied by the editor visualizer.
 */
UCLASS(MinimalAPI, NotBlueprintable, NotBlueprintType, ClassGroup = (Editor), meta = (BlueprintSpawnableComponent))
class UWireShapeComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UWireShapeComponent();

#if WITH_EDITORONLY_DATA
	/** Named point markers at owner-local positions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shapes)
	TMap<FName, FWirePointData> WirePoints;

	/** Named line segments with owner-local endpoints. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shapes)
	TMap<FName, FWireLineData> WireLines;

	/** Named circles with owner-local centers and orientations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shapes)
	TMap<FName, FWireCircleData> WireCircles;

	/** Named outlined circular sectors with owner-local centers and orientations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shapes)
	TMap<FName, FWireArcData> WireArcs;

	/** Named wire boxes with owner-local centers and orientations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shapes)
	TMap<FName, FWireBoxData> WireBoxes;

	/** Named wire spheres at owner-local centers. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shapes)
	TMap<FName, FWireSphereData> WireSpheres;

	/** Named wire cylinders with owner-local centers and orientations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shapes)
	TMap<FName, FWireCylinderData> WireCylinders;

	/** Named wire capsules with owner-local centers and orientations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shapes)
	TMap<FName, FWireCapsuleData> WireCapsules;

	/** Named navigation routes with owner-local targets and cached world-space paths, labeled by map key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shapes)
	TMap<FName, FWireNavPathData> WireNavPaths;

	/** Labels at owner-local positions; empty text falls back to the map key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shapes)
	TMap<FName, FWireStringData> WireStrings;
#endif

#if WITH_EDITOR
private:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunc) override;
#endif
};
