// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroGameState.h"
#include "Kismet/GameplayStatics.h"

AToroGameState::AToroGameState()
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

AGameStateBase* AToroGameState::Get(const UObject* ContextObject, TSubclassOf<AGameStateBase> Class)
{
	AGameStateBase* GS = UGameplayStatics::GetGameState(ContextObject);
	return IsValid(GS) && (!IsValid(Class) || GS->IsA(Class)) ? GS : nullptr;
}
