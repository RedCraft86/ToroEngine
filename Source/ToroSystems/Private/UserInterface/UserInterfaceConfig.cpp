// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/UserInterfaceConfig.h"
#include "ToroDevSettings.h"

FUserInterfaceConfig::FUserInterfaceConfig()
{
	SettingsWidgetClass = FSoftClassPath(TEXT("/ToroEngine/UserInterface/WBP_ToroSettings.WBP_ToroSettings_C"));
	DialogWidgetClass = FSoftClassPath(TEXT("/ToroEngine/UserInterface/WBP_ToroDialog.WBP_ToroDialog_C"));
}

const FUserInterfaceConfig* FUserInterfaceConfig::Get()
{
	const UToroDevSettings* Settings = GetDefault<UToroDevSettings>();
	return IsValid(Settings) ? &Settings->UserInterface : nullptr;
}
