// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/GameModeBase.h"
#include "ToroGameMode.generated.h"

UCLASS(Blueprintable, BlueprintType)
class TOROSYSTEMS_API AToroGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	AToroGameMode();

	/**
	 * Retrieves the active game mode if it matches the requested class.
	 * Only available on authority.
	 * @param Class Optional class filter; a null or invalid class disables type filtering.
	 * @return The found object, or nullptr if unavailable, invalid, or rejected by a valid class filter.
	 */
	UFUNCTION(BlueprintPure, Category = ToroEngine, DisplayName = "Get Game Mode",
		meta = (WorldContext = ContextObject, DeterminesOutputType = Class))
	[[nodiscard]] static AGameModeBase* Get(const UObject* ContextObject,
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<AGameModeBase> Class);

	/**
	 * Retrieves the active game mode as the requested type.
	 * @tparam T Type derived from AGameModeBase.
	 * @return The matching object cast to T, or nullptr if unavailable, invalid, or of a different type.
	 */
	template<typename T>
	[[nodiscard]] static T* Get(const UObject* ContextObject)
	{
		static_assert(TIsDerivedFrom<T, AGameModeBase>::Value,
			"T must derive from AGameModeBase");
		return Cast<T>(Get(ContextObject, T::StaticClass()));
	}
};
