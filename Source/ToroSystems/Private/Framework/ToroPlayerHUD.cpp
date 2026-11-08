// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroPlayerHUD.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

AToroPlayerHUD::AToroPlayerHUD()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_DuringPhysics;
	PrimaryActorTick.TickInterval = 0.033f; // 30 FPS

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false; // Stop World Partition from unloading this
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLOD
	SetCanBeDamaged(false);
}

AHUD* AToroPlayerHUD::Get(const UObject* ContextObject, TSubclassOf<AHUD> Class, int32 PlayerIdx)
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(ContextObject, PlayerIdx);
	AHUD* HUD = IsValid(PC) ? PC->GetHUD() : nullptr;
	return IsValid(HUD) && (!IsValid(Class) || HUD->IsA(Class)) ? HUD : nullptr;
}

void AToroPlayerHUD::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* PC = GetOwningPlayerController())
	{
		CameraManager = PC->PlayerCameraManager;
		MasterWidget = CreateWidget<UToroMasterWidget>(PC);
		if (MasterWidget)
		{
			MasterWidget->AddToPlayerScreen();
		}
	}
}

void AToroPlayerHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MasterWidget)
	{
		MasterWidget->RemoveFromParent();
		MasterWidget = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void AToroPlayerHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (CameraManager.IsValid() && CameraManager->bEnableFading)
	{
		MasterWidget->SetScreenFade(CameraManager->FadeAmount);
	}
	else
	{
		MasterWidget->SetScreenFade(0.0f);
	}
}
