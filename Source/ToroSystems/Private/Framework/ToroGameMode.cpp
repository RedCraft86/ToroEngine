// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroGameMode.h"
#include "Kismet/GameplayStatics.h"

AGameModeBase* AToroGameMode::Get(const UObject* ContextObject, TSubclassOf<AGameModeBase> Class)
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(ContextObject);
	return IsValid(GM) && (!IsValid(Class) || GM->IsA(Class)) ? GM : nullptr;
}
