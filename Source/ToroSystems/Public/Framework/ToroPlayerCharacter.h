// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Actors/ToroCharacter.h"
#include "ToroPlayerCharacter.generated.h"

UCLASS(Blueprintable, BlueprintType)
class TOROSYSTEMS_API AToroPlayerCharacter : public AToroCharacter
{
	GENERATED_BODY()

public:

	/**
	 * Retrieves the active player character if it matches the requested class.
	 * @param Class Optional class filter; a null or invalid class disables type filtering.
	 * @param PlayerIdx Index in the player controller list, with local players first, then available remote players.
	 * @return The found object, or nullptr if unavailable, invalid, or rejected by a valid class filter.
	 */
	UFUNCTION(BlueprintPure, Category = ToroEngine, DisplayName = "Get Player Character",
		meta = (WorldContext = ContextObject, DeterminesOutputType = Class))
	[[nodiscard]] static ACharacter* Get(const UObject* ContextObject,
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<ACharacter> Class, int32 PlayerIdx = 0);

	/**
	 * Retrieves the active player character as the requested type.
	 * @tparam T Type derived from ACharacter.
	 * @param PlayerIdx Index in the player controller list, with local players first, then available remote players.
	 * @return The matching object cast to T, or nullptr if unavailable, invalid, or of a different type.
	 */
	template<typename T>
	[[nodiscard]] static T* Get(const UObject* ContextObject, int32 PlayerIdx = 0)
	{
		static_assert(TIsDerivedFrom<T, ACharacter>::Value,
			"T must derive from ACharacter");
		return Cast<T>(Get(ContextObject, T::StaticClass(), PlayerIdx));
	}
};
