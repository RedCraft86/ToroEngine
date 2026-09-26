// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroMeshPreset.generated.h"

class USplineMeshComponent;

namespace ESplineMeshAxis
{
	enum Type : int;
}

/**
 * Reusable static mesh, material, overlay, shadow, and optional world-transform settings.
 * Assets use soft references and are loaded synchronously when needed.
 * C++ equality and hashing ignore transform state; Equals can optionally compare it.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FToroBaseMeshPreset
{
	GENERATED_BODY()

	/** Mesh asset to load synchronously when validating, filling materials, or applying the preset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MeshPreset)
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	/**
	 * Material overrides indexed by mesh slot.
	 * On application, omitted, null, or unloadable entries use mesh defaults;
	 * entries beyond the applied mesh slot count are ignored.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MeshPreset)
	TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

	/** Overlay material loaded on application; an empty reference clears the component overlay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MeshPreset)
	TSoftObjectPtr<UMaterialInterface> OverlayMaterial;

	/** Whether the target component casts shadows when this preset is applied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MeshPreset)
	bool bCastShadows;

	/** Whether application also sets the target component world transform. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MeshPreset, meta = (InlineEditConditionToggle))
	bool bUseTransform;

	/** World transform applied when bUseTransform is enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MeshPreset, meta = (EditCondition = bUseTransform, AllowPreserveRatio = true))
	FTransform Transform;

	/** Creates an empty preset with shadows enabled and world-transform application disabled. */
	FToroBaseMeshPreset()
		: StaticMesh(nullptr)
		, Materials({})
		, bCastShadows(true)
		, bUseTransform(false)
		, Transform(FTransform::Identity)
	{}

	virtual ~FToroBaseMeshPreset() = default;

	bool operator==(const FToroBaseMeshPreset& Other) const { return Equals(Other, false); }
	bool operator!=(const FToroBaseMeshPreset& Other) const { return !Equals(Other, false); }

	[[nodiscard]] FORCEINLINE friend uint32 GetTypeHash(const FToroBaseMeshPreset& Preset)
	{
		uint32 Hash = GetTypeHash(Preset.StaticMesh);
		Hash = HashCombine(Hash, GetTypeHash(Preset.Materials));
		Hash = HashCombine(Hash, GetTypeHash(Preset.OverlayMaterial));
		Hash = HashCombine(Hash, GetTypeHash(Preset.bCastShadows));
		return Hash;
	}

	/** Synchronously loads the mesh and checks its validity; other preset fields are not validated. */
	[[nodiscard]] FORCEINLINE bool IsValid() const
	{
		return ::IsValid(StaticMesh.LoadSynchronous());
	}

	/**
	 * Synchronously loads the mesh and fills materials from its default slots.
	 * @param bOverwrite Clear existing entries first, leaving them empty if mesh loading fails.
	 * When false, retain valid entries, fill missing or invalid entries, and preserve extra slots.
	 * Checking existing entries may synchronously load their material assets.
	 */
	virtual void FillMaterials(const bool bOverwrite);

	/**
	 * Compares mesh and ordered material references, overlay, and shadow state.
	 * @param bCheckTransform Also compare bUseTransform and the stored transform using
	 * FTransform::Equals, even when transform application is disabled.
	 */
	[[nodiscard]] virtual bool Equals(const FToroBaseMeshPreset& Other, const bool bCheckTransform) const;

	/**
	 * Captures mesh, effective slot materials, overlay, and shadows from a valid component.
	 * Replaces the material array and fills missing entries from mesh defaults.
	 * Invalid targets leave the preset unchanged.
	 * @param bIncludeTransform Capture the world transform and enable its application;
	 * otherwise disable transform application and store identity.
	 */
	virtual void FromMeshComponent(const UStaticMeshComponent* Target, const bool bIncludeTransform = false);

	/**
	 * Synchronously loads and applies assets, shadow settings, and the optional world transform.
	 * Replaces material overrides; omitted or invalid slot materials use mesh defaults.
	 * Extra material entries are ignored. An empty or unloadable mesh clears the target mesh.
	 * Invalid targets are left unchanged.
	 */
	virtual void ToMeshComponent(UStaticMeshComponent* Target) const;
};

