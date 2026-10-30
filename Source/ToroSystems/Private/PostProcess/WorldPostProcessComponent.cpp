// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "PostProcess/WorldPostProcessComponent.h"
#include "PostProcess/LumenSceneViewExtension.h"
#include "SceneViewExtension.h"
#include "Engine/World.h"

UWorldPostProcessComponent::UWorldPostProcessComponent(): Priority(0.0f)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = true;

	Settings.bOverride_BloomMethod = true;
	Settings.BloomMethod = BM_FFT;

	Settings.bOverride_AutoExposureMethod = true;
	Settings.AutoExposureMethod = AEM_Manual;
	Settings.bOverride_AutoExposureBias = true;
	Settings.AutoExposureBias = 10.0f;

	Settings.bOverride_DynamicGlobalIlluminationMethod = true;
	Settings.DynamicGlobalIlluminationMethod = EDynamicGlobalIlluminationMethod::Lumen;
	Settings.bOverride_ReflectionMethod = true;
	Settings.ReflectionMethod = EReflectionMethod::Lumen;
}

bool UWorldPostProcessComponent::IsUsingLumen() const
{
	if (LumenSceneViewExtension.IsValid())
	{
		return LumenSceneViewExtension->IsUsingLumen();
	}

	return Settings.bOverride_DynamicGlobalIlluminationMethod
		&& Settings.DynamicGlobalIlluminationMethod == EDynamicGlobalIlluminationMethod::Lumen;
}

FPostProcessVolumeProperties UWorldPostProcessComponent::GetProperties() const
{
	FPostProcessVolumeProperties Properties;
	Properties.Settings = &Settings;
	Properties.Priority = Priority;
	Properties.BlendRadius = 0.0f;
	Properties.BlendWeight = 1.0f;
	Properties.bIsEnabled = true;
	Properties.bIsUnbound = true;
	Properties.Size = DBL_MAX;
	Properties.VolumeGuid = VolumeGuid;
	return Properties;
}

bool UWorldPostProcessComponent::EncompassesPoint(FVector Point, float SphereRadius, float* OutDistanceToPoint)
{
	if (OutDistanceToPoint)
	{
		*OutDistanceToPoint = 0.0f;
	}

	return true;
}

void UWorldPostProcessComponent::OnRegister()
{
	Super::OnRegister();
	if (UWorld* World = GetWorld())
	{
		World->AddPostProcessVolume(this);
		LumenSceneViewExtension = FSceneViewExtensions::NewExtension<FLumenSceneViewExtension>(World);
	}
}

void UWorldPostProcessComponent::OnUnregister()
{
	LumenSceneViewExtension.Reset();
	if (UWorld* World = GetWorld())
	{
		World->RemovePostProcessVolume(this);
	}
	Super::OnUnregister();
}

#if WITH_EDITORONLY_DATA
void UWorldPostProcessComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	if (Ar.IsLoading())
	{
		Settings.OnAfterLoad();
	}
}
#endif
#if WITH_EDITOR
void UWorldPostProcessComponent::PostInitProperties()
{
	Super::PostInitProperties();
	if (!IsTemplate() && !VolumeGuid.IsValid())
	{
		VolumeGuid = FGuid::NewDeterministicGuid(GetPathName());
	}
}
#endif
