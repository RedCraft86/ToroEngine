// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroGameState.h"
#include "Kismet/GameplayStatics.h"

AGameStateBase* AToroGameState::Get(const UObject* ContextObject, TSubclassOf<AGameStateBase> Class)
{
	AGameStateBase* GS = UGameplayStatics::GetGameState(ContextObject);
	return IsValid(GS) && (!IsValid(Class) || GS->IsA(Class)) ? GS : nullptr;
}
