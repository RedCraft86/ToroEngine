// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroPlayerController.h"
#include "Kismet/GameplayStatics.h"

APlayerController* AToroPlayerController::Get(const UObject* ContextObject,
	TSubclassOf<APlayerController> Class, int32 PlayerIdx)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(ContextObject, PlayerIdx);
	return IsValid(PC) && (!IsValid(Class) || PC->IsA(Class)) ? PC : nullptr;
}
