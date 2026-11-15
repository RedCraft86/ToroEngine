// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/UserInterfaceConfig.h"
#include "ToroDevSettings.h"

FUserInterfaceConfig::FUserInterfaceConfig()
{
	SettingsWidgetClass = FSoftClassPath(TEXT("/ToroEngine/UserInterface/WBP_ToroSettings.WBP_ToroSettings_C"));
	DialogWidgetClass = FSoftClassPath(TEXT("/ToroEngine/UserInterface/WBP_ToroDialog.WBP_ToroDialog_C"));

	AutoWidgetClasses = {
		TSoftClassPtr<UToroAutoWidget>(FSoftClassPath(
			TEXT("/ToroEngine/UserInterface/WBP_ToroSubtitle.WBP_ToroSubtitle_C"))),
	};
}

float FUserInterfaceConfig::CalculateTimeForText(const FText& InText, float ExtraTime) const
{
	if (InText.IsEmptyOrWhitespace())
	{
		return 0.0f;
	}

	const FString TextStr(InText.ToString());

	float WeightedChars = 0.0f;
	for (const TCHAR Character : TextStr)
	{
		switch (Character)
		{
		case TEXT('.'):
		case TEXT('?'):
		case TEXT('!'):
			WeightedChars += 1.5f;
			break;

		case TEXT(','):
		case TEXT(';'):
		case TEXT(':'):
			WeightedChars += 1.2f;
			break;

		case TEXT('\u2026'): // Ellipsis "…" match three weighted periods
			WeightedChars += 4.5f;
			break;

		default:
			WeightedChars += 1.0f;
			break;
		}
	}

	const float CalculatedTime = WeightedChars / FMath::Max(1.0f, TextCharactersPerSecond);
	return FMath::Max(TextMinDuration, CalculatedTime + FMath::Max(ExtraTime, 0.0f));
}

const FUserInterfaceConfig* FUserInterfaceConfig::Get()
{
	const UToroDevSettings* Settings = GetDefault<UToroDevSettings>();
	return IsValid(Settings) ? &Settings->UserInterface : nullptr;
}
