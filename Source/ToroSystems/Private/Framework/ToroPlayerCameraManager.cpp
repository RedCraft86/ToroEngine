// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroPlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

APlayerCameraManager* AToroPlayerCameraManager::Get(const UObject* ContextObject,
	TSubclassOf<APlayerCameraManager> Class, int32 PlayerIdx)
{
	APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(ContextObject, PlayerIdx);
	return IsValid(PCM) && (!IsValid(Class) || PCM->IsA(Class)) ? PCM : nullptr;
}
