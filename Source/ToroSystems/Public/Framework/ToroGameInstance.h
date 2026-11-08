// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Engine/GameInstance.h"
#include "ToroGameInstance.generated.h"

UCLASS(Blueprintable, BlueprintType)
class TOROSYSTEMS_API UToroGameInstance : public UGameInstance
{
	GENERATED_BODY()

	friend class AToroGameMode;

public:

	/**
	 * Retrieves the context's game instance if it matches the requested class.
	 * @param Class Optional class filter; a null or invalid class disables type filtering.
	 * @return The found object, or nullptr if unavailable, invalid, or rejected by a valid class filter.
	 */
	UFUNCTION(BlueprintPure, Category = ToroEngine, DisplayName = "Get Game Instance",
		meta = (WorldContext = ContextObject, DeterminesOutputType = Class))
	[[nodiscard]] static UGameInstance* Get(const UObject* ContextObject,
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UGameInstance> Class);

	/**
	 * Retrieves the context's game instance as the requested type.
	 * @tparam T Type derived from UGameInstance.
	 * @return The matching object cast to T, or nullptr if unavailable, invalid, or of a different type.
	 */
	template<typename T = UToroGameInstance>
	[[nodiscard]] static T* Get(const UObject* ContextObject)
	{
		static_assert(TIsDerivedFrom<T, UGameInstance>::Value,
			"T must derive from UGameInstance");
		return Cast<T>(Get(ContextObject, T::StaticClass()));
	}

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "World Begin Play")
	void ReceiveWorldBeginPlay();

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "World End Play")
	void ReceiveWorldEndPlay();

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "World Tick")
	void ReceiveWorldTick(float DeltaTime, bool bGamePaused);

protected:

	virtual void WorldBeginPlay();
	virtual void WorldEndPlay();
	virtual void WorldTick(float DeltaTime, bool bGamePaused);
};
