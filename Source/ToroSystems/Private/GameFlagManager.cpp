// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "GameFlagManager.h"
#include "SaveSystem/SaveGameSubsystem.h"
#include "Utilities/ToroUtilities.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UGameFlagManager* UGameFlagManager::Get(const UObject* Context)
{
	const UWorld* World = ToroEngine::GetWorld(Context);
	const UGameInstance* GI = IsValid(World) ? World->GetGameInstance() : nullptr;
	return IsValid(GI) ? GI->GetSubsystem<UGameFlagManager>() : nullptr;
}

bool UGameFlagManager::HasGameFlag(FGameplayTag Flag, EGameFlagScope Scope) const
{
	if (!Flag.IsValid())
	{
		return false;
	}

	if (Scope == EGameFlagScope::Transient)
	{
		return TransientFlags.HasTagExact(Flag);
	}

	if (const USaveGameSubsystem* SaveSystem = USaveGameSubsystem::Get(this))
	{
		switch (Scope)
		{
		case EGameFlagScope::GlobalSave:
			if (const FGlobalSaveContainer* Global = SaveSystem->GetGlobal())
			{
				return Global->HasFlag(Flag);
			}
			return false;

		case EGameFlagScope::SlotSave:
			if (const FSlottedSaveContainer* Slotted = SaveSystem->GetSlotted())
			{
				return Slotted->HasFlag(Flag);
			}
			return false;

		default:
			return false;
		}
	}

	return false;
}

void UGameFlagManager::SetGameFlag(FGameplayTag Flag, EGameFlagScope Scope)
{
	if (!Flag.IsValid() || HasGameFlag(Flag, Scope))
	{
		return;
	}

	if (Scope == EGameFlagScope::Transient)
	{
		TransientFlags.AddTag(Flag);
		OnFlagStateChangedEvent.Broadcast(Flag, false, Scope);
		return;
	}

	if (const USaveGameSubsystem* SaveSystem = USaveGameSubsystem::Get(this))
	{
		switch (Scope)
		{
		case EGameFlagScope::GlobalSave:
			if (FGlobalSaveContainer* Global = SaveSystem->GetGlobal())
			{
				Global->AddFlag(Flag);
				OnFlagStateChangedEvent.Broadcast(Flag, false, Scope);
			}
			break;

		case EGameFlagScope::SlotSave:
			if (FSlottedSaveContainer* Slotted = SaveSystem->GetSlotted())
			{
				Slotted->AddFlag(Flag);
				OnFlagStateChangedEvent.Broadcast(Flag, false, Scope);
			}
			break;

		default:
			break;
		}
	}
}

void UGameFlagManager::ClearGameFlag(FGameplayTag Flag, EGameFlagScope Scope)
{
	if (!Flag.IsValid() || !HasGameFlag(Flag, Scope))
	{
		return;
	}

	if (Scope == EGameFlagScope::Transient)
	{
		TransientFlags.RemoveTag(Flag);
		OnFlagStateChangedEvent.Broadcast(Flag, true, Scope);
		return;
	}

	if (const USaveGameSubsystem* SaveSystem = USaveGameSubsystem::Get(this))
	{
		switch (Scope)
		{
		case EGameFlagScope::GlobalSave:
			if (FGlobalSaveContainer* Global = SaveSystem->GetGlobal())
			{
				Global->RemoveFlag(Flag);
				OnFlagStateChangedEvent.Broadcast(Flag, true, Scope);
			}
			break;

		case EGameFlagScope::SlotSave:
			if (FSlottedSaveContainer* Slotted = SaveSystem->GetSlotted())
			{
				Slotted->RemoveFlag(Flag);
				OnFlagStateChangedEvent.Broadcast(Flag, true, Scope);
			}
			break;

		default:
			break;
		}
	}
}
