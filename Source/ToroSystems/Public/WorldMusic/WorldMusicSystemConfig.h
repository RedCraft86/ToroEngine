// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "WorldMusicSystemConfig.generated.h"

class UMetaSoundSource;

USTRUCT(BlueprintInternalUseOnly)
struct TOROSYSTEMS_API FWorldMusicSystemConfig final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	TSoftObjectPtr<UMetaSoundSource> DefaultTrack;

	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 5.0f, UIMax = 5.0f))
	float DefaultFadeTime;

	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	TOptional<float> BasePressure;

	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	TOptional<float> BaseTension;

	FWorldMusicSystemConfig()
		: DefaultTrack(nullptr)
		, DefaultFadeTime(1.0f)
		, BasePressure()
		, BaseTension()
	{}
};
