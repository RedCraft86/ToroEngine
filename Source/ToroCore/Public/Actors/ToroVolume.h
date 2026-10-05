// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "GameFramework/Volume.h"
#include "Interfaces/ActivatableObject.h"
#include "ToroVolume.generated.h"

/**
 * Blueprintable gameplay volume with an activatable state and optional editor ticking.
 * Starts active and applies activation to visibility, collision, and ticking at BeginPlay.
 * By default, it cannot be damaged and is excluded from automatic HLOD generation.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROCORE_API AToroVolume : public AVolume, public IActivatableObject
{
	GENERATED_BODY()

public:

	AToroVolume();

	/** Returns the current activation state. */
	virtual bool GetActiveState_Implementation() const override;

	/** Updates activation state and invokes ApplyActiveState only when the state changes. */
	virtual void SetActiveState_Implementation(bool bNewState) override;

protected:

	/** Initial and current activation state; its behavior is applied at BeginPlay and on changes. */
	UPROPERTY(EditAnywhere, Category = Actor, meta = (DisplayPriority = -1))
	bool bActivated;

#if WITH_EDITORONLY_DATA
	/** Enables actor ticking in editor viewports and Blueprint script execution during editor ticks. */
	UPROPERTY(EditDefaultsOnly, Category = Tick)
	bool bTickInEditor = false;
#endif

	/**
	 * Applies the requested active state. The native implementation updates visibility, collision, and ticking.
	 * An override replaces the native behavior unless it calls the parent implementation.
	 */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "Apply Active State")
	void ApplyActiveState(bool bInState);
	virtual void ApplyActiveState_Implementation(bool bInState);

#if WITH_EDITOR
	/**
	 * Runs this actor's IsDataValid implementation and displays its issues and result in the Map Check log.
	 * Additional validators registered with the editor validator subsystem are not invoked.
	 */
	UFUNCTION(CallInEditor, Category = Actor, DisplayName = "Validate")
	virtual void ValidateData() const;
#endif

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif
};
