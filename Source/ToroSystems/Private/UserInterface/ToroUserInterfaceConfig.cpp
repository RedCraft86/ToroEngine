// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/ToroUserInterfaceConfig.h"
#include "ToroDevSettings.h"

const FToroUserInterfaceConfig* FToroUserInterfaceConfig::Get()
{
	const UToroDevSettings* Settings = GetDefault<UToroDevSettings>();
	return IsValid(Settings) ? &Settings->UserInterface : nullptr;
}
