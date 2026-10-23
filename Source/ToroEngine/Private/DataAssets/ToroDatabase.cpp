// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DataAssets/ToroDatabase.h"
#include "Misc/DataValidation.h"
#include "UObject/UnrealType.h"
#include "UObject/Class.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
bool UToroDatabase::IsValidKey(const FGameplayTag& Key) const
{
	return RootTag.IsValid() && Key.IsValid() && Key != RootTag && Key.MatchesTag(RootTag);
}

bool UToroDatabase::DoesKeyExist(const FGameplayTag& Key) const
{
	return IsValidKey(Key) && Entries.Contains(Key);
}

const FInstancedStruct& UToroDatabase::GetValue(const FGameplayTag& Key) const
{
	if (!ensureMsgf(RootTag.IsValid(), TEXT("RootTag for database %s is invalid."), *GetName())
		|| !ensureMsgf(IsValid(RootStruct.Get()) && RootStruct->IsChildOf<FToroDatabaseEntry>(),
			TEXT("RootStruct for database %s is invalid."), *GetName())
		|| !IsValidKey(Key))
	{
		return EmptyValue;
	}

	const FInstancedStruct* FoundValue = Entries.Find(Key);
	if (!FoundValue || !FoundValue->IsValid() || !FoundValue->GetScriptStruct()->IsChildOf(RootStruct))
	{
		return EmptyValue;
	}

	return *FoundValue;
}

#if WITH_EDITOR
EDataValidationResult UToroDatabase::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult SuperResult = Super::IsDataValid(Context);

	const bool bValidRootTag = RootTag.IsValid();
	const bool bValidRootStruct = IsValid(RootStruct.Get()) && RootStruct->IsChildOf<FToroDatabaseEntry>();
	if (!bValidRootTag || !bValidRootStruct)
	{
		if (!bValidRootTag)
		{
			Context.AddError(LOCTEXT("DatabaseRootTagInvalid", "Database RootTag is missing or invalid."));
		}
		if (!bValidRootStruct)
		{
			Context.AddError(LOCTEXT("DatabaseRootStructInvalid", "Database RootStruct is missing or invalid. It must derive from FToroDatabaseEntry."));
		}
		return EDataValidationResult::Invalid;
	}

	bool bHasErrors = false;
	for (const TPair<FGameplayTag, FInstancedStruct>& Entry : Entries)
	{
		if (!IsValidKey(Entry.Key))
		{
			bHasErrors = true;
			Context.AddError(FText::Format(
				LOCTEXT("DatabaseKeyInvalid", "Key {0} is not valid in this database. It should be a sub-tag of {1}."),
				FText::FromString(Entry.Key.ToString()), FText::FromName(RootTag.GetTagName())
			));
		}

		if (!Entry.Value.IsValid()
			|| !Entry.Value.GetScriptStruct()->IsChildOf(RootStruct)
			|| !Entry.Value.GetPtr<FToroDatabaseEntry>())
		{
			bHasErrors = true;
			Context.AddError(FText::Format(
				LOCTEXT("DatabaseValueInvalid", "Key {0} has an invalid value in this database. Its struct type must be {1} or a descendant."),
				FText::FromString(Entry.Key.ToString()), FText::FromName(RootStruct->GetFName())
			));
		}
		else if (!Entry.Value.Get<FToroDatabaseEntry>().IsValid())
		{
			bHasErrors = true;
			Context.AddError(FText::Format(
				LOCTEXT("DatabaseDataInvalid", "Key {0} has invalid payload data in this database."),
				FText::FromString(Entry.Key.ToString())
			));
		}
	}

	return bHasErrors ? EDataValidationResult::Invalid : SuperResult;
}

void UToroDatabase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UToroDatabase, Entries)
		&& RootStruct && RootStruct->IsChildOf<FToroDatabaseEntry>())
	{
		for (TPair<FGameplayTag, FInstancedStruct>& Entry : Entries)
		{
			if (!Entry.Value.IsValid())
			{
				Entry.Value.InitializeAs(RootStruct);
			}
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
#undef LOCTEXT_NAMESPACE