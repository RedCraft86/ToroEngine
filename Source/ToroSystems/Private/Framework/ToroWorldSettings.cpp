// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroWorldSettings.h"
#include "Framework/ToroGameInstance.h"
#include "PostProcess/WorldPostProcessComponent.h"
#include "Utilities/ToroUtilities.h"
#include "Sound/AudioSettings.h"
#include "Sound/SoundMix.h"
#include "Engine/World.h"
#include "AudioDevice.h"

AToroWorldSettings::AToroWorldSettings(): bIsMainLevelSettings(false)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_DuringPhysics;

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

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false; // Stop World Partition from unloading this
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLOD
	SetCanBeDamaged(false);
}

AWorldSettings* AToroWorldSettings::Get(const UObject* ContextObject, TSubclassOf<AWorldSettings> Class)
{
	const UWorld* World = ToroEngine::GetWorld(ContextObject);
	AWorldSettings* WS = IsValid(World) ? World->GetWorldSettings() : nullptr;
	return IsValid(WS) && (!IsValid(Class) || WS->IsA(Class)) ? WS : nullptr;
}

void AToroWorldSettings::SetSoundClassVolume(USoundClass* InClass, float InVolume) const
{
	if (DefaultBaseSoundMix && InClass && GEngine && GEngine->UseSound() && GetWorld()->bAllowAudioPlayback)
	{
		if (FAudioDeviceHandle AudioDevice = GetWorld()->GetAudioDevice())
		{
			AudioDevice->SetSoundMixClassOverride(DefaultBaseSoundMix, InClass, InVolume, 1.0f, 0.0f, true);
		}
	}
}

void AToroWorldSettings::BeginPlay()
{
	Super::BeginPlay();

	// Streamed levels may have their own world settings
	bIsMainLevelSettings = GetWorld()->GetWorldSettings() == this;

	GameInstance = GetGameInstance<UToroGameInstance>();
	if (bIsMainLevelSettings && GameInstance.IsValid())
	{
		GameInstance->WorldBeginPlay();
	}
}

void AToroWorldSettings::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bIsMainLevelSettings && GameInstance.IsValid())
	{
		GameInstance->WorldTick(DeltaSeconds, GetWorld()->IsPaused());
	}
}

void AToroWorldSettings::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (bIsMainLevelSettings && GameInstance.IsValid())
	{
		GameInstance->WorldEndPlay();
		GameInstance.Reset();
	}
}

#if WITH_EDITOR
void AToroWorldSettings::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (!DefaultBaseSoundMix)
	{
		if (const UAudioSettings* Settings = GetDefault<UAudioSettings>())
		{
			DefaultBaseSoundMix = Cast<USoundMix>(Settings->DefaultBaseSoundMix.TryLoad());
		}
	}
}
#endif