// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ToroDevSettings.h"
#include "GeneralProjectSettings.h"

UToroDevSettings::UToroDevSettings()
{
	CategoryName = TEXT("Project");
	SectionName = TEXT("ToroEngineRuntime");
}

FString UToroDevSettings::GetVersionInfo() const
{
	FString Result;
	if (const UGeneralProjectSettings* ProjectSettings = GetDefault<UGeneralProjectSettings>())
	{
		Result.Appendf(TEXT("Version: %s-%s"), *ProjectSettings->ProjectVersion,
			*FString(LexToString(FApp::GetBuildConfiguration())).ToUpper());
	}

	if (!DemoName.IsNone())
	{
		Result.Appendf(TEXT(" | %s"), *DemoName.ToString());
	}

	return Result;
}
