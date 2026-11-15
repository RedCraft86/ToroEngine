// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "GameFlags/GameFlagsSubsystem.h"
#include "GameFlags/GameFlagTags.h"
#include "DataTypes/RequestSet.h"
#include "Utilities/ToroLogging.h"
#include "Utilities/ToroUtilities.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UGameFlagsSubsystem* UGameFlagsSubsystem::Get(const UObject* Context)
{
	const UWorld* World = ToroEngine::GetWorld(Context);
	const UGameInstance* GI = IsValid(World) ? World->GetGameInstance() : nullptr;
	return IsValid(GI) ? GI->GetSubsystem<UGameFlagsSubsystem>() : nullptr;
}

bool UGameFlagsSubsystem::HasTransientFlag(FGameplayTag Flag) const
{
	return GameFlagTags::IsTransient(Flag) && ActiveTransientFlags.Contains(Flag);
}

void UGameFlagsSubsystem::AddTransientFlag(const UObject* Source, FGameplayTag Flag)
{
	if (!IsValid(Source))
	{
		return;
	}

	if (!GameFlagTags::IsTransient(Flag))
	{
		TORO_REPORT_WARNING(
			TEXT("Attempting to add non-transient flag '%s' using AddTransientFlag()"),
			*Flag.ToString()
		);
		return;
	}

	if (!TransientFlags.Contains(Flag))
	{
		const TSharedRef<FRequestSet> Requests = FRequestSet::Make();
		Requests->OnChanged().AddUObject(this, &UGameFlagsSubsystem::OnTransientChanged, Flag);
		TransientFlags.Add(Flag, Requests);
	}

	TransientFlags[Flag]->Add(Source);
}

void UGameFlagsSubsystem::ClearTransientFlag(const UObject* Source, FGameplayTag Flag)
{
	if (!IsValid(Source) || !GameFlagTags::IsTransient(Flag))
	{
		return;
	}

	if (const TSharedRef<FRequestSet>* Found = TransientFlags.Find(Flag))
	{
		(*Found)->Remove(Source);
	}
}

void UGameFlagsSubsystem::ClearAllTransientFlags(const UObject* Source)
{
	if (IsValid(Source))
	{
		TArray<TSharedRef<FRequestSet>> Requests;
		TransientFlags.GenerateValueArray(Requests);
		for (const TSharedRef<FRequestSet>& Request : Requests)
		{
			Request->Remove(Source);
		}
	}
}

bool UGameFlagsSubsystem::HasPersistentFlag(FGameplayTag Flag) const
{
	if (!GameFlagTags::IsPersistent(Flag))
	{
		return false;
	}

	return false;
	// TODO: Implement after save rework
}

void UGameFlagsSubsystem::AddPersistentFlag(EPersistentFlagScope Scope, FGameplayTag Flag)
{
	if (!GameFlagTags::IsPersistent(Flag))
	{
		TORO_REPORT_WARNING(
			TEXT("Attempting to add non-persistent flag '%s' using AddPersistentFlag()"),
			*Flag.ToString()
		);
		return;
	}

	// TODO: Implement after save rework
}

void UGameFlagsSubsystem::ClearPersistentFlag(EPersistentFlagScope Scope, FGameplayTag Flag)
{
	if (!GameFlagTags::IsPersistent(Flag))
	{
		return;
	}

	// TODO: Implement after save rework
}

void UGameFlagsSubsystem::OnTransientChanged(FGameplayTag Flag)
{
	const TSharedRef<FRequestSet>* Requests = TransientFlags.Find(Flag);
	const bool bEnabled = Requests && !(*Requests)->IsEmpty();
	if (ActiveTransientFlags.Contains(Flag) == bEnabled)
	{
		return;
	}

	if (bEnabled)
	{
		ActiveTransientFlags.Add(Flag);
		TransientAddedEvent.Broadcast(Flag);
	}
	else
	{
		ActiveTransientFlags.Remove(Flag);
		TransientClearedEvent.Broadcast(Flag);
	}
}
