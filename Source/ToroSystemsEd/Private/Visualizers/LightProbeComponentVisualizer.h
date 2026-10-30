// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "LightProbes/LightProbeComponent.h"
#include "Visualizers/ToroComponentVisualizer.h"

class FLightProbeComponentVisualizer final : public FToroComponentVisualizer
{
	virtual void DrawVisualization(const UActorComponent* Component,
		const FSceneView* View, FPrimitiveDrawInterface* PDI) override
	{
		if (const ULightProbeComponent* LightProbe = Cast<ULightProbeComponent>(Component))
		{
			DrawWireSphere(PDI,
				LightProbe->GetComponentLocation(),
				FLinearColor(0.5f, 1.0f, 1.0f),
				LightProbe->Radius,
				64,
				SDPG_World,
				2.0f
			);
		}
	}
};
