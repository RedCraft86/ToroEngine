// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "LightProbeSubsystem.generated.h"

#ifndef MAX_SUPPORTED_LIGHT_PROBE_COUNT
#define MAX_SUPPORTED_LIGHT_PROBE_COUNT 32
#endif

class ULightProbeComponent;
struct FLightProbeSystemConfig;

/**
 * Manages registered light probes in game, editor, and PIE worlds on the game thread.
 * Requires AToroWorldSettings, its post-process component, and valid light probe material assets.
 * Periodically selects the nearest relevant probes within the configured budget and updates
 * the material parameter collection and post-process blendable. Updates also run while paused.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class ULightProbeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	ULightProbeSubsystem()
		: CooldownTime(0.0f)
	{}

	/**
	 * Resolves the light probe subsystem for the context's world on the game thread.
	 * @return The world's subsystem, or nullptr if no valid world or subsystem is available.
	 */
	TOROSYSTEMS_API static ULightProbeSubsystem* Get(const UObject* Context);

	/**
	 * Registers a valid probe once using a weak reference, then removes stale registrations.
	 * Must be called on the game thread; registration does not keep the probe alive.
	 * @param InComponent Probe to register. Null or invalid components are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = LightProbes)
	TOROSYSTEMS_API void RegisterProbe(const ULightProbeComponent* InComponent);

	/**
	 * Removes a valid probe's registration, then removes stale registrations. Must be called on the game thread.
	 * @param InComponent Probe to remove. Null or invalid components are ignored; an unregistered probe is harmless.
	 */
	UFUNCTION(BlueprintCallable, Category = LightProbes)
	TOROSYSTEMS_API void UnregisterProbe(const ULightProbeComponent* InComponent);

	/** Removes registrations whose weak component references are not valid. Must be called on the game thread. */
	UFUNCTION(BlueprintCallable, Category = LightProbes)
	TOROSYSTEMS_API void ClearInvalidProbes();

private:

	float CooldownTime;
	TWeakObjectPtr<class AToroWorldSettings> WorldSettings;
	TWeakObjectPtr<class UWorldPostProcessComponent> PostProcess;
	TArray<TWeakObjectPtr<const ULightProbeComponent>> Probes;

	[[nodiscard]] bool IsSystemValid() const;
	[[nodiscard]] const FLightProbeSystemConfig& GetConfig() const;
	void UpdatePostProcessBlendable(bool bRemoval = false) const;

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return IsSystemValid(); }
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(ULightProbeSubsystem, STATGROUP_Tickables);
	}

#if WITH_EDITOR
	virtual bool IsTickableInEditor() const override { return true; }
#endif
};
