// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "LightProbes/LightProbeSubsystem.h"
#include "LightProbes/LightProbeComponent.h"
#include "LightProbes/LightProbeSystemConfig.h"
#include "PostProcess/WorldPostProcessComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Framework/ToroWorldSettings.h"
#include "Libraries/ToroCameraLibrary.h"
#include "Utilities/ToroUtilities.h"
#include "Engine/World.h"

namespace
{
	const FName PARAM_NumProbes = TEXT("NumProbes");

	FName PARAM_PosData(int32 Index)
	{
		const uint8 LightIdx = FMath::Clamp(Index, 0, MAX_SUPPORTED_LIGHT_PROBE_COUNT);
		return *(TEXT("PosData_") + FString::FromInt(LightIdx));
	}

	FName PARAM_EffectData(int32 Index)
	{
		const uint8 LightIdx = FMath::Clamp(Index, 0, MAX_SUPPORTED_LIGHT_PROBE_COUNT);
		return *(TEXT("EffectData_") + FString::FromInt(LightIdx));
	}
}

ULightProbeSubsystem* ULightProbeSubsystem::Get(const UObject* Context)
{
	const UWorld* World = ToroEngine::GetWorld(Context);
	return IsValid(World) ? World->GetSubsystem<ULightProbeSubsystem>() : nullptr;
}

void ULightProbeSubsystem::RegisterProbe(const ULightProbeComponent* InComponent)
{
	if (IsValid(InComponent))
	{
		Probes.AddUnique(InComponent);
		ClearInvalidProbes();
	}
}

void ULightProbeSubsystem::UnregisterProbe(const ULightProbeComponent* InComponent)
{
	if (IsValid(InComponent))
	{
		Probes.Remove(InComponent);
		ClearInvalidProbes();
	}
}

void ULightProbeSubsystem::ClearInvalidProbes()
{
	Probes.RemoveAllSwap([](const TWeakObjectPtr<const ULightProbeComponent>& Element) -> bool
	{
		return !Element.IsValid();
	});
}

bool ULightProbeSubsystem::IsSystemValid() const
{
	return WorldSettings.IsValid()
		&& PostProcess.IsValid()
		&& GetConfig().IsValid();
}

const FLightProbeSystemConfig& ULightProbeSubsystem::GetConfig() const
{
	return WorldSettings->GetLightProbeConfig();
}

void ULightProbeSubsystem::UpdatePostProcessBlendable(bool bRemoval) const
{
	if (IsSystemValid())
	{
		if (bRemoval || !GetConfig().IsEnabled())
		{
			PostProcess->Settings.RemoveBlendable(GetConfig().PostProcessMaterial);
		}
		else
		{
			PostProcess->Settings.AddBlendable(GetConfig().PostProcessMaterial, 1.0f);
		}
	}
}

bool ULightProbeSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game
		|| WorldType == EWorldType::Editor
		|| WorldType == EWorldType::PIE;
}

void ULightProbeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UWorld* World = GetWorld();
	if (!ensureMsgf(IsValid(World), TEXT("[%s] Cannot initialize LightProbes due to an invalid world."), *GetName()))
	{
		return;
	}

	WorldSettings = Cast<AToroWorldSettings>(World->GetWorldSettings());
	if (!ensureMsgf(WorldSettings.IsValid(),
		TEXT("[%s] Cannot initialize Light Probes due to invalid World Settings actor.")
		TEXT("Did you forget to change it to AToroWorldSettings or its subclass?"), *GetName()))
	{
		return;
	}

	PostProcess = WorldSettings->GetPostProcess();
	if (!ensureMsgf(PostProcess.IsValid(),
		TEXT("[%s] Cannot initialize Light Probes due to invalid World Post Process.")
		TEXT("Your level data related to the World Settings might be corrupted."), *GetName()))
	{
		return;
	}

	ensureMsgf(GetConfig().IsValid(),
		TEXT("[%s] Cannot initialize Light Probes due to invalid configs.")
		TEXT("Is the material and/or parameter collection provided in the World Settings."), *GetName());
}

void ULightProbeSubsystem::Deinitialize()
{
	UpdatePostProcessBlendable(true);
	WorldSettings.Reset();
	PostProcess.Reset();
	Probes.Empty();

	Super::Deinitialize();
}

void ULightProbeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!IsSystemValid())
	{
		CooldownTime = 0.0f;
		return;
	}

	if ((CooldownTime -= DeltaTime) > 0.0f)
	{
		return;
	}

	CooldownTime = FLightProbeSystemConfig::GetUpdateInterval();

	UMaterialParameterCollectionInstance* ParamCollection = GetWorld()
		->GetParameterCollectionInstance(GetConfig().ParameterCollection);
	if (!ensureMsgf(ParamCollection,
		TEXT("Failed to get the world instance of Parameter Collection %s"),
		*GetNameSafe(GetConfig().ParameterCollection)))
	{
		return;
	}

	UpdatePostProcessBlendable();
	TArray<TWeakObjectPtr<const ULightProbeComponent>> ActiveProbes;
	const FTransform CamTransform = UToroCameraLibrary::GetViewTransform(this);
	if (GetConfig().IsEnabled())
	{
		ClearInvalidProbes();

		ActiveProbes = Probes.FilterByPredicate([&CamTransform, this](
			const TWeakObjectPtr<const ULightProbeComponent>& Element
		) -> bool
		{
			return Element.IsValid() && Element->IsProbeRelevant(CamTransform, PostProcess->IsUsingLumen());
		});

		ActiveProbes.Sort([&CamTransform](
			const TWeakObjectPtr<const ULightProbeComponent>& A,
			const TWeakObjectPtr<const ULightProbeComponent>& B
		) -> bool
		{
			return FVector::DistSquared(A->GetComponentLocation(), CamTransform.GetLocation()) <
				FVector::DistSquared(B->GetComponentLocation(), CamTransform.GetLocation());
		});
	}

	const int32 Budget = FMath::Clamp(FLightProbeSystemConfig::GetMaxProbeLimit(), 0, MAX_SUPPORTED_LIGHT_PROBE_COUNT);
	const int32 NumActive = FMath::Min(ActiveProbes.Num(), Budget);

	ParamCollection->SetScalarParameterValue(PARAM_NumProbes, NumActive);
	for (int32 i = 0; i < MAX_SUPPORTED_LIGHT_PROBE_COUNT; i++)
	{
		FVector4 PosData = FVector4::Zero();
		FVector4 EffectData = FVector4::Zero();
		if (i < NumActive)
		{
			ActiveProbes[i]->GetProbeData(CamTransform, PosData, EffectData);
		}

		ParamCollection->SetVectorParameterValue(PARAM_PosData(i), PosData);
		ParamCollection->SetVectorParameterValue(PARAM_EffectData(i), EffectData);
	}
}
