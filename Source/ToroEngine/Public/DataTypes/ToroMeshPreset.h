// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once


#include "Math/Vector.h"
#include "Math/Transform.h"
#include "Containers/Array.h"
#include "Templates/TypeHash.h"
#include "UObject/ObjectMacros.h"
#include "UObject/SoftObjectPtr.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroMeshPreset.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UStaticMeshComponent;

/**
 * Reusable mesh, partial material, lighting, and optional world-transform settings.
 * Applying the preset loads soft references synchronously. Non-null mesh paths do
 * not guarantee that the referenced asset exists or can be loaded.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FToroStaticMeshPreset
{
	GENERATED_BODY()

	/** Mesh to load and assign; a null or failed load resolves to a null mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Preset)
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	/**
	 * Partial per-slot material overrides, applied only within the target material count.
	 * Null, unresolved, and unspecified entries leave existing overrides unchanged.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Preset)
	TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

	/** Overlay material to load and assign; a null or failed load clears the overlay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Preset)
	TSoftObjectPtr<UMaterialInterface> OverlayMaterial;

	/** Whether the target mesh component casts shadows. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Preset)
	bool bCastShadows;

	/** Whether the target affects dynamic indirect lighting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Preset)
	bool bAffectIndirectLighting;

	/** Whether application sets the world transform and, for spline presets, local spline geometry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Preset)
	bool bUseTransform;

	/** World-space component transform applied only when bUseTransform is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Preset)
	FTransform Transform;

	/** Constructs an empty preset with disabled lighting flags and no transform application. */
	FToroStaticMeshPreset()
		: StaticMesh(nullptr)
		, Materials({})
		, OverlayMaterial(nullptr)
		, bCastShadows(false)
		, bAffectIndirectLighting(false)
		, bUseTransform(false)
		, Transform(FTransform::Identity)
	{}

	virtual ~FToroStaticMeshPreset() = default;

	bool operator==(const FToroStaticMeshPreset& Other) const { return Equals(Other, false); }
	bool operator!=(const FToroStaticMeshPreset& Other) const { return !Equals(Other, false); }

	[[nodiscard]] FORCEINLINE friend uint32 GetTypeHash(const FToroStaticMeshPreset& Preset)
	{
		uint32 Hash = GetTypeHash(Preset.StaticMesh);
		Hash = HashCombine(Hash, GetTypeHash(Preset.Materials));
		Hash = HashCombine(Hash, GetTypeHash(Preset.OverlayMaterial));
		Hash = HashCombine(Hash, GetTypeHash(Preset.bCastShadows));
		return Hash;
	}

	/** Returns whether a mesh reference is assigned, without loading or validating its asset. */
	[[nodiscard]] FORCEINLINE bool IsValid() const
	{
		return !StaticMesh.IsNull();
	}

	/**
	 * Compares mesh and material references and lighting flags without loading assets.
	 * @param Other Preset to compare against.
	 * @param bCheckTransform Also compares bUseTransform and the stored world transform,
	 *         using FTransform::Equals even when transform application is disabled.
	 */
	[[nodiscard]] virtual bool Equals(const FToroStaticMeshPreset& Other, bool bCheckTransform) const;

	/**
	 * Synchronously loads the mesh and fills material slots from its default materials.
	 * On a successful mesh load, resizes Materials to match its slot count and fills entries
	 * that fail to resolve. Existing entries that resolve are retained unless overwritten.
	 * @param bOverwrite Clears Materials before loading the mesh; a failed mesh load then
	 *         leaves it empty. Without overwriting, a failed mesh load preserves Materials.
	 */
	virtual void FillMaterials(bool bOverwrite);

	/**
	 * Captures mesh, resolved materials, overlay, and lighting settings from a component.
	 * @param Target Component to read.
	 * @param bIncludeTransform Captures the world transform and enables its application.
	 *         Otherwise stores identity and disables transform application.
	 * @return False for an invalid target, leaving the preset unchanged; true otherwise.
	 */
	virtual bool FromMeshComponent(const UStaticMeshComponent* Target, bool bIncludeTransform = false);

	/**
	 * Synchronously loads and applies mesh, partial materials, overlay, and lighting settings.
	 * Only resolved material entries within the target slot count are assigned; remaining
	 * overrides are preserved. Applies the world transform only when bUseTransform is true.
	 * @param Target Component to modify.
	 * @return False for an invalid target or a rejected mesh change; true otherwise.
	 *         Asset-load failures alone do not cause failure: the mesh or overlay may clear.
	 */
	virtual bool ToMeshComponent(UStaticMeshComponent* Target) const;
};

