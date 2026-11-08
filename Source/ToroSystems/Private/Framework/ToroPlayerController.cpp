// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroPlayerController.h"
#include "Framework/ToroPlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

AToroPlayerController::AToroPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	PlayerCameraManagerClass = AToroPlayerCameraManager::StaticClass();

	PauseRequests->OnChanged().AddUObject(this, &OnPauseRequestsChanged);
	InputConfigs->OnChanged().AddUObject(this, &OnInputConfigsChanged);

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
