// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CommonGameViewportClient.h"
#include "ToroGameViewportClient.generated.h"

UCLASS(NotBlueprintable, BlueprintType)
class TOROSYSTEMS_API UToroGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()

public:

	/**
	 * Retrieves the active game viewport client if it matches the requested class.
	 * Only available when the context's game instance has a viewport.
	 * @param Class Optional class filter; a null or invalid class disables type filtering.
	 * @return The found object, or nullptr if unavailable, invalid, or rejected by a valid class filter.
	 */
	UFUNCTION(BlueprintPure, Category = ToroEngine, DisplayName = "Get Game Viewport Client",
		meta = (WorldContext = ContextObject, DeterminesOutputType = Class))
	[[nodiscard]] static UGameViewportClient* Get(const UObject* ContextObject,
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UGameViewportClient> Class);

	/**
	 * Retrieves the active game viewport client as the requested type.
	 * @tparam T Type derived from UGameViewportClient.
	 * @return The matching object cast to T, or nullptr if unavailable, invalid, or of a different type.
	 */
	template<typename T = UToroGameViewportClient>
	[[nodiscard]] static T* Get(const UObject* ContextObject)
	{
		static_assert(TIsDerivedFrom<T, UGameViewportClient>::Value,
			"T must derive from UGameViewportClient");
		return Cast<T>(Get(ContextObject, T::StaticClass()));
	}
};
