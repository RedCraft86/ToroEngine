// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SaveGameContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

/**
 * Game-instance owner of global and slotted save containers for native and Blueprint callers.
 * Initialization creates containers and reads slot metadata; binary payloads are loaded explicitly.
 * Deinitialization saves global data and the selected slot when its container is marked in use.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class USaveGameSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	USaveGameSubsystem();

	/** Returns the context world's save subsystem, or nullptr if its world or game instance is unavailable. */
	TOROSYSTEMS_API static USaveGameSubsystem* Get(const UObject* Context);

	/** Loads global flags, or an empty set for a missing file; returns false on persistence failure. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = SaveSystem)
	TOROSYSTEMS_API bool LoadGlobal() const;

	/** Saves global flags; returns false on persistence failure if fatal reporting returns in PIE. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = SaveSystem)
	TOROSYSTEMS_API bool SaveGlobal() const;

	/**
	 * Loads slot metadata and payload, initializing empty data if its binary file is missing.
	 * Broadcasts OnLoadedDifferentSlot only after success when the selected slot number changes.
	 * @param SlotNum Slot in 1 through FSlottedSaveContainer::MaxNumSlots, or zero for the current selection.
	 * @return False for an invalid selection or persistence failure if fatal reporting returns.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = SaveSystem)
	TOROSYSTEMS_API bool LoadSlotted(uint8 SlotNum) const;

	/**
	 * Saves the current payload and standard metadata without a custom metadata callback.
	 * Does not broadcast OnLoadedDifferentSlot, even when saving to a different slot.
	 * @param SlotNum Destination in 1 through FSlottedSaveContainer::MaxNumSlots, or zero for the current selection.
	 * @return True only if binary and metadata persistence both succeed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = SaveSystem)
	TOROSYSTEMS_API bool SaveSlotted(uint8 SlotNum) const;

	/**
	 * Saves the current payload and lets native callers customize slot metadata.
	 * Binary and metadata writes are separate; metadata failure does not undo a completed binary write.
	 * Does not broadcast OnLoadedDifferentSlot.
	 * @param SlotNum Destination in 1 through FSlottedSaveContainer::MaxNumSlots, or zero for the current selection.
	 * @param ApplyMetadata Synchronous callback editing slot JSON after standard fields are populated; must not reenter container operations.
	 * @return True only if binary and metadata persistence both succeed.
	 */
	TOROSYSTEMS_API bool SaveSlotted(uint8 SlotNum, TFunctionRef<void(FJsonObject&)> ApplyMetadata) const;

	/** Provides mutable global state; valid only during the initialized subsystem lifetime. */
	FGlobalSaveContainer& GetGlobal() const { return *GlobalContainer.Get(); }

	/** Provides mutable slot state; valid only during the initialized subsystem lifetime. */
	FSlottedSaveContainer& GetSlotted() const { return *SlottedContainer.Get(); }

	/**
	 * Native notification after LoadSlotted successfully selects a different slot number.
	 * Reloading the selected slot and saving to another slot do not broadcast this notification.
	 */
	FSimpleMulticastDelegate OnLoadedDifferentSlot;

private:

	/** Global state owned from initialization until deinitialization. */
	TUniquePtr<FGlobalSaveContainer> GlobalContainer;

	/** Slot state and metadata owned from initialization until deinitialization. */
	TUniquePtr<FSlottedSaveContainer> SlottedContainer;

	virtual void Deinitialize() override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
