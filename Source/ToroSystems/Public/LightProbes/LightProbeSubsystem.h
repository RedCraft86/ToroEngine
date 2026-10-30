// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "LightProbeSystemConfig.h"
#include "Utilities/ToroUtilities.h"
#include "Subsystems/WorldSubsystem.h"
#include "LightProbeSubsystem.generated.h"

#ifndef MAX_SUPPORTED_LIGHT_PROBE_COUNT
#define MAX_SUPPORTED_LIGHT_PROBE_COUNT 32
#endif

class ULightProbeComponent;

UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class ULightProbeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	ULightProbeSubsystem()
		: CooldownTime(0.0f)
	{}

	TOROSYSTEMS_API static ULightProbeSubsystem* Get(const UObject* Context)
	{
		const UWorld* World = ToroEngine::GetWorld(Context);
		return IsValid(World) ? World->GetSubsystem<ULightProbeSubsystem>() : nullptr;
	}

	UFUNCTION(BlueprintCallable, Category = LightProbes)
	TOROSYSTEMS_API void RegisterProbe(const ULightProbeComponent* InComponent);

	UFUNCTION(BlueprintCallable, Category = LightProbes)
	TOROSYSTEMS_API void UnregisterProbe(const ULightProbeComponent* InComponent);

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