/**
 * Extends the static mesh preset with a forward axis and optional local spline geometry.
 * Capture and application require a spline mesh component. The transform opt-in also
 * controls spline geometry: capture without it stores zero vectors, and application
 * without it leaves component geometry unchanged. Application refreshes dirty spline
 * rendering and collision after the base preset succeeds.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FToroSplineMeshPreset : public FToroStaticMeshPreset
{
	GENERATED_BODY()

	/** Mesh axis used as the spline forward direction; always captured and applied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Preset)
	TEnumAsByte<ESplineMeshAxis::Type> ForwardAxis;

	/** Spline start position in component-local space, applied when bUseTransform is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Spline")
	FVector StartPos;

	/** Spline end position in component-local space, applied when bUseTransform is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Spline")
	FVector EndPos;

	/** Spline start tangent in component-local space, applied when bUseTransform is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Spline")
	FVector StartTangent;

	/** Spline end tangent in component-local space, applied when bUseTransform is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Spline")
	FVector EndTangent;

	/** Constructs a spline preset with the X forward axis and zeroed local geometry. */
	FToroSplineMeshPreset()
		: ForwardAxis(0)
		, StartPos(FVector::ZeroVector)
		, EndPos(FVector::ZeroVector)
		, StartTangent(FVector::ZeroVector)
		, EndTangent(FVector::ZeroVector)
	{}

	FORCEINLINE explicit operator FToroStaticMeshPreset&() { return *this; }
	FORCEINLINE explicit operator const FToroStaticMeshPreset&() const { return *this; }

	bool operator==(const FToroSplineMeshPreset& Other) const { return Equals(Other, false); }
	bool operator!=(const FToroSplineMeshPreset& Other) const { return !Equals(Other, false); }

	[[nodiscard]] FORCEINLINE friend uint32 GetTypeHash(const FToroSplineMeshPreset& Preset)
	{
		return HashCombine(
			GetTypeHash(static_cast<const FToroStaticMeshPreset&>(Preset)),
			GetTypeHash(Preset.ForwardAxis)
		);
	}

	/**
	 * Compares base preset settings and the spline forward axis.
	 * @param Other Spline preset to compare against.
	 * @param bCheckTransform Also compares the base transform settings and all four spline
	 *         vectors using their default equality tolerances, even when bUseTransform is false.
	 */
	[[nodiscard]] virtual bool Equals(const FToroSplineMeshPreset& Other, bool bCheckTransform) const;
	virtual bool FromMeshComponent(const UStaticMeshComponent* Target, bool bIncludeTransform = false) override;
	virtual bool ToMeshComponent(UStaticMeshComponent* Target) const override;

private:

	// Prevent external code from calling this base type overload by overriding as a private member
	virtual bool Equals(const FToroStaticMeshPreset& Other, bool bCheckTransform) const override
	{
		return Super::Equals(Other, bCheckTransform);
	}
};

