// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "AmbientSoundActionBehavior.generated.h"

class AAmbientSound;

/** Command applied to an ambient sound's audio component. */
UENUM(BlueprintType)
enum class EToroAmbientSoundAction : uint8
{
	Play,
	Stop,
	Pause,
	Unpause
};

/**
 * Controls an existing ambient-sound actor without loading it or waiting for audio completion.
 * Play and stop fades below 0.1 seconds are immediate; spatialization follows the target's configuration.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class UAmbientSoundActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

	/** Existing ambient-sound actor to control. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSoftObjectPtr<AAmbientSound> Target;

	/** Playback command to request. */
	UPROPERTY(EditAnywhere, Category = Settings)
	EToroAmbientSoundAction Action = EToroAmbientSoundAction::Play;

	/** Playback offset in seconds for Play. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (EditCondition = "Action == EToroAmbientSoundAction::Play", EditConditionHides, ClampMin = 0.0f, UIMin = 0.0f))
	float StartTime = 0.0f;

	/** Play fade-in or stop fade-out duration in seconds. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (EditCondition = "Action == EToroAmbientSoundAction::Play || Action == EToroAmbientSoundAction::Stop", EditConditionHides, ClampMin = 0.0f, UIMin = 0.0f))
	float FadeTime = 0.0f;

	virtual void OnTriggered() override;
};
