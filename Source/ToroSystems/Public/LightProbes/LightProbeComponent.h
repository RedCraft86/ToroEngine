// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Components/SceneComponent.h"
#include "LightProbeComponent.generated.h"

UCLASS(MinimalAPI, NotBlueprintable, BlueprintType, ClassGroup = (Lights, Common), HideCategories = (Object), meta = (BlueprintSpawnableComponent))
class ULightProbeComponent final : public USceneComponent
{
	GENERATED_BODY()

public:

	ULightProbeComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, meta = (ClampMin = 1.0f, UIMin = 10.0f, UIMax = 100.0f))
	float Intensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, meta = (ClampMin = 50.0f, UIMax = 1000.0f))
	float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, meta = (ClampMin = 0.0f, UIMax = 8.0f))
	float Falloff;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, meta = (HideAlphaChannel))
	FLinearColor Color;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, AdvancedDisplay)
	bool bDisableWithLumenGI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, AdvancedDisplay, meta = (ClampMin = 1000.0f, ClampMax = 10000.0f))
	float MaxDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Light, AdvancedDisplay, meta = (ClampMin = 0.0f, ClampMax = 1000.0f))
	float FadeRange;

	[[nodiscard]] bool IsProbeRelevant(const FTransform& CamTransform, bool bHasLumenGI) const;
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
