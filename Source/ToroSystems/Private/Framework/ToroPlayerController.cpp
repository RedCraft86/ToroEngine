// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroPlayerController.h"
#include "Framework/ToroPlayerCameraManager.h"
#include "EnhancedInputSubsystems.h"
#include "DataTypes/ToroLevelInfo.h"
#include "Kismet/GameplayStatics.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

AToroPlayerController::AToroPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	bShouldPerformFullTickWhenPaused = true;

	PlayerCameraManagerClass = AToroPlayerCameraManager::StaticClass();

	PauseRequests->OnChanged().AddUObject(this, &AToroPlayerController::OnPauseRequestsChanged);

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

void AToroPlayerController::OnPauseRequestsChanged()
{
	SetPause(!PauseRequests->IsEmpty());
}

void AToroPlayerController::BeginPlay()
{
	Super::BeginPlay();

	const FToroLevelInfo* LevelInfo = FToroLevelInfo::Get(this);
	auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (LevelInfo && InputSubsystem)
	{
		if (const UInputMappingContext* InputMappings = LevelInfo->InputMappings.LoadSynchronous())
		{
			InputSubsystem->AddMappingContext(InputMappings, 0);
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->bIsCameraMoveableWhenPaused = true;
	}
}
