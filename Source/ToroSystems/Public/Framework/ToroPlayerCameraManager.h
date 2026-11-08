// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "ToroPlayerCameraManager.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (ChildCanTick = true))
class TOROSYSTEMS_API AToroPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:

	AToroPlayerCameraManager();

	/**
	 * Retrieves the active player camera manager if it matches the requested class.
	 * @param Class Optional class filter; a null or invalid class disables type filtering.
	 * @param PlayerIdx Index in the player controller list, with local players first, then available remote players.
	 * @return The found object, or nullptr if unavailable, invalid, or rejected by a valid class filter.
	 */
	UFUNCTION(BlueprintPure, Category = ToroEngine, DisplayName = "Get Player Camera Manager",
		meta = (WorldContext = ContextObject, DeterminesOutputType = Class))
	[[nodiscard]] static APlayerCameraManager* Get(const UObject* ContextObject,
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<APlayerCameraManager> Class, int32 PlayerIdx = 0);

	/**
	 * Retrieves the active player camera manager as the requested type.
	 * @tparam T Type derived from APlayerCameraManager.
	 * @param PlayerIdx Index in the player controller list, with local players first, then available remote players.
	 * @return The matching object cast to T, or nullptr if unavailable, invalid, or of a different type.
	 */
	template<typename T = AToroPlayerCameraManager>
	[[nodiscard]] static T* Get(const UObject* ContextObject, int32 PlayerIdx = 0)
	{
		static_assert(TIsDerivedFrom<T, APlayerCameraManager>::Value,
			"T must derive from APlayerCameraManager");
		return Cast<T>(Get(ContextObject, T::StaticClass(), PlayerIdx));
	}
};
