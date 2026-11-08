// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroGameMode.h"
#include "Framework/ToroGameState.h"
#include "Framework/ToroPlayerController.h"
#include "Framework/ToroPlayerCharacter.h"
#include "Framework/ToroPlayerState.h"
#include "Framework/ToroPlayerHUD.h"
#include "Kismet/GameplayStatics.h"

AToroGameMode::AToroGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
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
