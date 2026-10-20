// Copyright (C) RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreGlobals.h"
#include "Containers/UnrealString.h"
#include "Engine/LatentActionManager.h"
#include "Misc/AssertionMacros.h"
#include "Math/NumericLimits.h"

/**
 * Creates latent action information using a shared, game-thread-only ID counter.
 * IDs are reused after wrapping and are not checked against active actions or IDs
 * allocated by other code; they are not guaranteed to be globally unique.
 */
class TOROENGINE_API FLatentInfo final
{
	/** Next ID to issue; wraps to 1000 before the maximum int32 value can be issued. */
	static inline int32 LatentId = 1000;

public:

	/**
	 * Creates latent action information with linkage zero and the next generated ID.
	 * Must be called on the game thread. Does not register or start a latent action.
	 * The default arguments do not provide a usable completion callback.
	 * @param Target Object receiving the completion callback. A callback requires a valid target.
	 * @param FuncName Name of a reflected function on Target compatible with the integer
	 *        linkage argument supplied by the latent action manager.
	 */
	[[nodiscard]] static FLatentActionInfo Make(UObject* Target = nullptr, const FString& FuncName = FString())
	{
		checkf(IsInGameThread(), TEXT("FLatentInfo::Make(...) should only be called from the GameThread."));
		return FLatentActionInfo(0, NextId(), *FuncName, Target);
	}

	/**
	 * Issues the next counter value; must be called on the game thread.
	 * @return An ID in [1000, INT32_MAX - 1]. The sequence wraps to 1000 and does not
	 *         check for collisions with active actions or IDs allocated elsewhere.
	 */
	[[nodiscard]] static int32 NextId()
	{
		checkf(IsInGameThread(), TEXT("FLatentInfo::NextId(...) should only be called from the GameThread."));
		if (LatentId == TNumericLimits<int32>::Max())
		{
			LatentId = 1000;
		}

		return LatentId++;
	}
};