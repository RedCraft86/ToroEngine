// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroPlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

AToroPlayerCameraManager::AToroPlayerCameraManager()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickGroup = TG_DuringPhysics;

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false; // Stop World Partition from unloading this
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLOD
	SetCanBeDamaged(false);
}

APlayerCameraManager* AToroPlayerCameraManager::Get(const UObject* ContextObject,
	TSubclassOf<APlayerCameraManager> Class, int32 PlayerIdx)
{
	APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(ContextObject, PlayerIdx);
	return IsValid(PCM) && (!IsValid(Class) || PCM->IsA(Class)) ? PCM : nullptr;
}
