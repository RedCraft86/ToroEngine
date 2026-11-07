// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "NativeGameplayTags.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"
#include "WorldMusicSystemConfig.generated.h"

class UMetaSoundSource;

namespace WorldMusicTags
{
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Base);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pressure);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tension);
}

/**
 * World music defaults used to start playback and initialize or reset parameter contributions.
 */
USTRUCT(BlueprintInternalUseOnly)
struct TOROSYSTEMS_API FWorldMusicSystemConfig final
{
	GENERATED_BODY()

	/** MetaSound loaded synchronously at world begin play; unset by default to skip automatic playback. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSoftObjectPtr<UMetaSoundSource> DefaultTrack;

	/** Fade-in duration in seconds for the default track; defaults to one second, with zero starting immediately. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 5.0f, UIMax = 5.0f))
	float DefaultFadeTime;

	/**
	 * Initial additive contributions, restored when a track change requests a parameter reset.
	 * Keys must be descendants of WorldMusic; the root itself is not a parameter.
	 * Audio parameter names omit the WorldMusic prefix (WorldMusic.Pressure becomes Pressure).
	 */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.0f, UIMin = 0.0f, Categories = "WorldMusic"))
	TMap<FGameplayTag, float> BaseParameters;

	FWorldMusicSystemConfig()
		: DefaultTrack(nullptr)
		, DefaultFadeTime(1.0f)
		, BaseParameters()
	{}
};
