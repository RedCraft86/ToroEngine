// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/Registration/UserSettingsRegistry.h"
#include "UserSettings/Registration/UserSettingsExtension.h"
#include "UserSettings/UserSettingsConfig.h"

FUserSettingsSection& FUserSettingsPage::FindOrAddSection(FName Key, const FText& InDisplayName)
{
	checkf(!Key.IsNone(), TEXT("Section key cannot be none"));

	if (const TPair<FName, TUniquePtr<FUserSettingsSection>>* FoundPage = Sections.FindByPredicate(
		[Key](const TPair<FName, TUniquePtr<FUserSettingsSection>>& Page)
		{
			return Page.Key == Key;
		}))
	{
		FoundPage->Value->DisplayName = InDisplayName;
		return *FoundPage->Value;
	}

	TUniquePtr<FUserSettingsSection> Section = MakeUnique<FUserSettingsSection>(InDisplayName);
	FUserSettingsSection& Result = *Section;
	Sections.Add({Key, MoveTemp(Section)});
	return Result;
}

FUserSettingsPage& FUserSettingsRegistry::FindOrAddPage(FName Key, const FText& InDisplayName)
{
	checkf(!Key.IsNone(), TEXT("Page key cannot be none"));

	if (const TPair<FName, TUniquePtr<FUserSettingsPage>>* FoundPage = Pages.FindByPredicate(
		[Key](const TPair<FName, TUniquePtr<FUserSettingsPage>>& Page)
		{
			return Page.Key == Key;
		}))
	{
		FoundPage->Value->DisplayName = InDisplayName;
		return *FoundPage->Value;
	}

	TUniquePtr<FUserSettingsPage> Page = MakeUnique<FUserSettingsPage>(InDisplayName);
	FUserSettingsPage& Result = *Page;
	Pages.Add({Key, MoveTemp(Page)});
	return Result;
}

void FUserSettingsRegistry::RegisterExtension(UToroGameUserSettings& UserSettings)
{
	const FUserSettingsConfig* Cfg = FUserSettingsConfig::Get();
	const TSubclassOf<UUserSettingsExtension> Class = Cfg ? Cfg->SettingsExtension.LoadSynchronous() : nullptr;
	if (const UUserSettingsExtension* Extension = Class->GetDefaultObject<UUserSettingsExtension>())
	{
		Extension->RegisterSettings(*this, &UserSettings);
	}
}

#define LOCTEXT_NAMESPACE "ToroEngine"
void FUserSettingsRegistry::BuildStandardRegistry(UToroGameUserSettings& Settings)
{
	// Register the core stuff


	RegisterExtension(Settings);
}
#undef