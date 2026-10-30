// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Engine/Scene.h"
#include "Components/ActorComponent.h"
#include "Interfaces/Interface_PostProcessVolume.h"
#include "WorldPostProcessComponent.generated.h"

class FLumenSceneViewExtension;

/**
 * Supplies world-wide post-process settings as a component owned by ToroWorldSettings.
 * Registers an always-enabled, unbound volume with full blend weight while registered with a world,
 * and maintains a world-scoped view extension for querying cached Lumen GI configuration.
 */
UCLASS(NotBlueprintable, BlueprintType, Within = ToroWorldSettings, HideCategories = (
	ComponentTick, Tags, AssetUserData, Replication, ComponentReplication,
	Activation, Variable, Navigation, Cooking, "Components|Activation"))
class TOROSYSTEMS_API UWorldPostProcessComponent final : public UActorComponent, public IInterface_PostProcessVolume
{
	GENERATED_BODY()

public:

	UWorldPostProcessComponent();

	/** Post-process settings applied across the world, subject to each setting's override flag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PostProcess, meta = (ShowOnlyInnerProperties))
	FPostProcessSettings Settings;

	/** Blend ordering priority; higher values take precedence over lower-priority volumes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PostProcess)
	float Priority;

	/**
	 * Reports the view extension's cached Lumen GI configuration for its latest processed view family.
	 * The cache is initially false and excludes capture views; it does not confirm that Lumen rendered.
	 * Without an extension, checks only whether Settings explicitly overrides the GI method to Lumen,
	 * without checking platform support, console variables, or view-family show flags.
	 */
	[[nodiscard]] bool IsUsingLumen() const;

	virtual FPostProcessVolumeProperties GetProperties() const override;
	virtual bool EncompassesPoint(FVector Point, float SphereRadius, float* OutDistanceToPoint) override;

#if DEBUG_POST_PROCESS_VOLUME_ENABLE
	FORCEINLINE virtual FString GetDebugName() const override
	{
		return TEXT("WorldPostProcess_") + GetName();
	}
#endif

private:

	/** Tie-breaker for volume ordering, initialized from the object path in the editor when missing. */
	UPROPERTY(VisibleAnywhere, Category = PostProcess, TextExportTransient, NonPIEDuplicateTransient, NonTransactional)
	FGuid VolumeGuid;

	/** Keeps the world's Lumen detection extension registered until this component unregisters. */
	TSharedPtr<FLumenSceneViewExtension> LumenSceneViewExtension;

	// Restrict direct activation calls through this type; UActorComponent access remains available.
	virtual void Deactivate() override { Super::Deactivate(); }
	virtual void Activate(bool bReset) override { Super::Activate(bReset); }
	virtual void SetActive(bool bNewActive, bool bReset) override { Super::SetActive(bNewActive, bReset); }
	virtual void SetAutoActivate(bool bNewAutoActivate) override { Super::SetAutoActivate(bNewAutoActivate); }
	virtual void ToggleActive() override { Super::ToggleActive(); }

	virtual void OnRegister() override;
	virtual void OnUnregister() override;

#if WITH_EDITORONLY_DATA
	virtual void Serialize(FArchive& Ar) override;
#endif
#if WITH_EDITOR
	virtual void PostInitProperties() override;
#endif
};
