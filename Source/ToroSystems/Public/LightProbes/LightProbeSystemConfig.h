// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UObject/SoftObjectPtr.h"
#include "LightProbeSystemConfig.generated.h"

class UMaterialInterface;
class UMaterialParameterCollection;

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
	TSoftObjectPtr<UMaterialInterface> PostProcessMaterial;

	/** Material parameter collection used to supply light probe data to materials. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSoftObjectPtr<UMaterialParameterCollection> ParameterCollection;

	FLightProbeSystemConfig();

	/**
	 * Synchronously resolves the required material assets and checks their validity,
	 * independent of enablement settings. May load assets; call on the game thread.
	 */
	[[nodiscard]] bool IsValid() const;

	/**
	 * Whether this world allows light probes, r.LightProbes.Enable is enabled,
	 * the configured probe limit is positive, and both required assets are valid.
	 * Must be called on the game thread.
	 */
	[[nodiscard]] bool IsEnabled() const;

	/**
	 * Resolves the configured post-process material, loading it synchronously if needed.
	 * May block while loading; call on the game thread. Does not check system enablement.
	 * @return The resolved material, or nullptr if unassigned or loading fails.
	 */
	[[nodiscard]] UMaterialInterface* GetPostProcessMaterial() const;

	/**
	 * Resolves the configured material parameter collection, loading it synchronously if needed.
	 * May block while loading; call on the game thread. Does not check system enablement.
	 * @return The resolved collection, or nullptr if unassigned or loading fails.
	 */
	[[nodiscard]] UMaterialParameterCollection* GetParameterCollection() const;

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
