// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

APlayerState* AToroPlayerState::Get(const UObject* ContextObject, TSubclassOf<APlayerState> Class, int32 PlayerIdx)
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(ContextObject, PlayerIdx);
	APlayerState* PS = IsValid(PC) ? PC->GetPlayerState<APlayerState>() : nullptr;
	return IsValid(PS) && (!IsValid(Class) || PS->IsA(Class)) ? PS : nullptr;
}
