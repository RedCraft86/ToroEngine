// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

ACharacter* AToroPlayerCharacter::Get(const UObject* ContextObject, TSubclassOf<ACharacter> Class, int32 PlayerIdx)
{
	ACharacter* PC = UGameplayStatics::GetPlayerCharacter(ContextObject, PlayerIdx);
	return IsValid(PC) && (!IsValid(Class) || PC->IsA(Class)) ? PC : nullptr;
}
