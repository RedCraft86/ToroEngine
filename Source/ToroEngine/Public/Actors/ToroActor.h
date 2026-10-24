// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/Actor.h"
#include "Interfaces/ActivatableObject.h"
#include "ToroActor.generated.h"

/**
 * Base actor with a native scene root, activation support, and optional editor ticking.
 * Default activation behavior controls in-game visibility, collision, and actor ticking.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, meta = (ChildCanTick = true))
class TOROENGINE_API AToroActor : public AActor, public IActivatableObject
{
	GENERATED_BODY()

public:

	AToroActor();

	virtual bool GetActiveState_Implementation() const override;
	virtual void SetActiveState_Implementation(bool bNewState) override;

protected:

	/** Default native scene component installed as this actor's root. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Subobjects)
	TObjectPtr<class UToroSceneComponent> SceneRoot;

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
#endif

	/**
	 * Applies activation behavior at BeginPlay and after interface-driven state changes.
	 * The default implementation hides the actor in game and disables collision and actor ticking when inactive.
	 * When active, it unhides the actor, enables collision, and requests actor ticking only if bStartWithTickEnabled is true.
	 * Native overrides should call Super::ApplyActiveState_Implementation() to retain these default effects.
	 * @param bInState Activation state to apply.
	 */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "Apply Active State")
	void ApplyActiveState(bool bInState);
	virtual void ApplyActiveState_Implementation(bool bInState);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif
};
