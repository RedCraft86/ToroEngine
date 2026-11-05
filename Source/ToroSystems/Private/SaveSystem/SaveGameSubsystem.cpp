// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "SaveSystem/SaveGameSubsystem.h"
#include "Utilities/ToroUtilities.h"

USaveGameSubsystem::USaveGameSubsystem()
{
}

USaveGameSubsystem* USaveGameSubsystem::Get(const UObject* Context)
{
	const UWorld* World = ToroEngine::GetWorld(Context);
	const UGameInstance* GI = IsValid(World) ? World->GetGameInstance() : nullptr;
	return IsValid(GI) ? GI->GetSubsystem<USaveGameSubsystem>() : nullptr;
}

bool USaveGameSubsystem::LoadGlobal() const
{
	return GlobalContainer->LoadContainer();
}

bool USaveGameSubsystem::SaveGlobal() const
{
	return GlobalContainer->SaveContainer();
}

bool USaveGameSubsystem::LoadSlotted(uint8 SlotNum) const
{
	const uint8 OldSlot = SlottedContainer->GetActiveSlot();
	if (SlottedContainer->LoadContainer(SlotNum))
	{
		if (OldSlot != SlottedContainer->GetActiveSlot())
		{
			OnLoadedDifferentSlotEvent.Broadcast();
		}

		return true;
	}

	return false;
}

bool USaveGameSubsystem::SaveSlotted(uint8 SlotNum) const
{
	return SaveSlotted(SlotNum, [](FJsonObject&) -> void {});
}

bool USaveGameSubsystem::SaveSlotted(uint8 SlotNum, TFunctionRef<void(FJsonObject&)> ApplyMetadata) const
{
	return SlottedContainer->SaveContainer(SlotNum, ApplyMetadata);
}

void USaveGameSubsystem::Deinitialize()
{
	Super::Deinitialize();

	SaveGlobal();
	if (SlottedContainer->IsValidContainer())
	{
		SaveSlotted(0); // Save the active slot
	}

	GlobalContainer.Reset();
	SlottedContainer.Reset();
}

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GlobalContainer = MakeUnique<FGlobalSaveContainer>();
	SlottedContainer = MakeUnique<FSlottedSaveContainer>();
}
