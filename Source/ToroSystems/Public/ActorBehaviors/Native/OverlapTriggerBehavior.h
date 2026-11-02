// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "TriggerBehaviorBase.h"
#include "OverlapTriggerBehavior.generated.h"

/**
 * Starts and ends a trigger in response to the owning actor's overlap events with player-controlled pawns.
 * Player control is checked independently on entry and exit; overlapping pawns are not tracked.
 * Binds events at BeginPlay without checking existing overlaps. The owning actor's collision
 * must be configured to generate overlap events.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class UOverlapTriggerBehavior final : public UTriggerBehaviorBase
{
	GENERATED_BODY()

public:

	UOverlapTriggerBehavior() {}

private:

	/**
	 * Starts the trigger if OtherActor is a valid player-controlled pawn.
	 * @param OtherActor Actor entering the owner's overlap.
	 */
	UFUNCTION()
	void OnBeginOverlap(AActor*, AActor* OtherActor);

	/**
	 * Ends the trigger if OtherActor is still a valid player-controlled pawn on exit.
	 * @param OtherActor Actor leaving the owner's overlap.
	 */
	UFUNCTION()
	void OnEndOverlap(AActor*, AActor* OtherActor);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
