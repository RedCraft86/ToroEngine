// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DataTypes/RequestSet.h"
#include "DataTypes/Accumulators.h"
#include "DataTypes/ToroInputConfig.h"
#include "GameFramework/PlayerController.h"
#include "ToroPlayerController.generated.h"

UCLASS(Blueprintable, BlueprintType)
class TOROSYSTEMS_API AToroPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AToroPlayerController();

	/**
	 * Retrieves the active player controller if it matches the requested class.
	 * @param Class Optional class filter; a null or invalid class disables type filtering.
	 * @param PlayerIdx Index in the player controller list, with local players first, then available remote players.
	 * @return The found object, or nullptr if unavailable, invalid, or rejected by a valid class filter.
	 */
	UFUNCTION(BlueprintPure, Category = ToroEngine, DisplayName = "Get Player Controller",
		meta = (WorldContext = ContextObject, DeterminesOutputType = Class))
	[[nodiscard]] static APlayerController* Get(const UObject* ContextObject,
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<APlayerController> Class, int32 PlayerIdx = 0);

	/**
	 * Retrieves the active player controller as the requested type.
	 * @tparam T Type derived from APlayerController.
	 * @param PlayerIdx Index in the player controller list, with local players first, then available remote players.
	 * @return The matching object cast to T, or nullptr if unavailable, invalid, or of a different type.
	 */
	template<typename T = AToroPlayerController>
	[[nodiscard]] static T* Get(const UObject* ContextObject, int32 PlayerIdx = 0)
	{
		static_assert(TIsDerivedFrom<T, APlayerController>::Value,
			"T must derive from APlayerController");
		return Cast<T>(Get(ContextObject, T::StaticClass(), PlayerIdx));
	}

	UFUNCTION(BlueprintCallable, Category = PlayerController, meta = (DefaultToSelf = Source))
	virtual void RequestPause(const UObject* Source);

	UFUNCTION(BlueprintCallable, Category = PlayerController, meta = (DefaultToSelf = Source))
	virtual void ReleasePause(const UObject* Source);

	UFUNCTION(BlueprintCallable, Category = PlayerController, meta = (DefaultToSelf = Source))
	virtual void RequestInputConfig(const UObject* Source, const FToroInputConfig& InConfig);

	UFUNCTION(BlueprintCallable, Category = PlayerController, meta = (DefaultToSelf = Source))
	virtual void ReleaseInputConfig(const UObject* Source);

protected:

	UPROPERTY(EditAnywhere, Category = PlayerController)
	FToroInputConfig DefaultInputConfig;

	TSharedRef<FRequestSet> PauseRequests;
	TSharedRef<TOrderedAccumulator<FToroInputConfig>> InputConfigs;

	void OnPauseRequestsChanged();
	void OnInputConfigsChanged();

	virtual void BeginPlay() override;
};
