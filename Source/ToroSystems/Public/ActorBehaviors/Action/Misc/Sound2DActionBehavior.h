// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "Sound2DActionBehavior.generated.h"

class USoundBase;

/**
 * Loads a sound asset on demand and issues a non-spatialized, fire-and-forget playback request.
 * Playback completion is independent of the action's execution.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class USound2DActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

	/** Sound asset loaded synchronously when the action executes. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSoftObjectPtr<USoundBase> Sound;

	/** Playback volume multiplier. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float Volume = 1.0f;

	/** Playback pitch multiplier. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.1f, UIMin = 0.1f))
	float Pitch = 1.0f;

	/** Playback offset in seconds; seeking support depends on the sound. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float StartTime = 0.0f;

	virtual void OnTriggered() override;
};
