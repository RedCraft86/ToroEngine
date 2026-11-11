// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ToroDevSettings.h"

UToroDevSettings::UToroDevSettings()
{
	CategoryName = TEXT("Project");
	SectionName = TEXT("ToroEngineRuntime");

	UserInterface.DialogWidgetClass = FSoftClassPath(TEXT("/ToroEngine/UserInterface/WBP_ToroDialog.WBP_ToroDialog_C"));
}
