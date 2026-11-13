// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "EntryTypes/UserSettingEntry.h"
#include "EntryTypes/UserSettingAction.h"
#include "EntryTypes/UserSettingSlider.h"
#include "EntryTypes/UserSettingToggle.h"
#include "EntryTypes/UserSettingSelection.h"
#include "UserSettings/UserSettingsTypes.h"

struct FUserSettingsSection
{
	[[nodiscard]] const FUserSettingEntry* operator[](int32 Index)
	{
		if (!Entries.IsValidIndex(Index)
			|| !Entries[Index].IsValid())
		{
			return nullptr;
		}

		return Entries[Index].Get();
	}

	[[nodiscard]] FORCEINLINE int32 Num() const
	{
		return Entries.Num();
	}

	[[nodiscard]] FORCEINLINE const FText& GetDisplayName() const
	{
		return DisplayName;
	}

	FUserSettingSlider& AddSlider(FGameplayTag Key)
	{
		return AddEntry<FUserSettingSlider>(Key);
	}

	FUserSettingToggle& AddToggle(FGameplayTag Key)
	{
		return AddEntry<FUserSettingToggle>(Key);
	}

	template<typename T>
	TUserSettingSelection<T>& AddSelector(FGameplayTag Key)
	{
		return AddEntry<TUserSettingSelection<T>>(Key);
	}

	FUserSettingAction& AddAction(FGameplayTag Key)
	{
		return AddEntry<FUserSettingAction>(Key);
	}

private:

	FText DisplayName;
    TArray<TUniquePtr<FUserSettingEntry>> Entries;

	template<typename EntryType>
	EntryType& AddEntry(FGameplayTag Key)
	{
		checkf(UserSettingsTags::IsValid(Key),
			TEXT("Provided key %s is not a derivative of UserSetting tags"),
			*Key.ToString()
		);

		checkf(!Entries.ContainsByPredicate([Key](const TUniquePtr<FUserSettingEntry>& Entry)
		{
			return Entry->GetKey() == Key;
		}), TEXT("An entry is already defined for key %s"), *Key.ToString());

		TUniquePtr<EntryType> Entry = MakeUnique<EntryType>(Key);
		EntryType& Result = *Entry;
		Entries.Add(MoveTemp(Entry));
		return Result;
	}
};

struct FUserSettingsPage
{
	[[nodiscard]] const FUserSettingsSection* operator[](int32 Index)
	{
		if (!Sections.IsValidIndex(Index)
			|| !Sections[Index].Value.IsValid())
		{
			return nullptr;
		}

		return Sections[Index].Value.Get();
	}

	[[nodiscard]] FORCEINLINE int32 Num() const
	{
		return Sections.Num();
	}

	[[nodiscard]] FORCEINLINE const FText& GetDisplayName() const
	{
		return DisplayName;
	}

	[[nodiscard]] FUserSettingsSection& FindOrAddSection(FName Key);

private:

	FText DisplayName;
	TArray<TPair<FName, TUniquePtr<FUserSettingsSection>>> Sections;
};

struct FUserSettingsRegistry
{
	[[nodiscard]] const FUserSettingsPage* operator[](int32 Index)
	{
		if (!Pages.IsValidIndex(Index)
			|| !Pages[Index].Value.IsValid())
		{
			return nullptr;
		}

		return Pages[Index].Value.Get();
	}

	[[nodiscard]] FORCEINLINE int32 Num() const
	{
		return Pages.Num();
	}

	[[nodiscard]] FUserSettingsPage& FindOrAddPage(FName Key);
	void RegisterExtension(UToroGameUserSettings* UserSettings);

private:

	TArray<TPair<FName, TUniquePtr<FUserSettingsPage>>> Pages;
};