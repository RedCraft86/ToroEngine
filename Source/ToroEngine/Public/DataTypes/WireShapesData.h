// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "WireShapesData.generated.h"

class AActor;

/**
 * Shared color and depth-priority settings for editor wire shapes.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWireShapeDataBase
{
	GENERATED_BODY()

	/** Drawing color; navigation routes use this for lines and labels. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (HideAlphaChannel = true))
	FLinearColor Color;

	/** Draw in the foreground depth-priority group rather than the world group. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	bool bRenderOnTop;

	FWireShapeDataBase()
		: Color(FColor::Cyan)
		, bRenderOnTop(false)
	{}
};

/**
 * A point marker at an owner-local position, transformed without owner scale.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWirePointData final : public FWireShapeDataBase
{
	GENERATED_BODY()

	/** Position relative to the owner; owner scale is ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FVector Location;

	/** Point-marker size passed to the drawing interface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float Size;

	FWirePointData()
		: Location(FVector::ZeroVector)
		, Size(2.0f)
	{}
};

/**
 * A line segment with owner-local endpoints, transformed without owner scale.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWireLineData final : public FWireShapeDataBase
{
	GENERATED_BODY()

	/** Segment start relative to the owner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FVector Start;

	/** Segment end relative to the owner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FVector End;

	/** Line thickness passed to the drawing interface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float Thickness;

	FWireLineData()
		: Start(FVector::ZeroVector)
		, End(FVector::ZeroVector)
		, Thickness(1.0f)
	{}
};

/**
 * A wire circle in its local XY plane, positioned and rotated relative to the owner without owner scale.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWireCircleData final : public FWireShapeDataBase
{
	GENERATED_BODY()

	/** Position relative to the owner; owner scale is ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FVector Location;

	/** Orientation relative to the owner's rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FRotator Rotation;

	/** Radius in world units, unaffected by owner scale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float Radius;

	/** Number of straight segments approximating each circle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 3, UIMin = 3))
	uint8 NumOfSides;

	/** Line thickness passed to the drawing interface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float Thickness;

	FWireCircleData()
		: Location(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
		, Radius(32.0f)
		, NumOfSides(16)
		, Thickness(1.0f)
	{}
};

/**
 * An outlined circular sector in its local XY plane, transformed relative to the owner without owner scale.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWireArcData final : public FWireShapeDataBase
{
	GENERATED_BODY()

	/** Position relative to the owner; owner scale is ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FVector Location;

	/** Orientation relative to the owner's rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FRotator Rotation;

	/** Starting angle in degrees, measured from local +X toward local +Y. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = -360.0f, UIMin = -360.0f, ClampMax = 360.0f, UIMax = 360.0f))
	float MinAngle;

	/** Ending angle in degrees; values below MinAngle reverse the sweep. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = -360.0f, UIMin = -360.0f, ClampMax = 360.0f, UIMax = 360.0f))
	float MaxAngle;

	/** Radius in world units, unaffected by owner scale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1, UIMin = 1))
	float Radius;

	/** Number of straight segments approximating the sector arc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1, UIMin = 1))
	uint8 Sections;

	/** Line thickness passed to the drawing interface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float Thickness;

	FWireArcData()
		: Location(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
		, MinAngle(0.0f)
		, MaxAngle(90.0f)
		, Radius(32.0f)
		, Sections(8)
		, Thickness(1.0f)
	{}
};

/**
 * An oriented wire box defined by half-extents, transformed relative to the owner without owner scale.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWireBoxData final : public FWireShapeDataBase
{
	GENERATED_BODY()

	/** Position relative to the owner; owner scale is ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FVector Location;

	/** Orientation relative to the owner's rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FRotator Rotation;

	/** Half-size along each rotated box axis, in world units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	FVector Extents;

	/** Line thickness passed to the drawing interface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float Thickness;

	FWireBoxData()
		: Location(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
		, Extents(FVector(32.0f))
		, Thickness(1.0f)
	{}
};

/**
 * A wire sphere at an owner-local center, transformed without owner scale.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWireSphereData final : public FWireShapeDataBase
{
	GENERATED_BODY()

	/** Position relative to the owner; owner scale is ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FVector Location;

	/** Radius in world units, unaffected by owner scale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float Radius;

	/** Number of straight segments approximating each circle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 3, UIMin = 3))
	uint8 NumOfSides;

	/** Line thickness passed to the drawing interface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float Thickness;

	FWireSphereData()
		: Location(FVector::ZeroVector)
		, Radius(32.0f)
		, NumOfSides(16)
		, Thickness(1.0f)
	{}
};

/**
 * A wire cylinder along its local Z axis, transformed relative to the owner without owner scale.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWireCylinderData final : public FWireShapeDataBase
{
	GENERATED_BODY()

	/** Position relative to the owner; owner scale is ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FVector Location;

	/** Orientation relative to the owner's rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FRotator Rotation;

	/** Radius in world units, unaffected by owner scale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float Radius;

	/** Distance from the center to either end along local Z, in world units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float HalfHeight;

	/** Number of straight segments approximating each circle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 3, UIMin = 3))
	uint8 NumOfSides;

	/** Line thickness passed to the drawing interface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float Thickness;

	FWireCylinderData()
		: Location(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
		, Radius(32.0f)
		, HalfHeight(64.0f)
		, NumOfSides(12)
		, Thickness(1.0f)
	{}
};

/**
 * A wire capsule along its local Z axis, transformed relative to the owner without owner scale.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWireCapsuleData final : public FWireShapeDataBase
{
	GENERATED_BODY()

	/** Position relative to the owner; owner scale is ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FVector Location;

	/** Orientation relative to the owner's rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape)
	FRotator Rotation;

	/** Requested radius in world units; drawing limits it to HalfHeight. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float Radius;

	/** Half the total capsule height including caps; drawing clamps the radius to this value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float HalfHeight;

	/** Number of straight segments approximating each circle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 4, UIMin = 4))
	uint8 NumOfSides;

	/** Line thickness passed to the drawing interface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireShape, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float Thickness;

	FWireCapsuleData()
		: Location(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
		, Radius(32.0f)
		, HalfHeight(64.0f)
		, NumOfSides(16)
		, Thickness(1.0f)
	{}
};

/**
 * Navigation debug route with owner-local targets and cached world-space vertices.
 * Owner scale is ignored during generation. The editor visualizer selects labels per viewport independently of LabelPoint.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWireNavPathData final : public FWireShapeDataBase
{
	GENERATED_BODY()

	/** Ordered owner-local navigation targets, transformed without owner scale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WirePath)
	TArray<FVector> Targets;

	/** Retain the reachable prefix, including a valid partial segment, instead of clearing the route on failure. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WirePath)
	bool bAllowPartialPath = false;

	/** Thickness of the lines connecting cached navigation vertices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WirePath, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float LineThickness;

	/** Size of the markers drawn at cached navigation vertices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WirePath, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float PointSize;

	/** Color of the markers drawn at cached navigation vertices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WirePath, meta = (HideAlphaChannel = true))
	FLinearColor PointColor;

	/** Cached world-space vertex selected by FindNearestToCamera; reset during path generation or for an empty path. */
	UPROPERTY(VisibleAnywhere, Category = WirePath, meta = (DisplayPriority = 999))
	TOptional<FVector> LabelPoint;

	/** Generated world-space route vertices; adjacent duplicates are removed. */
	UPROPERTY(VisibleAnywhere, Category = WirePath, meta = (DisplayPriority = 999))
	TArray<FVector> PathPoints;

	FWireNavPathData()
		: LineThickness(4.0f)
		, PointSize(16.0f)
		, PointColor(FColor::Green)
	{}

	/**
	 * Selects a cached route vertex using equally weighted camera-facing and proximity scores.
	 * Proximity falls to zero at 1,000 units; ties retain the first vertex. Empty paths clear LabelPoint.
	 * Uses global world resolution and the camera library's default player or active editor viewport.
	 * If no camera resolves, the library supplies the identity transform; selection may therefore use the origin and +X.
	 * Call on the game thread. The editor visualizer selects its own label independently for each viewport.
	 */
	void FindNearestToCamera();

	/**
	 * Synchronously rebuilds the world-space route between consecutive targets on the game thread.
	 * Clears the previous route and label first; invalid owners, missing worlds, or fewer than two targets leave it empty.
	 * With bAllowPartialPath disabled, any failed or partial segment clears the entire route.
	 * When enabled, retains the reachable prefix, appends a valid partial segment if available, and stops at the first failure or partial result.
	 * @param Owner Supplies the navigation world and transform for local targets; scale is ignored.
	 */
	void GeneratePathPoints(AActor* Owner);
};

/**
 * A canvas label anchored at an owner-local position, transformed without owner scale.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FWireStringData final
{
	GENERATED_BODY()

	/** Label text; an empty string makes the component visualizer use the containing map key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireString)
	FString Text;

	/** Position relative to the owner; owner scale is ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireString)
	FVector Location;

	/** Text color, including opacity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireString)
	FLinearColor Color;

	/** Font size supplied to the text helper; zero uses its default size of 12. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireString, meta = (ClampMin = 1, UIMin = 1))
	uint8 FontScale;

	/** Draw a black shadow behind the label. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WireString)
	bool bDrawShadow;

	FWireStringData()
		: Text(TEXT("Wire_String"))
		, Location(FVector::ZeroVector)
		, Color(FColor::Green)
		, FontScale(12)
		, bDrawShadow(true)
	{}
};
