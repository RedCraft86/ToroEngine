// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroPlayerController.h"
#include "Framework/ToroPlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

AToroPlayerController::AToroPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	bShouldPerformFullTickWhenPaused = true;

	PlayerCameraManagerClass = AToroPlayerCameraManager::StaticClass();

	PauseRequests->OnChanged().AddUObject(this, &AToroPlayerController::OnPauseRequestsChanged);
	InputConfigs->OnChanged().AddUObject(this, &AToroPlayerController::OnInputConfigsChanged);

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false; // Stop World Partition from unloading this
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLOD
	SetCanBeDamaged(false);
}

APlayerController* AToroPlayerController::Get(const UObject* ContextObject,
	TSubclassOf<APlayerController> Class, int32 PlayerIdx)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(ContextObject, PlayerIdx);
	return IsValid(PC) && (!IsValid(Class) || PC->IsA(Class)) ? PC : nullptr;
}

void AToroPlayerController::RequestPause(const UObject* Source)
{
	PauseRequests->Add(Source);
}

void AToroPlayerController::ReleasePause(const UObject* Source)
{
	PauseRequests->Remove(Source);
}

void AToroPlayerController::RequestInputConfig(const UObject* Source, const FToroInputConfig& InConfig)
{
	InputConfigs->Add(Source, InConfig);
}

void AToroPlayerController::ReleaseInputConfig(const UObject* Source)
{
	InputConfigs->Remove(Source);
}

void AToroPlayerController::OnPauseRequestsChanged()
{
	SetPause(!PauseRequests->IsEmpty());
}

void AToroPlayerController::OnInputConfigsChanged()
{
	FToroInputConfig Config = DefaultInputConfig;
	if (const FToroInputConfig* Last = InputConfigs->GetLast())
	{
		Config = *Last;
	}

	FlushPressedKeys();
	SetInputMode(Config.GetInputMode());
	SetShowMouseCursor(Config.ShouldShowMouse());
}

void AToroPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		World->bIsCameraMoveableWhenPaused = true;
	}

	OnInputConfigsChanged();
}
