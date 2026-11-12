// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/UserInterfaceConfig.h"
#include "ToroDevSettings.h"

const FUserInterfaceConfig* FUserInterfaceConfig::Get()
{
	const UToroDevSettings* Settings = GetDefault<UToroDevSettings>();
	return IsValid(Settings) ? &Settings->UserInterface : nullptr;
}
