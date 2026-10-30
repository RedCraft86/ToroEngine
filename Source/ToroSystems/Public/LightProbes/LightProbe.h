// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/Actor.h"
#include "LightProbe.generated.h"

class ULightProbeComponent;

/**
 * Placeable actor that provides a light probe contribution through its root light probe component.
 * Editor scaling adjusts the probe's influence radius instead of the actor's scale.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType, ComponentWrapperClass, ClassGroup = (Lights, PointLights))
class ALightProbe : public AActor
{
	GENERATED_BODY()

public:

	ALightProbe();

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Light, meta = (AllowPrivateAccess = true))
	TObjectPtr<ULightProbeComponent> LightProbeComponent;

#if WITH_EDITOR
	virtual void EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation,
		bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
#endif
};
