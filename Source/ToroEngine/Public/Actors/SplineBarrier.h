// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/Actor.h"
#include "SplineBarrier.generated.h"

class UBoxComponent;
class USplineComponent;
class IConsoleVariable;

/**
 * Authors invisible collision barriers along an open or closed spline, with points forced linear.
 * Point Y/Z scale controls thickness/height relative to 10/200 cm, using the larger endpoint scale
 * per section and a 1 cm minimum. Sections tilt along slopes; the spline follows their bottom center.
 * Toggle r.Debug.ShowInvisibleWalls to show diagnostic outlines in builds that support debug rendering.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType, PrioritizeCategories = (Settings), HideCategories = (
	Display, Tick, Input, Collision, Physics, Rendering, RayTracing, Replication, Networking,
	AI, Game, Cooking, HLOD, DataLayers, WorldPartition, Level, LevelInstance))
class ASplineBarrier final : public AActor
{
	GENERATED_BODY()

public:

	ASplineBarrier();

protected:

	UPROPERTY(VisibleDefaultsOnly, Category = DefaultSubobjects)
	TObjectPtr<USplineComponent> RootSpline;

	/** Zero-based segment indices to leave open; index i connects point i to the next point, wrapping on closed splines. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSet<int32> SkipIndices;

	UPROPERTY()
	TArray<TObjectPtr<UBoxComponent>> WallSections;

	void OnVisualCVarChanged(IConsoleVariable* Variable);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;
};
