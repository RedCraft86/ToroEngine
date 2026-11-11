// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/ToroUserInterfaceConfig.h"
#include "ToroSystemSettings.h"

const FToroUserInterfaceConfig* FToroUserInterfaceConfig::Get()
{
	const UToroSystemSettings* Settings = GetDefault<UToroSystemSettings>();
	return IsValid(Settings) ? &Settings->UserInterface : nullptr;
}
