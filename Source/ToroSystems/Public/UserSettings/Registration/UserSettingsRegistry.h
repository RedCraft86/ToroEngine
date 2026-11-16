// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "EntryTypes/UserSettingEntry.h"
#include "EntryTypes/UserSettingAction.h"
#include "EntryTypes/UserSettingSlider.h"
#include "EntryTypes/UserSettingToggle.h"
#include "EntryTypes/UserSettingSelection.h"
#include "UserSettings/UserSettingsTypes.h"

class FUserSettingsSection
{
	friend class FUserSettingsPage;

	FUserSettingMetadata Metadata;
    TArray<TUniquePtr<FUserSettingEntry>> Entries;

	template<typename EntryType>
	EntryType& AddEntry(FName Key)
	{
		checkf(!Entries.ContainsByPredicate([Key](const TUniquePtr<FUserSettingEntry>& Entry)
		{
			return Entry->GetKey() == Key;
		}), TEXT("An entry is already defined for key %s"), *Key.ToString());

		TUniquePtr<EntryType> Entry = MakeUnique<EntryType>(Key);
		EntryType& Result = *Entry;
		Entries.Add(MoveTemp(Entry));
		return Result;
	}

public:

	FUserSettingsSection() = delete;
	FUserSettingsSection(const FUserSettingsSection&) = delete;
	FUserSettingsSection& operator=(const FUserSettingsSection&) = delete;

	explicit FUserSettingsSection(const FText& InDisplayName)
	{
		Metadata.DisplayName = InDisplayName;
		Metadata.bIsCategory = true;
	}

	[[nodiscard]] const FUserSettingEntry* operator[](int32 Index) const
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

	[[nodiscard]] FORCEINLINE FUserSettingMetadata& GetMetadata()
	{
		return Metadata;
	}

	[[nodiscard]] FORCEINLINE const FUserSettingMetadata& GetMetadata() const
	{
		return Metadata;
	}

	FUserSettingToggle& AddToggle(FName Key)
	{
		return AddEntry<FUserSettingToggle>(Key);
	}

	FUserSettingToggle& AddToggle(FGameplayTag Key)
	{
		return AddToggle(Key.GetTagName());
	}

	FUserSettingSlider& AddSlider(FName Key)
	{
		return AddEntry<FUserSettingSlider>(Key);
	}

	FUserSettingSlider& AddSlider(FGameplayTag Key)
	{
		return AddSlider(Key.GetTagName());
	}

	template<typename T>
	TUserSettingSelection<T>& AddSelector(FName Key)
	{
		return AddEntry<TUserSettingSelection<T>>(Key);
	}

	template<typename T>
	TUserSettingSelection<T>& AddSelector(FGameplayTag Key)
	{
		return AddSelector<T>(Key.GetTagName());
	}

	FUserSettingAction& AddAction(FName Key)
	{
		return AddEntry<FUserSettingAction>(Key);
	}

	FUserSettingAction& AddAction(FGameplayTag Key)
	{
		return AddAction(Key.GetTagName());
	}
};

class FUserSettingsPage
{
	friend class FUserSettingsRegistry;

	FUserSettingMetadata Metadata;
	TArray<TPair<FName, TUniquePtr<FUserSettingsSection>>> Sections;

public:

	FUserSettingsPage() = delete;
	FUserSettingsPage(const FUserSettingsPage&) = delete;
	FUserSettingsPage& operator=(const FUserSettingsPage&) = delete;

	explicit FUserSettingsPage(const FText& InDisplayName)
	{
		Metadata.DisplayName = InDisplayName;
		Metadata.bIsCategory = true;
	}

	[[nodiscard]] const FUserSettingsSection* operator[](int32 Index) const
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

	[[nodiscard]] FORCEINLINE FUserSettingMetadata& GetMetadata()
	{
		return Metadata;
	}

	[[nodiscard]] FORCEINLINE const FUserSettingMetadata& GetMetadata() const
	{
		return Metadata;
	}

	[[nodiscard]] FUserSettingsSection& EditSection(FName Key, const FText& DisplayName);
};

class FUserSettingsRegistry
{
	TArray<TPair<FName, TUniquePtr<FUserSettingsPage>>> Pages;

public:

	FUserSettingsRegistry() {}
	FUserSettingsRegistry(const FUserSettingsRegistry&) = delete;
	FUserSettingsRegistry& operator=(const FUserSettingsRegistry&) = delete;

	[[nodiscard]] const FUserSettingsPage* operator[](int32 Index) const
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

	[[nodiscard]] FUserSettingsPage& EditPage(FName Key, const FText& DisplayName);

	void Reset()
	{
		Pages.Reset();
	}
};
