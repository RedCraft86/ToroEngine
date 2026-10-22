// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreGlobals.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "UObject/Object.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Misc/AssertionMacros.h"
#if WITH_EDITOR
#include "Misc/App.h"
#endif

/**
 * Game-thread-only world lookup with a shared weak fallback cache.
 * Caching does not happen in the editor to prevent world leakage.
 * The cache does not keep a world alive.
 */
class TOROENGINE_API FWorldGetter final
{
	/** Last cached or explicitly supplied fallback world, held without extending its lifetime. */
	static inline TWeakObjectPtr<UWorld> CachedWorld = nullptr;

public:
	/**
	 * Resolves a world on the game thread, preferring a valid explicit context.
	 * Falls back to the permitted cache, current play world, and finally GWorld.
	 * Within the editor, the world context is not cached to avoid leakage.
	 * Context-free selection is best-effort; supply context when world identity matters.
	 * @return The resolved valid world, or null if every resolution path fails.
	 */
	[[nodiscard]] static UWorld* Get(const UObject* Context = nullptr)
	{
		checkf(IsInGameThread(), TEXT("FWorldGetter::Get(...) should only be called from the GameThread."));

		UWorld* PossibleWorld = nullptr;
		if (IsValid(Context) && !Context->IsTemplate() && GEngine)
		{
			PossibleWorld = GEngine->GetWorldFromContextObject(Context, EGetWorldErrorMode::ReturnNull);
		}

#if WITH_EDITOR
		if (FApp::IsGame() && !IsValid(PossibleWorld))
#else
		if (!IsValid(PossibleWorld))
#endif
		{
			PossibleWorld = CachedWorld.Get();
		}

		if (!IsValid(PossibleWorld) && GEngine)
		{
			PossibleWorld = GEngine->GetCurrentPlayWorld();
		}

		if (!IsValid(PossibleWorld))
		{
			PossibleWorld = GWorld.GetReference();
		}

		if (!IsValid(PossibleWorld))
		{
			return nullptr;
		}

#if WITH_EDITOR
		if (!FApp::IsGame())
		{
			return PossibleWorld;
		}
#endif

		CachedWorld = PossibleWorld;
		return CachedWorld.Get();
	}

	/**
	 * Supplies the shared fallback world on the game thread.
	 * A resolvable context passed to Get() still takes precedence and may replace it.
	 * Invalid worlds and assignments while editor caching is disabled are ignored.
	 * @param InWorld Valid world to cache without extending its lifetime.
	 * @note This is automatically called by the ToroEngine module during <c>FWorldDelegates::OnPostWorldInitialization</c>.
	 */
	static void SetWorld(UWorld* InWorld)
	{
#if WITH_EDITOR
		if (!FApp::IsGame())
		{
			return;
		}
#endif 

		checkf(IsInGameThread(), TEXT("FWorldGetter::SetWorld(...) should only be called from the GameThread."));
		if (IsValid(InWorld))
		{
			CachedWorld = InWorld;
		}
	}

	/**
	 * Clears the shared weak cache; must be called on the game thread.
	 * @note This is automatically called by the ToroEngine module during <c>FWorldDelegates::OnWorldCleanup</c>.
	 */
	static void Reset()
	{
		checkf(IsInGameThread(), TEXT("FWorldGetter::Reset() should only be called from the GameThread."));
		CachedWorld.Reset();
	}
};
