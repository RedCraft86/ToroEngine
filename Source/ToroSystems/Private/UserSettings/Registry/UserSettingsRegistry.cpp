// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/Registration/UserSettingsRegistry.h"

FUserSettingsSection& FUserSettingsPage::EditSection(FName Key, const FText& DisplayName)
{
	checkf(!Key.IsNone(), TEXT("Section key cannot be none"));

	if (const TPair<FName, TUniquePtr<FUserSettingsSection>>* FoundSection = Sections.FindByPredicate(
		[Key](const TPair<FName, TUniquePtr<FUserSettingsSection>>& Page)
		{
			return Page.Key == Key;
		}))
	{
		if (!DisplayName.IsEmptyOrWhitespace())
		{
			FoundSection->Value->Metadata.DisplayName = DisplayName;
		}
		return *FoundSection->Value;
	}

	TUniquePtr<FUserSettingsSection> Section = MakeUnique<FUserSettingsSection>(
		DisplayName.IsEmptyOrWhitespace() ? FText::FromName(Key) : DisplayName
	);

	FUserSettingsSection& Result = *Section;
	Sections.Add({Key, MoveTemp(Section)});
	return Result;
}

FUserSettingsPage& FUserSettingsRegistry::EditPage(FName Key, const FText& DisplayName)
{
	checkf(!Key.IsNone(), TEXT("Page key cannot be none"));

	if (const TPair<FName, TUniquePtr<FUserSettingsPage>>* FoundPage = Pages.FindByPredicate(
		[Key](const TPair<FName, TUniquePtr<FUserSettingsPage>>& Page)
		{
			return Page.Key == Key;
		}))
	{
		if (!DisplayName.IsEmptyOrWhitespace())
		{
			FoundPage->Value->Metadata.DisplayName = DisplayName;
		}
		return *FoundPage->Value;
	}

	TUniquePtr<FUserSettingsPage> Page = MakeUnique<FUserSettingsPage>(
		DisplayName.IsEmptyOrWhitespace() ? FText::FromName(Key) : DisplayName
	);

	FUserSettingsPage& Result = *Page;
	Pages.Add({Key, MoveTemp(Page)});
	return Result;
}

