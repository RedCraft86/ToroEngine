// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SaveGameContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

class USaveGameBehavior;

/**
 * Game-instance owner of global and slotted persistence, slot selection, and save-behavior registration.
 * Operations are synchronous and must run on the game thread; persistence failures display an error dialog.
 * Global data loads during initialization and saves during deinitialization; slot saves are explicit.
 * Registered behaviors are currently tracked only; callers remain responsible for transferring live actor state.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class USaveGameSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/** Requests the remembered active slot without changing the selection. */
	static constexpr TOptional<uint8> UnsetSlot = TOptional<uint8>();

	/** Resolves the context's game-instance save subsystem, or nullptr when unavailable. */
	[[nodiscard]] TOROSYSTEMS_API static USaveGameSubsystem* Get(const UObject* Context);

	/** Loads global state; missing files and failed loads preserve current data. */
	TOROSYSTEMS_API ESaveContainerResult LoadGlobal() const;

	/** Persists global flags and the remembered slot selection. */
	TOROSYSTEMS_API ESaveContainerResult SaveGlobal() const;

	/** Returns the remembered valid slot, falling back to DefaultSlot when unavailable. */
	TOROSYSTEMS_API uint8 GetActiveSlot() const;

	/**
	 * Deletes the resolved slot and recovery files. Invalid explicit slots resolve to DefaultSlot.
	 * Deleting the active slot initializes fresh progression and reloads the current gameplay level.
	 */
	TOROSYSTEMS_API ESaveContainerResult DeleteSlot(uint8 InSlot);

	/**
	 * Loads progression, or initializes a fresh playthrough and returns MissingFile for an empty slot.
	 * Accepted loads reload the current gameplay level, including same-slot loads; other levels are not changed.
	 * An explicit request selects and persists its resolved slot before loading; an unset request retains selection.
	 */
	TOROSYSTEMS_API ESaveContainerResult LoadSlot(TOptional<uint8> InSlot);

	/**
	 * Writes current container data without capturing registered actors automatically.
	 * An explicit request selects and persists its resolved slot before saving; invalid requests use DefaultSlot.
	 * @param JsonProc Synchronous callback editing slot metadata; do not reenter save/load operations.
	 */
	TOROSYSTEMS_API ESaveContainerResult SaveSlot(TOptional<uint8> InSlot, TFunctionRef<void(FJsonObject&)> JsonProc);

	/** Reads metadata only, preserving OutHeader on failure; invalid explicit slots resolve to DefaultSlot. */
	TOROSYSTEMS_API ESaveContainerResult GetSlotHeader(uint8 InSlot, FSlottedSaveContainer::FHeader& OutHeader);

	/** Tracks a valid behavior weakly; repeated registration has no additional effect. */
	TOROSYSTEMS_API void RegisterSaveBehavior(USaveGameBehavior* Behavior);

	/** Removes a behavior from tracking without changing its saved record. */
	TOROSYSTEMS_API void UnregisterSaveBehavior(USaveGameBehavior* Behavior);

	/** Borrows global state; nullptr after subsystem teardown. Ownership stays with the subsystem. */
	[[nodiscard]] FGlobalSaveContainer* GetGlobal() const { return GlobalContainer.Get(); }

	/** Borrows playthrough state; nullptr after subsystem teardown. Direct operations bypass subsystem coordination. */
	[[nodiscard]] FSlottedSaveContainer* GetSlotted() const { return SlottedContainer.Get(); }

private:

	TUniquePtr<FGlobalSaveContainer> GlobalContainer = MakeUnique<FGlobalSaveContainer>();
	TUniquePtr<FSlottedSaveContainer> SlottedContainer = MakeUnique<FSlottedSaveContainer>();
	TSet<TWeakObjectPtr<USaveGameBehavior>> SaveBehaviors;
	bool bSlotChanged;

	uint8 ResolveSlot(TOptional<uint8> RequestedSlot, bool bSetAsActive = false);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
