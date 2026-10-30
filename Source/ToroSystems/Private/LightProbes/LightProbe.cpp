// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "LightProbes/LightProbe.h"
#include "LightProbes/LightProbeComponent.h"

ALightProbe::ALightProbe()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	LightProbeComponent = CreateDefaultSubobject<ULightProbeComponent>(TEXT("LightProbe"));
	SetRootComponent(LightProbeComponent);
}

#if WITH_EDITOR
void ALightProbe::EditorApplyScale(const FVector& DeltaScale,
	const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	const FVector ModifiedScale = DeltaScale * (bUsePercentageBasedScaling ? 10000.0f : 100.0f);
	FMath::ApplyScaleToFloat(LightProbeComponent->Radius, ModifiedScale, 1.0f);
	PostEditChange();
}
#endif