/**
 * Blueprint queries, comparisons, capture, and application for static and spline mesh presets.
 * Material filling and application load soft asset references synchronously.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroMeshPresetLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Returns whether a mesh reference is assigned; does not load or validate the referenced asset. */
	UFUNCTION(BlueprintPure, Category = "MeshPreset|Static")
	static bool IsStaticMeshPresetValid(const FToroStaticMeshPreset& Preset);

	/**
	 * Compares mesh and material references and lighting flags without loading assets.
	 * @param bCheckTransform Also compares transform settings, even when application is disabled.
	 */
	UFUNCTION(BlueprintPure, Category = "MeshPreset|Static", meta = (CompactNodeTitle = "==", Keywords = "== equal"))
	static bool EqualStaticMeshPreset(const FToroStaticMeshPreset& A, const FToroStaticMeshPreset& B, bool bCheckTransform = false);

	/**
	 * Returns the inverse of EqualStaticMeshPreset() with the same comparison options.
	 * @param bCheckTransform Includes transform settings in the comparison.
	 */
	UFUNCTION(BlueprintPure, Category = "MeshPreset|Static", meta = (CompactNodeTitle = "!=", Keywords = "!= not equal"))
	static bool NotEqualStaticMeshPreset(const FToroStaticMeshPreset& A, const FToroStaticMeshPreset& B, bool bCheckTransform = false);

	/**
	 * Updates the supplied preset's material overrides from its mesh defaults, loading assets synchronously.
	 * Successful mesh loads resize the array and fill unresolved slots; resolved overrides are retained.
	 * @param bOverwrite Clears overrides first; a failed mesh load then leaves them empty.
	 *        Without overwriting, a failed mesh load preserves the array.
	 */
	UFUNCTION(BlueprintCallable, Category = "MeshPreset|Static")
	static void FillStaticMeshPresetMaterials(UPARAM(ref) FToroStaticMeshPreset& Preset, bool bOverwrite = false);

	/**
	 * Updates the supplied preset from a static mesh component.
	 * @param bIncludeTransform Captures the world transform and enables application.
	 *        When false, stores identity and disables transform application.
	 * @return False for an invalid target, leaving the preset unchanged; true otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "MeshPreset|Static")
	static bool CaptureStaticMeshPreset(UPARAM(ref) FToroStaticMeshPreset& Preset, UStaticMeshComponent* Target, bool bIncludeTransform = false);

	/**
	 * Synchronously loads and applies the preset to a static mesh component.
	 * Unresolved or unspecified material slots preserve existing overrides.
	 * Failed mesh or overlay loads may clear the target mesh or overlay.
	 * Transform application follows bUseTransform.
	 * @return False for an invalid target or rejected mesh change; asset-load failure alone does not imply failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "MeshPreset|Static")
	static bool ApplyStaticMeshPreset(const FToroStaticMeshPreset& Preset, UStaticMeshComponent* Target);

	/** Returns whether a mesh reference is assigned; does not load or validate the referenced asset. */
	UFUNCTION(BlueprintPure, Category = "MeshPreset|Spline")
	static bool IsSplineMeshPresetValid(const FToroSplineMeshPreset& Preset);

	/**
	 * Compares mesh and material references, lighting flags, and the spline forward axis without loading assets.
	 * @param bCheckTransform Also compares transform settings and local spline geometry, even when application is disabled.
	 */
	UFUNCTION(BlueprintPure, Category = "MeshPreset|Spline", meta = (CompactNodeTitle = "==", Keywords = "== equal"))
	static bool EqualSplineMeshPreset(const FToroSplineMeshPreset& A, const FToroSplineMeshPreset& B, bool bCheckTransform = false);

	/**
	 * Returns the inverse of EqualSplineMeshPreset() with the same comparison options.
	 * @param bCheckTransform Includes transform settings and local spline geometry in the comparison.
	 */
	UFUNCTION(BlueprintPure, Category = "MeshPreset|Spline", meta = (CompactNodeTitle = "!=", Keywords = "!= not equal"))
	static bool NotEqualSplineMeshPreset(const FToroSplineMeshPreset& A, const FToroSplineMeshPreset& B, bool bCheckTransform = false);

	/**
	 * Updates the supplied preset's material overrides from its mesh defaults, loading assets synchronously.
	 * Successful mesh loads resize the array and fill unresolved slots; resolved overrides are retained.
	 * @param bOverwrite Clears overrides first; a failed mesh load then leaves them empty.
	 *        Without overwriting, a failed mesh load preserves the array.
	 */
	UFUNCTION(BlueprintCallable, Category = "MeshPreset|Spline")
	static void FillSplineMeshPresetMaterials(UPARAM(ref) FToroSplineMeshPreset& Preset, bool bOverwrite = false);

	/**
	 * Updates the supplied preset from a spline mesh component.
	 * @param bIncludeTransform Captures the world transform and local spline geometry and enables application.
	 *        When false, stores identity and zeroed local geometry and disables transform application.
	 * @return False for an invalid target, leaving the preset unchanged; true otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "MeshPreset|Spline")
	static bool CaptureSplineMeshPreset(UPARAM(ref) FToroSplineMeshPreset& Preset, USplineMeshComponent* Target, bool bIncludeTransform = false);

	/**
	 * Synchronously loads and applies the preset to a spline mesh component.
	 * Unresolved or unspecified material slots preserve existing overrides.
	 * Failed mesh or overlay loads may clear the target mesh or overlay.
	 * Transform and spline geometry application follows bUseTransform. Refreshes spline rendering and collision after success.
	 * @return False for an invalid target or rejected mesh change; asset-load failure alone does not imply failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "MeshPreset|Spline")
	static bool ApplySplineMeshPreset(const FToroSplineMeshPreset& Preset, USplineMeshComponent* Target);
};
