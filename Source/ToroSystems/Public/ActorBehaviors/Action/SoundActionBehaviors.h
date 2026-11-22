// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UObject/SoftObjectPtr.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "SoundActionBehaviors.generated.h"

class AAmbientSound;
class USoundBase;

/** Playback command for an Ambient Sound actor. */
UENUM()
enum class EToroAmbientSoundAction : uint8
{
	Play,
	Stop,
	Pause,
	Unpause
};

/**
 * Plays a non-spatialized 2D sound effect.
 * Ideal for UI sounds, background music, or global voice-overs that do not have a location in 3D space.
 */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Sound] Simple (2D)")
class TOROSYSTEMS_API USound2DActionBehavior : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	/** The sound asset to play. Handled as a Soft Pointer to avoid loading the asset until the action runs. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSoftObjectPtr<USoundBase> Sound;

	/** Multiplier for the sound volume. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.1, UIMin = 0.1))
	float Volume = 1.0f;

	/** Multiplier for the sound pitch/speed. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.1, UIMin = 0.1))
	float Pitch = 1.0f;

	/** The timestamp (in seconds) within the audio file to begin playback. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.0, UIMin = 0.0))
	float StartTime = 0.0f;

	virtual void OnTriggered() override;
};

/**
 * Controls an existing spatialized AAmbientSound actor in the level.
 * Supports playback, fades, and pause control.
 */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Sound] World (3D)")
class TOROSYSTEMS_API UAmbientSoundActionBehavior : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	/** The target Ambient Sound actor in the current level. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSoftObjectPtr<AAmbientSound> SoundActor;

	/** The specific audio command to send to the target actor. */
	UPROPERTY(EditAnywhere, Category = Settings)
	EToroAmbientSoundAction Action = EToroAmbientSoundAction::Play;

	/** Playback start offset in seconds. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (EditCondition = "Action == EToroAmbientSoundAction::Play", EditConditionHides, ClampMin = 0.0, UIMin = 0.0))
	float StartTime = 0.0f;

	/** Fade duration in seconds. Values below 0.1 play or stop immediately. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (EditCondition = "Action == EToroAmbientSoundAction::Play || Action == EToroAmbientSoundAction::Stop", EditConditionHides, ClampMin = 0.0, UIMin = 0.0))
	float FadeTime = 0.0f;

	virtual void OnTriggered() override;
};
