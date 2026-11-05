// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroGameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UGameViewportClient* UToroGameViewportClient::Get(const UObject* ContextObject, TSubclassOf<UGameViewportClient> Class)
{
	const UGameInstance* GI = UGameplayStatics::GetGameInstance(ContextObject);
	UGameViewportClient* GVC = IsValid(GI) ? GI->GetGameViewportClient() : nullptr;
	return IsValid(GVC) && (!IsValid(Class) || GVC->IsA(Class)) ? GVC : nullptr;
}
