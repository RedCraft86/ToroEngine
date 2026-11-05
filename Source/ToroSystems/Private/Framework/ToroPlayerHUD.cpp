// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroPlayerHUD.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AHUD* AToroPlayerHUD::Get(const UObject* ContextObject, TSubclassOf<AHUD> Class, int32 PlayerIdx)
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(ContextObject, PlayerIdx);
	AHUD* HUD = IsValid(PC) ? PC->GetHUD() : nullptr;
	return IsValid(HUD) && (!IsValid(Class) || HUD->IsA(Class)) ? HUD : nullptr;
}
