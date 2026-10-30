// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameterCollection.h"
#include "LightProbeSystemConfig.generated.h"

/**
 * Per-world light probe settings and access to global light probe console variables.
 * Both material assets must be valid for the system to be enabled.
 */
USTRUCT(BlueprintInternalUseOnly)
struct TOROSYSTEMS_API FLightProbeSystemConfig final
{
	GENERATED_BODY()

	/** Allows light probe processing in this world, subject to global settings and asset validity. */
	UPROPERTY(EditAnywhere, Category = Settings)
	bool bEnableForWorld;

	/** Post-process material used to render the light probe contribution. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TObjectPtr<UMaterialInterface> PostProcessMaterial;

	/** Material parameter collection used to supply light probe data to materials. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TObjectPtr<UMaterialParameterCollection> ParameterCollection;

	FLightProbeSystemConfig()
		: bEnableForWorld(true)
	{}

	/** Whether both required material assets are valid, independent of enablement settings. */
	[[nodiscard]] FORCEINLINE bool IsValid() const
	{
		return ::IsValid(PostProcessMaterial)
			&& ::IsValid(ParameterCollection);
	}

	/**
	 * Whether this world allows light probes, r.LightProbes.Enable is enabled,
	 * the configured probe limit is positive, and both required assets are valid.
	 * Must be called on the game thread.
	 */
	[[nodiscard]] bool IsEnabled() const;

	/**
	 * Returns r.LightProbes.MaxLimit without clamping to the system's supported capacity.
	 * Nonpositive values disable light probe processing. Must be called on the game thread.
	 */
	[[nodiscard]] static int32 GetMaxProbeLimit();

	/**
	 * Returns the update interval in seconds, clamped to a minimum of zero.
	 * Uses r.LightProbes.UpdateInterval, overridden by r.LightProbes.EditorUpdateInterval
	 * in editor builds when FApp::IsGame() is false and the editor interval is nonnegative.
	 * Negative editor intervals retain the runtime interval. Must be called on the game thread.
	 */
	[[nodiscard]] static float GetUpdateInterval();
};
