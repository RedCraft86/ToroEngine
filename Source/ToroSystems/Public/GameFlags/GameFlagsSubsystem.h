// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFlagsSubsystem.generated.h"

class FRequestSet;

/** Selects the save container that owns a persistent flag. */
UENUM(BlueprintType)
enum class EPersistentFlagScope : uint8
{
	/** Shared across save slots. */
	Global,

	/** Stored with the current save slot. */
	Slot
};

/**
 * Provides source-owned transient flags and access to persistent flags in global and slot saves.
 * Transient flags use exact tag membership and remain enabled while at least one source requests them.
 * Sources are held weakly; release requests explicitly when they end, including on pooled widget deactivation.
 * Destroyed sources are pruned after garbage collection. Access and delegate callbacks occur on the game thread.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class UGameFlagsSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnFlagChanged, FGameplayTag);

	/**
	 * Retrieves the game flags subsystem from the context's world on the game thread.
	 * @return The subsystem, or nullptr if the context cannot resolve a valid world, game instance, or subsystem.
	 */
	[[nodiscard]] TOROSYSTEMS_API static UGameFlagsSubsystem* Get(const UObject* Context);

	/**
	 * Checks whether the exact transient flag currently has an active request.
	 * @param Flag Tag under Flag.Transient; other tags return false.
	 */
	UFUNCTION(BlueprintCallable, Category = GameFlags)
	[[nodiscard]] TOROSYSTEMS_API bool HasTransientFlag(
		UPARAM(meta = (Categories = "Flag.Transient")) FGameplayTag Flag) const;

	/**
	 * Adds a source's request for a transient flag. Repeating the same source/tag pair has no effect.
	 * @param Source Valid object identifying the request; one source can request multiple flags.
	 * @param Flag Tag under Flag.Transient to enable. Tags outside that hierarchy are rejected.
	 */
	UFUNCTION(BlueprintCallable, Category = GameFlags)
	TOROSYSTEMS_API void AddTransientFlag(const UObject* Source,
		UPARAM(meta = (Categories = "Flag.Transient")) FGameplayTag Flag);

	/**
	 * Removes only this source's request; the flag stays enabled while other requests remain.
	 * @param Source Valid object whose request should be removed. Missing requests have no effect.
	 * @param Flag Tag under Flag.Transient whose request should be removed.
	 */
	UFUNCTION(BlueprintCallable, Category = GameFlags)
	TOROSYSTEMS_API void ClearTransientFlag(const UObject* Source,
		UPARAM(meta = (Categories = "Flag.Transient")) FGameplayTag Flag);

	/**
	 * Removes all flag requests associated with this source.
	 * @param Source Valid object whose requests should be removed. Missing requests have no effect.
	 */
	UFUNCTION(BlueprintCallable, Category = GameFlags)
	TOROSYSTEMS_API void ClearAllTransientFlags(const UObject* Source);

	/**
	 * Checks whether a persistent flag is present in the global save or current slot save.
	 * @param Flag Tag under Flag.Persistent; other tags return false.
	 */
	UFUNCTION(BlueprintCallable, Category = GameFlags)
	[[nodiscard]] TOROSYSTEMS_API bool HasPersistentFlag(
		UPARAM(meta = (Categories = "Flag.Persistent")) FGameplayTag Flag) const;

	/**
	 * Sets a persistent flag in the selected save container.
	 * @param Scope Whether to store the flag globally or in the current save slot.
	 * @param Flag Tag under Flag.Persistent to set. Tags outside that hierarchy are rejected.
	 */
	UFUNCTION(BlueprintCallable, Category = GameFlags)
	TOROSYSTEMS_API void AddPersistentFlag(EPersistentFlagScope Scope,
		UPARAM(meta = (Categories = "Flag.Persistent")) FGameplayTag Flag);

	/**
	 * Removes a persistent flag from the selected save container.
	 * @param Scope Container to clear; a copy in the other scope is unaffected.
	 * @param Flag Tag under Flag.Persistent to remove. Missing flags have no effect.
	 */
	UFUNCTION(BlueprintCallable, Category = GameFlags)
	TOROSYSTEMS_API void ClearPersistentFlag(EPersistentFlagScope Scope,
		UPARAM(meta = (Categories = "Flag.Persistent")) FGameplayTag Flag);

	/**
	 * Provides subscription access to transient flag transitions, passing the changed tag.
	 * Notifications occur synchronously on the first request added or the last request removed,
	 * including removal during garbage-collection cleanup. Binding does not replay current state.
	 * @param bCleared True selects disabled notifications; false selects enabled notifications.
	 */
	[[nodiscard]] TOROSYSTEMS_API FOnFlagChanged::RegistrationType& OnTransientFlag(bool bCleared = false)
	{
		return bCleared ? TransientClearedEvent : TransientAddedEvent;
	}

	/**
	 * Provides subscription access to persistent flag changes, passing the changed tag.
	 * Binding does not replay current state; query HasPersistentFlag when initializing a listener.
	 * @param bCleared True selects cleared notifications; false selects added notifications.
	 */
	[[nodiscard]] TOROSYSTEMS_API FOnFlagChanged::RegistrationType& OnPersistentFlag(bool bCleared = false)
	{
		return bCleared ? PersistentClearedEvent : PersistentAddedEvent;
	}

private:

	FOnFlagChanged TransientAddedEvent, TransientClearedEvent;
	FOnFlagChanged PersistentAddedEvent, PersistentClearedEvent;
	TMap<FGameplayTag, TSharedRef<FRequestSet>> TransientFlags;
	TSet<FGameplayTag> ActiveTransientFlags;

	void OnTransientChanged(FGameplayTag Flag);
};
