// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroWorldSettings.h"
#include "PostProcess/WorldPostProcessComponent.h"

AToroWorldSettings::AToroWorldSettings()
{
	PostProcess = CreateDefaultSubobject<UWorldPostProcessComponent>(TEXT("PostProcess"));

	// Light Probes Config
	{
		const ConstructorHelpers::FObjectFinder<UMaterialInterface> PPM(
			TEXT("/ToroEngine/LightProbes/PPM_LightProbes.PPM_LightProbes"));
		if (PPM.Succeeded()) LightProbes.PostProcessMaterial = PPM.Object;

		const ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> MPC(
			TEXT("/ToroEngine/LightProbes/MPC_LightProbes.MPC_LightProbes"));
		if (MPC.Succeeded()) LightProbes.ParameterCollection = MPC.Object;
	}
}
