// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DataTypes/RequestSet.h"
#include "DataTypes/Accumulators.h"
#include "DataTypes/ToroInputConfig.h"
#include "GameFramework/PlayerController.h"
#include "ToroPlayerController.generated.h"

/**
 * Player controller with source-owned pause and input configuration requests.
 * Pause requests keep the game paused while any remain; the latest input request takes precedence.
 * Sources are weakly tracked and invalid sources are pruned after garbage collection or successful request changes.
 * Release requests when their purpose ends, especially for pooled widgets that remain alive.
 * Request operations must run on the game thread.
 * At BeginPlay, applies the current level's optional input mapping context at priority zero
 * and enables camera updates while paused.
 */
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

	/**
	 * Requests that the game remain paused while this source's request exists.
	 * Repeated requests from the same source do not stack. Pausing is subject to the game mode's pause rules.
	 * @param Source Valid object identifying the request, without being retained; invalid sources are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = PlayerController, meta = (DefaultToSelf = Source))
	virtual void RequestPause(const UObject* Source);

	/**
	 * Releases this source's pause request, attempting to unpause when no requests remain.
	 * @param Source Object identifying the request; invalid sources and sources without a request are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = PlayerController, meta = (DefaultToSelf = Source))
	virtual void ReleasePause(const UObject* Source);

	/**
	 * Applies an input configuration with precedence over all earlier requests.
	 * Re-requesting from the same source replaces its configuration and makes it the latest request.
	 * Applying a configuration flushes pressed keys and updates input mode and cursor visibility.
	 * @param Source Valid object identifying the request, without being retained; invalid sources are ignored.
	 * @param InConfig Configuration copied into the request and used until superseded or released.
	 */
	UFUNCTION(BlueprintCallable, Category = PlayerController, meta = (DefaultToSelf = Source))
	virtual void RequestInputConfig(const UObject* Source, const FToroInputConfig& InConfig);

	/**
	 * Releases this source's input request and reapplies the latest remaining configuration.
	 * Falls back to DefaultInputConfig when no requests remain.
	 * @param Source Object identifying the request; invalid sources and sources without a request are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = PlayerController, meta = (DefaultToSelf = Source))
	virtual void ReleaseInputConfig(const UObject* Source);

protected:

	/**
	 * Input configuration applied at BeginPlay and whenever no source-owned input requests remain.
	 * Controls input mode and mouse cursor behavior.
	 */
	UPROPERTY(EditAnywhere, Category = PlayerController)
	FToroInputConfig DefaultInputConfig;

	TSharedRef<FRequestSet> PauseRequests = FRequestSet::Make();
	TSharedRef<TOrderedAccumulator<FToroInputConfig>> InputConfigs = TOrderedAccumulator<FToroInputConfig>::Make();

	void OnPauseRequestsChanged();
	void OnInputConfigsChanged();

	virtual void BeginPlay() override;
};
