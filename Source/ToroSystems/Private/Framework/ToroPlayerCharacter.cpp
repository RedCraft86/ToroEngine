// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

AToroPlayerCharacter::AToroPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	AutoReceiveInput = EAutoReceiveInput::Player0;

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false; // Stop World Partition from unloading this
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLOD
	SetCanBeDamaged(false);
}

ACharacter* AToroPlayerCharacter::Get(const UObject* ContextObject, TSubclassOf<ACharacter> Class, int32 PlayerIdx)
{
	ACharacter* PC = UGameplayStatics::GetPlayerCharacter(ContextObject, PlayerIdx);
	return IsValid(PC) && (!IsValid(Class) || PC->IsA(Class)) ? PC : nullptr;
}
