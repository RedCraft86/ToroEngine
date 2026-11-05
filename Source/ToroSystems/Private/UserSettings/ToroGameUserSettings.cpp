// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/ToroGameUserSettings.h"
#include "Engine/Engine.h"

UGameUserSettings* UToroGameUserSettings::Get(TSubclassOf<UGameUserSettings> Class)
{
	UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	return IsValid(GUS) && (!IsValid(Class) || GUS->IsA(Class)) ? GUS : nullptr;
}
