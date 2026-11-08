// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/GameStateBase.h"
#include "ToroGameState.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (ChildCanTick = true))
class TOROSYSTEMS_API AToroGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	AToroGameState();

	/**
	 * Retrieves the active game state if it matches the requested class.
	 * @param Class Optional class filter; a null or invalid class disables type filtering.
	 * @return The found object, or nullptr if unavailable, invalid, or rejected by a valid class filter.
	 */
	UFUNCTION(BlueprintPure, Category = ToroEngine, DisplayName = "Get Game State",
		meta = (WorldContext = ContextObject, DeterminesOutputType = Class))
	[[nodiscard]] static AGameStateBase* Get(const UObject* ContextObject,
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<AGameStateBase> Class);

	/**
	 * Retrieves the active game state as the requested type.
	 * @tparam T Type derived from AGameStateBase.
	 * @return The matching object cast to T, or nullptr if unavailable, invalid, or of a different type.
	 */
	template<typename T>
	[[nodiscard]] static T* Get(const UObject* ContextObject)
	{
		static_assert(TIsDerivedFrom<T, AGameStateBase>::Value,
			"T must derive from AGameStateBase");
		return Cast<T>(Get(ContextObject, T::StaticClass()));
	}
};
