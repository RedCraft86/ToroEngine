// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroGameMode.h"
#include "Framework/ToroGameState.h"
#include "Framework/ToroPlayerController.h"
#include "Framework/ToroPlayerCharacter.h"
#include "Framework/ToroPlayerState.h"
#include "Framework/ToroPlayerHUD.h"
#include "Framework/ToroGameInstance.h"
#include "Kismet/GameplayStatics.h"

AToroGameMode::AToroGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickGroup = TG_DuringPhysics;

	GameStateClass = AToroGameState::StaticClass();
	PlayerControllerClass = AToroPlayerController::StaticClass();
	DefaultPawnClass = AToroPlayerCharacter::StaticClass();
	PlayerStateClass = AToroPlayerState::StaticClass();
	HUDClass = AToroPlayerHUD::StaticClass();

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false; // Stop World Partition from unloading this
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLOD
	SetCanBeDamaged(false);
}

AGameModeBase* AToroGameMode::Get(const UObject* ContextObject, TSubclassOf<AGameModeBase> Class)
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(ContextObject);
	return IsValid(GM) && (!IsValid(Class) || GM->IsA(Class)) ? GM : nullptr;
}

void AToroGameMode::BeginPlay()
{
	Super::BeginPlay();
	GameInstance = GetGameInstance<UToroGameInstance>();
	if (GameInstance.IsValid())
	{
		GameInstance->WorldBeginPlay();
	}
}

void AToroGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (GameInstance.IsValid())
	{
		GameInstance->WorldTick(DeltaSeconds, IsPaused());
	}
}

void AToroGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (GameInstance.IsValid())
	{
		GameInstance->WorldEndPlay();
	}
}