/**
 * Extends the base mesh preset with a spline forward axis.
 * Capture and application require a valid spline mesh component.
 * Spline shape, tangents, and other deformation settings are not captured.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FToroSplineMeshPreset : public FToroBaseMeshPreset
{
	GENERATED_BODY()

	/** Mesh axis aligned along the spline when the preset is applied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MeshPreset)
	TEnumAsByte<ESplineMeshAxis::Type> ForwardAxis;

	/** Creates a default base preset with X as the spline forward axis. */
	FToroSplineMeshPreset()
		: ForwardAxis(0)
	{}

	FORCEINLINE explicit operator FToroBaseMeshPreset&() { return *this; }
	FORCEINLINE explicit operator const FToroBaseMeshPreset&() const { return *this; }

	bool operator==(const FToroSplineMeshPreset& Other) const { return Equals(Other, false); }
	bool operator!=(const FToroSplineMeshPreset& Other) const { return !Equals(Other, false); }

	[[nodiscard]] FORCEINLINE friend uint32 GetTypeHash(const FToroSplineMeshPreset& Preset)
	{
		return HashCombine(
			GetTypeHash(static_cast<const FToroBaseMeshPreset&>(Preset)), 
			GetTypeHash(Preset.ForwardAxis)
		);
	}

	/**
	 * Compares base preset settings and the spline forward axis.
	 * @param bCheckTransform Also compare the use-transform flag and stored transform
	 * through the base comparison, even when transform application is disabled.
	 */
	[[nodiscard]] virtual bool Equals(const FToroSplineMeshPreset& Other, const bool bCheckTransform) const;

	/**
	 * Captures base settings and the forward axis from a valid spline mesh component.
	 * Invalid or non-spline targets log a warning and leave this preset unchanged.
	 * @param bIncludeTransform Capture and enable the world transform; otherwise store identity
	 * and disable transform application. Spline shape and deformation settings are not captured.
	 */
	virtual void FromMeshComponent(const UStaticMeshComponent* Target, const bool bIncludeTransform = false) override;

	/**
	 * Applies base settings and the forward axis to a valid spline mesh component.
	 * Uses the base asset-loading, material-fallback, and optional world-transform behavior.
	 * Invalid or non-spline targets log a warning and are left unchanged.
	 */
	virtual void ToMeshComponent(UStaticMeshComponent* Target) const override;

private:

	// Prevent external code from calling this base type overload by overriding as a private member
	virtual bool Equals(const FToroBaseMeshPreset& Other, const bool bCheckTransform) const override
	{
		return Super::Equals(Other, bCheckTransform);
	}
};

