// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "LightProbes/LightProbeSystemConfig.h"
#include "HAL/IConsoleManager.h"
#if WITH_EDITOR
#include "Misc/App.h"
#endif

static TAutoConsoleVariable<bool> CVarLightProbesEnabled(
	TEXT("r.LightProbes.Enable"),
	true,
	TEXT("Whether the light probe system is enabled.")
);

static TAutoConsoleVariable<int32> CVarLightProbesMaxLimit(
	TEXT("r.LightProbes.MaxLimit"),
	24,
	TEXT("Maximum number of light probes that can be processed at once by the light probe system."),
	ECVF_Scalability
);

static TAutoConsoleVariable<float> CVarLightProbesUpdateInterval(
	TEXT("r.LightProbes.UpdateInterval"),
	0.1f,
	TEXT("Interval (in seconds) between each update by the light probe system."),
	ECVF_Scalability
);

#if WITH_EDITOR
static TAutoConsoleVariable<float> CVarLightProbesEditorUpdateInterval(
	TEXT("r.LightProbes.EditorUpdateInterval"),
	0.05f,
	TEXT("Interval (in seconds) between each update by the light probe system in the editor. ")
	TEXT("-1 will use the same value as r.LightProbes.UpdateInterval"),
	ECVF_Scalability
);
#endif

bool FLightProbeSystemConfig::IsEnabled() const
{
	return bEnableForWorld
		&& CVarLightProbesEnabled.GetValueOnGameThread()
		&& GetMaxProbeLimit() > 0
		&& IsValid();
}

int32 FLightProbeSystemConfig::GetMaxProbeLimit()
{
	return CVarLightProbesMaxLimit.GetValueOnGameThread();
}

float FLightProbeSystemConfig::GetUpdateInterval()
{
	float Interval = CVarLightProbesUpdateInterval.GetValueOnGameThread();
#if WITH_EDITOR
	if (!FApp::IsGame())
	{
		const float EditorInterval = CVarLightProbesEditorUpdateInterval.GetValueOnGameThread();
		if (EditorInterval < 0.0f)
		{
			Interval = EditorInterval;
		}
	}
#endif
	return FMath::Max(0.0f, Interval);
}
