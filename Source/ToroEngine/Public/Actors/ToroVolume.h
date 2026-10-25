// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/Volume.h"
#include "Interfaces/ActivatableObject.h"
#include "ToroVolume.generated.h"

/**
 * Base volume with activation-controlled visibility, collision, and ticking, plus optional editor ticking.
 * Native ticking and damage reception are disabled by default.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, meta = (ChildCanTick = true))
class TOROENGINE_API AToroVolume : public AVolume, public IActivatableObject
{
	GENERATED_BODY()

public:

	AToroVolume();

	virtual bool GetActiveState_Implementation() const override;
	virtual void SetActiveState_Implementation(bool bNewState) override;

protected:

	/**
	 * Configured activation state, initially true and updated through the activation interface setter.
	 * Applied at BeginPlay and whenever the setter changes the state.
	 */
	UPROPERTY(EditAnywhere, Category = Actor, meta = (DisplayPriority = -1))
	bool bActivated;

#if WITH_EDITORONLY_DATA
	/**
	 * Allows viewport-only editor ticking and actor Blueprint tick execution in editor worlds.
	 * Tick capability and tick enablement must also be configured separately.
	 */
	UPROPERTY(EditDefaultsOnly, Category = Tick)
	bool bTickInEditor = false;

	/** If the volume brush should use a custom color as opposed to the default color. */
	UPROPERTY(EditAnywhere, Category = BrushSettings, meta = (InlineEditConditionToggle))
	bool bColoredVolume = false;

	/** Custom color the volume brush should use. */
	UPROPERTY(EditAnywhere, Category = BrushSettings, DisplayName = "Color", meta = (HideAlphaChannel, EditCondition = bColoredVolume))
	FColor VolumeColor = FColor::Green;
#endif

	/**
	 * Applies activation behavior at BeginPlay and after interface-driven state changes.
	 * The default implementation hides the volume in game and disables collision and actor ticking when inactive.
	 * When active, it unhides the volume, enables collision, and requests actor ticking only if bStartWithTickEnabled is true.
	 * Native overrides should call Super::ApplyActiveState_Implementation() to retain these default effects.
	 * @param bInState Activation state to apply.
	 */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "Apply Active State")
	void ApplyActiveState(bool bInState);
	virtual void ApplyActiveState_Implementation(bool bInState);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif
};
