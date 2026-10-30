// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameterCollection.h"
#include "LightProbeSystemConfig.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct TOROSYSTEMS_API FLightProbeSystemConfig final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	bool bEnableForWorld;

	UPROPERTY(EditAnywhere, Category = Settings)
	TObjectPtr<UMaterialInterface> PostProcessMaterial;

	UPROPERTY(EditAnywhere, Category = Settings)
	TObjectPtr<UMaterialParameterCollection> ParameterCollection;

	FLightProbeSystemConfig()
		: bEnableForWorld(true)
	{}

	[[nodiscard]] FORCEINLINE bool IsValid() const
	{
		return ::IsValid(PostProcessMaterial)
			&& ::IsValid(ParameterCollection);
	}

	[[nodiscard]] bool IsEnabled() const;
	[[nodiscard]] static int32 GetMaxProbeLimit();
	[[nodiscard]] static float GetUpdateInterval();
};
