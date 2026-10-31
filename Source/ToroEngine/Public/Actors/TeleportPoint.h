// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Engine/TargetPoint.h"
#include "TeleportPoint.generated.h"

class ACharacter;
class UStaticMeshComponent;

/**
 * Tagged destination for character teleportation, with an optional pawn-blocking support plane.
 * Provides an editor capsule preview and floor snapping; editor placement allows yaw rotation only and no scaling.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType, PrioritizeCategories = (Tools), HideCategories = (
	Display, Tick, Input, Collision, Physics, Rendering, RayTracing, Replication, Networking,
	AI, Game, Cooking, HLOD, DataLayers, WorldPartition, Level, LevelInstance))
class ATeleportPoint final : public ATargetPoint
{
	GENERATED_BODY()

public:

	ATeleportPoint();

	/**
	 * Finds the first valid teleport point with the given actor tag in the context's world.
	 * @param SearchTag Actor tag to match; NAME_None does not match any point.
	 * @return The first matching point, or nullptr if the world is invalid or no match exists.
	 */
	UFUNCTION(BlueprintCallable, Category = TeleportPoint,
		DisplayName = "Find Teleport Point", meta = (WorldContext = ContextObject))
	static TOROENGINE_API ATeleportPoint* Get(const UObject* ContextObject, FName SearchTag);

	/**
	 * Teleports a character to this point's location and rotation without checking destination collision.
	 * @param Target Character to move; invalid targets are ignored.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = TeleportPoint)
	TOROENGINE_API void Teleport(ACharacter* Target) const;

	/**
	 * Teleports the indexed player character using Teleport(); does nothing if no character is available.
	 * @param PlayerIdx Player index used to look up the character in this world.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = TeleportPoint)
	TOROENGINE_API void TeleportPlayer(const int32 PlayerIdx = 0) const;

private:

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> SupportPlane;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<class UCapsuleComponent> PreviewCapsule;

	/** Requested preview capsule half-height in centimeters, clamped by the capsule to at least its radius. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 10.0f, UIMin = 10.0f, Units = "cm"))
	float CapsuleHeight = 88.0f;

	/** Downward distance in centimeters for each floor trace; zero disables snapping. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.0f, UIMin = 0.0f, Units = "cm"))
	float FloorSearchRange = 250.0f;

	/** Last preview center recorded during construction, used to detect movement requiring a floor snap. */
	UPROPERTY(VisibleAnywhere, Category = Settings, AdvancedDisplay)
	FVector LastPosition;
#endif

	/** Enables the support plane's editor visibility and actor collision at BeginPlay. */
	UPROPERTY(EditAnywhere, Category = Settings)
	bool bUseSupportPlane;

	virtual void BeginPlay() override;

#if WITH_EDITOR
	/**
	 * Moves vertically to place the preview capsule bottom 2 cm above the first visibility-channel hit.
	 * Traces downward from the capsule center, top, then bottom, ignoring this actor.
	 * Leaves placement unchanged if no hit is found or the preview, world, or search range is unavailable.
	 */
	UFUNCTION(CallInEditor, Category = Tools)
	void SnapToFloor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void EditorApplyRotation(const FRotator& DeltaRotation,
		bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	virtual void EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation,
		bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
#endif
};
