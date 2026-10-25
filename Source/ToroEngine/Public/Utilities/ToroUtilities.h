// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"

namespace ToroEngine
{
	namespace World
	{
		/**
		 * Resolves a valid world from the optional context on the game thread.
		 * If permitted and context resolution fails, tries the current play world, then GWorld.
		 * Context-free selection is best-effort; supply context when world identity matters.
		 * @param bTryFindPossible Whether to try fallback worlds when the context has no valid world.
		 * @return The resolved valid world, or nullptr if resolution fails.
		 */
		[[nodiscard]] TOROENGINE_API UWorld* Get(const UObject* Context = nullptr, bool bTryFindPossible = true);
	}

	namespace Latent
	{
		/**
		 * Creates latent action information with linkage zero and a successive ID on the game thread.
		 * IDs start at 1000 and wrap to 1000 before reaching the maximum int32 value.
		 * IDs may collide with outstanding actions after wrapping or with IDs assigned elsewhere.
		 * @param Target Object used to associate the action and receive its completion callback.
		 * @param FuncName Name of the function to invoke on Target when the action completes.
		 * @note The default arguments do not configure a usable completion callback.
		 */
		[[nodiscard]] TOROENGINE_API FLatentActionInfo MakeInfo(UObject* Target = nullptr, const FString& FuncName = FString());
	}
}
