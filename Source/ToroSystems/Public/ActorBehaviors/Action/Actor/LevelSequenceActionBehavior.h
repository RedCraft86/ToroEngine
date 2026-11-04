// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "DataTypes/MiscEnums.h"
#include "UObject/SoftObjectPtr.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "LevelSequenceActionBehavior.generated.h"

class ALevelSequenceActor;

/**
 * Requests forward or reverse playback on an existing level-sequence actor through Toro's sequence helper.
 * Does not load the actor or wait for the sequence to finish.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class ULevelSequenceActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

	/** Existing sequence actor to play. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSoftObjectPtr<ALevelSequenceActor> Target;

	/** Requested playback direction. */
	UPROPERTY(EditAnywhere, Category = Settings)
	EToroAnimationPlayMode PlayMode = EToroAnimationPlayMode::Forward;

	/** Playback rate; direction is selected separately. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.1f, UIMin = 0.1f))
	float PlayRate = 1.0f;

	/** Retain the current position; false requests a rewind before playback. */
	UPROPERTY(EditAnywhere, Category = Settings)
	bool bResumePlay = true;

	virtual void OnTriggered() override;
};
