// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFlagManager.generated.h"

/** Selects which store to read or change. Each store tracks its flags independently. */
UENUM(BlueprintType)
enum class EGameFlagScope : uint8
{
	/** In-memory flags that last for the lifetime of the game instance. */
	Transient,

	/** Flags shared across save slots. */
	GlobalSave,

	/** Flags stored in the current gameplay save container. */
	SlotSave
};

/**
 * Manages gameplay flags keyed by exact gameplay tag within each scope.
 * Changes to saved scopes update the in-memory containers and reach disk when the corresponding container is saved.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class UGameFlagManager final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnFlagStateChanged, FGameplayTag, bool /** bRemoved */, EGameFlagScope);

	/** Returns the game instance's flag manager, or nullptr if the context cannot resolve one. */
	[[nodiscard]] TOROSYSTEMS_API static UGameFlagManager* Get(const UObject* Context);

	/** Checks for an exact tag in the selected scope. Returns false for an invalid tag or unavailable save container. */
	UFUNCTION(BlueprintCallable, Category = GameFlags)
	[[nodiscard]] TOROSYSTEMS_API bool HasGameFlag(FGameplayTag Flag,
		EGameFlagScope Scope = EGameFlagScope::Transient) const;

	/** Sets the flag if absent. Invalid tags and unavailable save containers are ignored. */
	UFUNCTION(BlueprintCallable, Category = GameFlags)
	TOROSYSTEMS_API void SetGameFlag(FGameplayTag Flag,
		EGameFlagScope Scope = EGameFlagScope::Transient);

	/** Clears the flag if present. Invalid tags and unavailable save containers are ignored. */
	UFUNCTION(BlueprintCallable, Category = GameFlags)
	TOROSYSTEMS_API void ClearGameFlag(FGameplayTag Flag,
		EGameFlagScope Scope = EGameFlagScope::Transient);

	/**
	 * Registers listeners for changes made through SetGameFlag and ClearGameFlag.
	 * Each callback receives the flag, whether it was removed, and the changed scope.
	 */
	[[nodiscard]] FORCEINLINE FOnFlagStateChanged::RegistrationType& OnFlagStateChanged()
	{
		return OnFlagStateChangedEvent;
	}

private:

	FGameplayTagContainer TransientFlags;
	FOnFlagStateChanged OnFlagStateChangedEvent;
};
