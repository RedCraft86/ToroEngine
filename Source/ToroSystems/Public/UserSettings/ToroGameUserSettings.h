// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/GameUserSettings.h"
#include "ToroGameUserSettings.generated.h"

UCLASS(Blueprintable, BlueprintType)
class TOROSYSTEMS_API UToroGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:

	/**
	 * Retrieves the engine's active game user settings if they match the requested class.
	 * @param Class Optional class filter; a null or invalid class disables type filtering.
	 * @return The found object, or nullptr if unavailable, invalid, or rejected by a valid class filter.
	 */
	UFUNCTION(BlueprintPure, Category = ToroEngine, DisplayName = "Get Game User Settings",
		meta = (DeterminesOutputType = Class))
	[[nodiscard]] static UGameUserSettings* Get(
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UGameUserSettings> Class);

	/**
	 * Retrieves the engine's active game user settings as the requested type.
	 * @tparam T Type derived from UGameUserSettings.
	 * @return The matching object cast to T, or nullptr if unavailable, invalid, or of a different type.
	 */
	template<typename T = UToroGameUserSettings>
	[[nodiscard]] static T* Get()
	{
		static_assert(TIsDerivedFrom<T, UGameUserSettings>::Value,
			"T must derive from UGameUserSettings");
		return Cast<T>(Get(T::StaticClass()));
	}
};
