// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Objects/ToroDatabase.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "UObject/UnrealType.h"
#endif

bool UToroDatabase::DoesKeyExist(const FGameplayTag& Key) const
{
	return Key.IsValid() && Entries.Contains(Key);
}

FInstancedStruct UToroDatabase::GetValue(const FGameplayTag& Key) const
{
	if (!Key.IsValid())
	{
		return FInstancedStruct();
	}

	const FInstancedStruct* FoundValue = Entries.Find(Key);
	return FoundValue && FoundValue->IsValid() ? *FoundValue : FInstancedStruct();
}

bool UToroDatabase::IsValidKey(const FGameplayTag& Key) const
{
	return Key.IsValid() && Key != RootTag && Key.MatchesTag(RootTag);
}

const TMap<FWrappedGameplayTag, FInstancedStruct>& UToroDatabase::GetEntries() const
{
	return Entries;
}

UToroDatabase::UToroDatabase()
{
}

UToroDatabase::UToroDatabase(const FGameplayTag& TagType, const UScriptStruct* StructType)
	: RootTag(TagType), RootStruct(StructType)
{
	ensureAlwaysMsgf(StructType && StructType->IsChildOf<FToroDatabaseEntry>(),
		TEXT("StructType (%s) must derive from FToroDatabaseEntry"), *GetNameSafe(StructType));
}

#if WITH_EDITOR
#define LOCTEXT_NAMESPACE "ToroDatabase"
EDataValidationResult UToroDatabase::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult SuperResult = Super::IsDataValid(Context);
	bool bHasIssues = false;

	const bool bValidRootStruct = RootStruct && RootStruct->IsChildOf<FToroDatabaseEntry>();
	if (!bValidRootStruct)
	{
		bHasIssues = true;
		Context.AddError(LOCTEXT("InvalidRootStruct", "Invalid RootStruct: Configure a struct derived from FToroDatabaseEntry."));
	}

	if (!RootTag.IsValid())
	{
		bHasIssues = true;
		Context.AddError(LOCTEXT("InvalidRootTag", "Invalid RootTag: Configure a valid root gameplay tag in the native database subclass."));
	}

	for (const TPair<FWrappedGameplayTag, FInstancedStruct>& Entry : Entries)
	{
		const FText KeyText = FText::FromString(Entry.Key.ToString());
		if (!IsValidKey(Entry.Key))
		{
			bHasIssues = true;
			Context.AddError(FText::Format(LOCTEXT("InvalidKey", "Invalid Key: {0}"), KeyText));
		}

		if (!Entry.Value.IsValid())
		{
			bHasIssues = true;
			Context.AddError(FText::Format(LOCTEXT("MissingStruct", "Struct Not Set: {0}"), KeyText));
			continue;
		}

		const FToroDatabaseEntry* ValuePtr = Entry.Value.GetPtr<FToroDatabaseEntry>();
		if (!ValuePtr || (bValidRootStruct && !Entry.Value.GetScriptStruct()->IsChildOf(RootStruct)))
		{
			bHasIssues = true;
			Context.AddError(FText::Format(LOCTEXT("IncompatibleType", "Incompatible Type: {0}"), KeyText));
		}
		else if (!ValuePtr->IsValid())
		{
			bHasIssues = true;
			Context.AddError(FText::Format(LOCTEXT("InvalidData", "Invalid Data: {0}"), KeyText));
		}
	}

	return CombineDataValidationResults(SuperResult, bHasIssues ? EDataValidationResult::Invalid : EDataValidationResult::Valid);
}

void UToroDatabase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UToroDatabase, Entries)
		&& RootStruct && RootStruct->IsChildOf<FToroDatabaseEntry>())
	{
		for (TPair<FWrappedGameplayTag, FInstancedStruct>& Entry : Entries)
		{
			if (!Entry.Value.IsValid())
			{
				Entry.Value.InitializeAs(RootStruct);
			}
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#undef LOCTEXT_NAMESPACE
#endif
