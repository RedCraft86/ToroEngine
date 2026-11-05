// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroGameInstance.h"
#include "Kismet/GameplayStatics.h"

UGameInstance* UToroGameInstance::Get(const UObject* ContextObject, TSubclassOf<UGameInstance> Class)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(ContextObject);
	return IsValid(GI) && (!IsValid(Class) || GI->IsA(Class)) ? GI : nullptr;
}
