// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Components/SceneComponent.h"
#include "LightProbeComponent.generated.h"

/**
 * Defines a local light probe contribution for the world's light probe subsystem.
 * Joins the subsystem on registration and provides camera-dependent relevance and material data.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType, ClassGroup = (Lights, Common), HideCategories = (Object), meta = (BlueprintSpawnableComponent))
class ULightProbeComponent final : public USceneComponent
{
	GENERATED_BODY()

public:

	ULightProbeComponent();

	/** Scales the probe's RGB contribution. Values below 0.1 make the probe irrelevant. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, meta = (ClampMin = 0.0f, UIMin = 10.0f, UIMax = 100.0f))
	float Intensity;

	/** Influence radius in world units. Values below 50 make the probe irrelevant. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, meta = (ClampMin = 50.0f, UIMax = 1000.0f))
	float Radius;

	/** Falloff value supplied to the light probe material. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, meta = (ClampMin = 0.0f, UIMax = 8.0f))
	float Falloff;

	/** Linear RGB tint of the probe contribution. Alpha is not used in the packed material data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, meta = (HideAlphaChannel))
	FLinearColor Color;

	/** Excludes this probe when the caller reports that Lumen global illumination is in use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, AdvancedDisplay)
	bool bDisableWithLumenGI;

	/** Maximum camera-to-probe distance in world units at which this probe remains relevant. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, AdvancedDisplay, meta = (ClampMin = 1000.0f, ClampMax = 10000.0f))
	float MaxDistance;

	/**
	 * Distance in world units over which intensity fades linearly to zero before MaxDistance.
	 * Positive values fade from MaxDistance - FadeRange to MaxDistance; nonpositive values disable fading.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, AdvancedDisplay, meta = (ClampMin = 0.0f, ClampMax = 1000.0f))
	float FadeRange;

	/**
	 * Whether this probe passes visibility, radius, intensity, Lumen, distance, and camera-facing checks.
	 * The probe may extend behind the camera plane by up to its radius.
	 * @param CamTransform Camera world transform used for distance and forward-direction checks.
	 * @param bHasLumenGI Whether Lumen global illumination is in use, for bDisableWithLumenGI filtering.
	 */
	[[nodiscard]] bool IsProbeRelevant(const FTransform& CamTransform, bool bHasLumenGI) const;

	/**
	 * Packs this probe's position and effect data for material parameters, applying camera-distance fading.
	 * Does not perform relevance filtering; callers should use IsProbeRelevant() when selecting probes.
	 * @param CamTransform Camera world transform whose location determines distance fading.
	 * @param[out] PosData Probe world position in XYZ and influence radius in W.
	 * @param[out] EffectData Linear RGB color scaled by intensity and distance fade, with falloff in W.
	 */
	void GetProbeData(const FTransform& CamTransform, FVector4& PosData, FVector4& EffectData) const;

protected:

	virtual void OnRegister() override;
	virtual void OnUnregister() override;

#if WITH_EDITOR
private:
	void UpdateLightIcon();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
