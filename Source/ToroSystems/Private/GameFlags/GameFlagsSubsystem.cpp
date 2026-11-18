// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "GameFlags/GameFlagsSubsystem.h"
#include "GameFlags/GameFlagTags.h"
#include "DataTypes/RequestSet.h"
#include "Utilities/ToroLogging.h"
#include "Utilities/ToroUtilities.h"
#include "SaveSystem/SaveGameSubsystem.h"
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
	if (!GameFlagTags::IsTransient(Flag))
	{
		TORO_REPORT_WARNING(
			TEXT("Attempting to check non-transient flag '%s' using HasTransientFlag()"),
			*Flag.ToString()
		);
		return false;
	}

	return ActiveTransientFlags.Contains(Flag);
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
	if (!IsValid(Source))
	{
		return;
	}

	if (!GameFlagTags::IsTransient(Flag))
	{
		TORO_REPORT_WARNING(
			TEXT("Attempting to clear non-transient flag '%s' using ClearTransientFlag()"),
			*Flag.ToString()
		);
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
		TORO_REPORT_WARNING(
			TEXT("Attempting to check non-persistent flag '%s' using HasPersistentFlag()"),
			*Flag.ToString()
		);
		return false;
	}

	if (!SaveSubsystem.IsValid())
	{
		TORO_REPORT_ERROR(TEXT("HasPersistentFlag requires a valid save game subsystem"));
		return false;
	}

	const FGlobalSaveContainer* Global = SaveSubsystem->GetGlobal();
	const FSlottedSaveContainer* Slotted = SaveSubsystem->GetSlotted();
	return (Global && Global->HasFlag(Flag)) || (Slotted && Slotted->HasFlag(Flag));
}

void UGameFlagsSubsystem::AddPersistentFlag(EPersistentFlagScope Scope, FGameplayTag Flag) const
{
	if (!GameFlagTags::IsPersistent(Flag))
	{
		TORO_REPORT_WARNING(
			TEXT("Attempting to add non-persistent flag '%s' using AddPersistentFlag()"),
			*Flag.ToString()
		);
		return;
	}

	if (!SaveSubsystem.IsValid())
	{
		TORO_REPORT_ERROR(TEXT("AddPersistentFlag requires a valid save game subsystem"));
		return;
	}

	const bool bWasPresent = HasPersistentFlag(Flag);
	switch (Scope)
	{
	case EPersistentFlagScope::Global:
		if (FGlobalSaveContainer* Container = SaveSubsystem->GetGlobal())
		{
			Container->AddFlag(Flag);
		}
		break;
	case EPersistentFlagScope::Slot:
		if (FSlottedSaveContainer* Container = SaveSubsystem->GetSlotted())
		{
			Container->AddFlag(Flag);
		}
		break;
	}

	if (!bWasPresent && HasPersistentFlag(Flag))
	{
		PersistentAddedEvent.Broadcast(Flag);
	}
}

void UGameFlagsSubsystem::ClearPersistentFlag(EPersistentFlagScope Scope, FGameplayTag Flag) const
{
	if (!GameFlagTags::IsPersistent(Flag))
	{
		TORO_REPORT_WARNING(
			TEXT("Attempting to clear non-persistent flag '%s' using ClearPersistentFlag()"),
			*Flag.ToString()
		);
		return;
	}

	if (!SaveSubsystem.IsValid())
	{
		TORO_REPORT_ERROR(TEXT("ClearPersistentFlag requires a valid save game subsystem"));
		return;
	}

	const bool bWasPresent = HasPersistentFlag(Flag);
	switch (Scope)
	{
	case EPersistentFlagScope::Global:
		if (FGlobalSaveContainer* Container = SaveSubsystem->GetGlobal())
		{
			Container->RemoveFlag(Flag);
		}
		break;
	case EPersistentFlagScope::Slot:
		if (FSlottedSaveContainer* Container = SaveSubsystem->GetSlotted())
		{
			Container->RemoveFlag(Flag);
		}
		break;
	}

	if (bWasPresent && !HasPersistentFlag(Flag))
	{
		PersistentClearedEvent.Broadcast(Flag);
	}
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

void UGameFlagsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// Fix: Initialize persistence before caching it; game-instance initialization needs no world.
	SaveSubsystem = Collection.InitializeDependency<USaveGameSubsystem>();
}

void UGameFlagsSubsystem::Deinitialize()
{
	SaveSubsystem.Reset();
	Super::Deinitialize();
}
