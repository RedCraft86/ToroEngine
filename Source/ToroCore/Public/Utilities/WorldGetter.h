// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Engine/World.h"
#include "Engine/Engine.h"
#if WITH_EDITOR
#include "Misc/App.h"
#endif

/**
 * Provides best-effort world lookup backed by a shared weak cache when context is unavailable.
 * The cache does not keep its world alive and is not scoped to a particular PIE instance.
 * Fallback results are not guaranteed to belong to the caller's context or PIE instance.
 * All methods require the game thread, including builds where assertions are disabled.
 */
class TOROCORE_API FWorldGetter final
{
	static inline TWeakObjectPtr<UWorld> CachedWorld = nullptr;

public:

	/**
	 * Resolves an optional context, then uses the cache, current play world, or GWorld in order.
	 * Successful resolution updates the cache; failed context resolution preserves it.
	 * Call only from the game thread.
	 * @param Context Optional source of a world. Null, invalid, or unresolved contexts allow fallback.
	 * @return A valid world from lookup or cache, or nullptr when no valid world is available.
	 */
	[[nodiscard]] static UWorld* Get(const UObject* Context = nullptr)
	{
		checkf(IsInGameThread(), TEXT("FWorldGetter::Get(...) should only be called from the GameThread."));

#if WITH_EDITOR
		if (!FApp::IsGame())
		{
			UWorld* MaybeWorld = nullptr;
			if (GEngine)
			{
				if (IsValid(Context))
				{
					MaybeWorld = GEngine->GetWorldFromContextObject(Context, EGetWorldErrorMode::ReturnNull);
				}

				if (!IsValid(MaybeWorld))
				{
					MaybeWorld = GEngine->GetCurrentPlayWorld();
				}
			}

			return IsValid(MaybeWorld) ? MaybeWorld : GWorld.GetReference();
		}
#endif

		if (IsValid(Context) && GEngine)
		{
			SetWorld(GEngine->GetWorldFromContextObject(Context, EGetWorldErrorMode::ReturnNull));
		}

		if (CachedWorld.IsValid())
		{
			return CachedWorld.Get();
		}

		if (GEngine)
		{
			SetWorld(GEngine->GetCurrentPlayWorld());
		}

		if (!CachedWorld.IsValid())
		{
			SetWorld(GWorld.GetReference());
		}

		return CachedWorld.Get();
	}

	/**
	 * Replaces the cached world only when InWorld is valid; null or invalid inputs are ignored.
	 * Called by ToroCoreModule through FCoreUObjectDelegates::PostLoadMapWithWorld.
	 * Call only from the game thread. Use Reset() to explicitly clear the cache.
	 */
	static void SetWorld(UWorld* InWorld)
	{
		checkf(IsInGameThread(), TEXT("FWorldGetter::SetWorld(...) should only be called from the GameThread."));
#if WITH_EDITOR
		if (FApp::IsGame() && IsValid(InWorld))
#else
		if (IsValid(InWorld))
#endif
		{
			CachedWorld = InWorld;
		}
	}

	/**
	 * Clears the weak cache; the next Get() may repopulate it through normal lookup.
	 * Called by ToroCoreModule through FCoreUObjectDelegates::PreLoadMap.
	 * Call only from the game thread.
	 */
	static void Reset()
	{
		checkf(IsInGameThread(), TEXT("FWorldGetter::Reset() should only be called from the GameThread."));
		CachedWorld.Reset();
	}
};
