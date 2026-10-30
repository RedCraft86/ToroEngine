// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "PostProcess/LumenSceneViewExtension.h"
#include "HAL/IConsoleManager.h"
#include "RenderUtils.h"
#include "SceneInterface.h"
#include "SceneView.h"

FLumenSceneViewExtension::FLumenSceneViewExtension(const FAutoRegister& AutoRegister, UWorld* World)
	: FWorldSceneViewExtension(AutoRegister, World), bUsingLumen(false)
{
}

void FLumenSceneViewExtension::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
	bUsingLumen = false;
	if (!InViewFamily.Scene || !DoesPlatformSupportLumenGI(InViewFamily.Scene->GetShaderPlatform()))
	{
		return;
	}

	static const IConsoleVariable* LumenAllowed = IConsoleManager::Get()
		.FindConsoleVariable(TEXT("r.Lumen.DiffuseIndirect.Allow"));
	if (!LumenAllowed || !LumenAllowed->GetBool())
	{
		return;
	}

	for (const FSceneView* View : InViewFamily.Views)
	{
		if (!View
			|| View->bIsSceneCapture
			|| View->bIsSceneCaptureCube
			|| View->bIsReflectionCapture
			|| View->bIsPlanarReflection
			|| View->bIsVirtualTexture)
		{
			continue;
		}

		if (View->FinalPostProcessSettings.DynamicGlobalIlluminationMethod == EDynamicGlobalIlluminationMethod::Lumen
			&& InViewFamily.EngineShowFlags.GlobalIllumination && InViewFamily.EngineShowFlags.LumenGlobalIllumination
			&& !InViewFamily.EngineShowFlags.PathTracing && !InViewFamily.EngineShowFlags.RayTracingDebug)
		{
			bUsingLumen = true;
			return;
		}
	}
}