/**
 * Blueprint helpers for comparing, capturing, and applying mesh presets.
 * Asset validation, material filling, and application may synchronously load assets.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroMeshPresetLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Synchronously loads the referenced static mesh and returns whether it is valid.
	 * Does not validate materials, transform state, or spline-axis settings.
	 */
	UFUNCTION(BlueprintPure, Category = "StaticMesh|MeshPreset", DisplayName = "Is Valid (Mesh Preset)")
	static bool IsMeshPresetValid(const FToroBaseMeshPreset& Target);

	/**
	 * Compares mesh and material references, material order, overlay, and shadow state.
	 * @param bCheckTransform Also compare the use-transform flag and stored transform
	 * using FTransform::Equals, even when transform application is disabled.
	 */
	UFUNCTION(BlueprintPure, Category = "StaticMesh|MeshPreset", DisplayName = "Equals (Mesh Preset)")
	static bool IsEqualsMeshPreset(const FToroBaseMeshPreset& A, const FToroBaseMeshPreset& B, const bool bCheckTransform = false);

	/**
	 * Synchronously loads the mesh and fills materials from its default slots.
	 * @param bOverwrite Clear existing entries first. If mesh loading fails, the cleared array stays empty.
	 * Otherwise retain valid entries, fill missing or unloadable entries, and preserve extra entries.
	 * Checking existing material entries may synchronously load them.
	 */
	UFUNCTION(BlueprintCallable, Category = "StaticMesh|MeshPreset", DisplayName = "Fill Materials (Mesh Preset)")
	static void FillMeshPresetMaterials(UPARAM(ref) FToroBaseMeshPreset& Target, const bool bOverwrite);

	/**
	 * Captures mesh, effective slot materials, overlay, and shadow state from the component.
	 * Replaces the stored material array, then fills missing materials from mesh defaults.
	 * @param bIncludeTransform Capture the world transform and enable its application;
	 * otherwise disable transform application and store identity.
	 * An invalid component leaves the preset unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "StaticMesh|MeshPreset", DisplayName = "Set From Component (Mesh Preset)")
	static void SetMeshPresetFromComponent(UPARAM(ref) FToroBaseMeshPreset& Target, const UStaticMeshComponent* Component, const bool bIncludeTransform = false);

	/**
	 * Synchronously loads and applies mesh, materials, overlay, and shadow settings.
	 * Clears existing material overrides; omitted, null, or unloadable slot materials use mesh defaults.
	 * Material entries beyond the applied mesh slot count are ignored.
	 * Applies the stored world transform only when enabled. An empty or unloadable mesh clears the mesh.
	 * An invalid component is left unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "StaticMesh|MeshPreset", DisplayName = "Apply To Component (Mesh Preset)")
	static void ApplyMeshPresetToComponent(const FToroBaseMeshPreset& Target, UStaticMeshComponent* Component);

	/** Returns a copy of the base preset fields, discarding the spline forward axis. */
	UFUNCTION(BlueprintPure, Category = "StaticMesh|MeshPreset", DisplayName = "Spline Mesh Preset to Base", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static FToroBaseMeshPreset SplineMeshPresetToBase(const FToroSplineMeshPreset& Target);

	/**
	 * Synchronously loads the referenced static mesh and returns whether it is valid.
	 * Does not validate materials, transform state, or spline-axis settings.
	 */
	UFUNCTION(BlueprintPure, Category = "StaticMesh|MeshPreset", DisplayName = "Is Valid (Spline Mesh Preset)")
	static bool IsSplineMeshPresetValid(const FToroSplineMeshPreset& Target);

	/**
	 * Compares mesh and material references, material order, overlay, and shadow state.
	 * Also compares the spline forward axis.
	 * @param bCheckTransform Also compare the use-transform flag and stored transform
	 * using FTransform::Equals, even when transform application is disabled.
	 */
	UFUNCTION(BlueprintPure, Category = "StaticMesh|MeshPreset", DisplayName = "Equals (Spline Mesh Preset)")
	static bool IsEqualsSplineMeshPreset(const FToroSplineMeshPreset& A, const FToroSplineMeshPreset& B, const bool bCheckTransform = false);

	/**
	 * Synchronously loads the mesh and fills materials from its default slots.
	 * @param bOverwrite Clear existing entries first. If mesh loading fails, the cleared array stays empty.
	 * Otherwise retain valid entries, fill missing or unloadable entries, and preserve extra entries.
	 * Checking existing material entries may synchronously load them.
	 */
	UFUNCTION(BlueprintCallable, Category = "StaticMesh|MeshPreset", DisplayName = "Fill Materials (Spline Mesh Preset)")
	static void FillSplineMeshPresetMaterials(UPARAM(ref) FToroSplineMeshPreset& Target, const bool bOverwrite);

	/**
	 * Captures mesh, effective slot materials, overlay, and shadow state from the component.
	 * Replaces the stored material array, then fills missing materials from mesh defaults.
	 * @param bIncludeTransform Capture the world transform and enable its application;
	 * otherwise disable transform application and store identity.
	 * Also captures the forward axis. Invalid or non-spline targets log a warning
	 * and leave the preset unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "StaticMesh|MeshPreset", DisplayName = "Set From Component (Spline Mesh Preset)")
	static void SetSplineMeshPresetFromComponent(UPARAM(ref) FToroSplineMeshPreset& Target, const USplineMeshComponent* Component, const bool bIncludeTransform = false);

	/**
	 * Synchronously loads and applies mesh, materials, overlay, and shadow settings.
	 * Clears existing material overrides; omitted, null, or unloadable slot materials use mesh defaults.
	 * Material entries beyond the applied mesh slot count are ignored.
	 * Applies the stored world transform only when enabled. An empty or unloadable mesh clears the mesh.
	 * Also applies the forward axis. Invalid or non-spline targets log a warning
	 * and are left unchanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "StaticMesh|MeshPreset", DisplayName = "Apply To Component (Spline Mesh Preset)")
	static void ApplySplineMeshPresetToComponent(const FToroSplineMeshPreset& Target, USplineMeshComponent* Component);
};
