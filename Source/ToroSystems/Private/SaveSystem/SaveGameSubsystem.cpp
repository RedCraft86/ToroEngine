// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "SaveSystem/SaveGameSubsystem.h"
#include "SaveSystem/SaveGameBehavior.h"
#include "Libraries/ToroWorldLibrary.h"
#include "DataTypes/ToroLevelInfo.h"
#include "Utilities/ToroUtilities.h"
#include "Misc/MessageDialog.h"

namespace
{
	ESaveContainerResult ProcessGlobalSaveResult(ESaveContainerResult InResult)
	{
		if (InResult == ESaveContainerResult::Success
			|| InResult == ESaveContainerResult::MissingFile)
		{
			return InResult;
		}

		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
			FText::Format(INVTEXT("Error Result: {0}"), static_cast<int32>(InResult)),
			INVTEXT("Global Container Error"));

		return InResult;
	}

	ESaveContainerResult ProcessSlottedSaveResult(ESaveContainerResult InResult, uint8 Slot)
	{
		if (InResult == ESaveContainerResult::Success
			|| InResult == ESaveContainerResult::MissingFile)
		{
			return InResult;
		}

		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
			FText::Format(INVTEXT("Slot: {0}\nError Result: {1}"), static_cast<int32>(Slot), static_cast<int32>(InResult)),
			INVTEXT("Slotted Container Error"));

		return InResult;
	}
}

USaveGameSubsystem* USaveGameSubsystem::Get(const UObject* Context)
{
	const UWorld* World = ToroEngine::GetWorld(Context);
	const UGameInstance* GI = IsValid(World) ? World->GetGameInstance() : nullptr;
	return IsValid(GI) ? GI->GetSubsystem<USaveGameSubsystem>() : nullptr;
}

ESaveContainerResult USaveGameSubsystem::LoadGlobal() const
{
	if (!GlobalContainer.IsValid())
	{
		return ProcessGlobalSaveResult(ESaveContainerResult::Unknown);
	}

	return ProcessGlobalSaveResult(GlobalContainer->Load());
}

ESaveContainerResult USaveGameSubsystem::SaveGlobal() const
{
	if (!GlobalContainer.IsValid())
	{
		return ProcessGlobalSaveResult(ESaveContainerResult::Unknown);
	}

	return ProcessGlobalSaveResult(GlobalContainer->Save());
}

uint8 USaveGameSubsystem::GetActiveSlot() const
{
	return GlobalContainer.IsValid()
		? GlobalContainer->GetActiveSlot()
		: FSlottedSaveContainer::DefaultSlot;
}

ESaveContainerResult USaveGameSubsystem::DeleteSlot(uint8 InSlot)
{
	const uint8 Slot = ResolveSlot(InSlot);
	const ESaveContainerResult Result = ProcessSlottedSaveResult(FSlottedSaveContainer::Delete(Slot), Slot);

	if (Result == ESaveContainerResult::Success && Slot == GetActiveSlot())
	{
		const ESaveContainerResult LoadResult = LoadSlot(Slot);
		return (LoadResult == ESaveContainerResult::Success || LoadResult == ESaveContainerResult::MissingFile)
			? Result : LoadResult;
	}

	return Result;
}

ESaveContainerResult USaveGameSubsystem::LoadSlot(TOptional<uint8> InSlot)
{
	const uint8 Slot = ResolveSlot(InSlot, true);
	if (!SlottedContainer.IsValid())
	{
		return ProcessSlottedSaveResult(ESaveContainerResult::Unknown, Slot);
	}

	const ESaveContainerResult Result = ProcessSlottedSaveResult(SlottedContainer->Load(Slot), Slot);

	const FToroLevelInfo* LevelInfo = FToroLevelInfo::Get(this);
	if ((Result == ESaveContainerResult::Success || Result == ESaveContainerResult::MissingFile)
		&& LevelInfo && LevelInfo->LevelType == LevelTypeTags::Gameplay)
	{
		UToroWorldLibrary::ReloadLevel(this);
	}
	return Result;
}

ESaveContainerResult USaveGameSubsystem::SaveSlot(TOptional<uint8> InSlot, TFunctionRef<void(FJsonObject&)> JsonProc)
{
	const uint8 Slot = ResolveSlot(InSlot, true);
	if (!SlottedContainer.IsValid())
	{
		return ProcessSlottedSaveResult(ESaveContainerResult::Unknown, Slot);
	}

	return ProcessSlottedSaveResult(SlottedContainer->Save(Slot, JsonProc), Slot);
}

ESaveContainerResult USaveGameSubsystem::GetSlotHeader(uint8 InSlot, FSlottedSaveContainer::FHeader& OutHeader)
{
	const uint8 Slot = ResolveSlot(InSlot);
	return ProcessSlottedSaveResult(FSlottedSaveContainer::LoadSlotHeader(Slot, OutHeader), Slot);
}

void USaveGameSubsystem::RegisterSaveBehavior(USaveGameBehavior* Behavior)
{
	if (IsValid(Behavior))
	{
		SaveBehaviors.Add(Behavior);
	}
}

void USaveGameSubsystem::UnregisterSaveBehavior(USaveGameBehavior* Behavior)
{
	SaveBehaviors.Remove(Behavior);
}

uint8 USaveGameSubsystem::ResolveSlot(TOptional<uint8> RequestedSlot, bool bSetAsActive)
{
	bSlotChanged = false;
	if (!RequestedSlot.IsSet())
	{
		return GetActiveSlot();
	}

	const uint8 Slot = FSlottedSaveContainer::IsValidSlot(RequestedSlot.GetValue())
		? RequestedSlot.GetValue()
		: FSlottedSaveContainer::DefaultSlot;

	if (bSetAsActive && GlobalContainer.IsValid())
	{
		bSlotChanged = GlobalContainer->SetActiveSlot(Slot);
		SaveGlobal();
	}

	return Slot;
}

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadGlobal();
}

void USaveGameSubsystem::Deinitialize()
{
	SaveGlobal();
	GlobalContainer.Reset();
	SlottedContainer.Reset();
	SaveBehaviors.Empty();
	bSlotChanged = false;
	Super::Deinitialize();
}
