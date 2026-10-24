// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/Character.h"
#include "Interfaces/ActivatableObject.h"
#include "ToroCharacter.generated.h"

/**
 * Base character with activation state, overridable look locations, and optional editor ticking.
 * Native ticking and damage reception are disabled by default.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, meta = (ChildCanTick = true))
class TOROENGINE_API AToroCharacter : public ACharacter, public IActivatableObject
{
	GENERATED_BODY()

public:

	AToroCharacter();

	/**
	 * Queries the world-space location this character is looking toward.
	 * The default implementation writes zero and returns false.
	 * @param Location Receives the target location; only meaningful when the query succeeds.
	 * @return True when a look target is available; false otherwise.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PawnEntity)
	bool GetLookTargetLocation(FVector& Location) const;
	virtual bool GetLookTargetLocation_Implementation(FVector& Location) const;

	/**
	 * Returns the world-space point on this character that others should look toward.
	 * Defaults to the local vertical BaseEyeHeight offset transformed by the actor's scale, rotation, and location.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PawnEntity)
	FVector GetLookAnchorLocation() const;
	virtual FVector GetLookAnchorLocation_Implementation() const;

	/**
	 * Sets the possessing controller's control rotation; does nothing without a controller.
	 * @param NewRotation Desired control rotation.
	 * @param bIgnoreRoll Whether to preserve the current control roll instead of applying NewRotation's.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = Character)
	void SetControlRotation(const FRotator& NewRotation, bool bIgnoreRoll) const;

	virtual bool GetActiveState_Implementation() const override;
	virtual void SetActiveState_Implementation(bool bNewState) override;
	virtual bool TeleportTo(const FVector& DestLocation, const FRotator& DestRotation, bool bIsATest = false, bool bNoCheck = false) override;

protected:

	/**
	 * Configured activation state, initially true and updated by SetActiveState_Implementation().
	 * Applied at BeginPlay and when the interface setter changes it.
	 * The default response to deactivation stops movement without disabling collision or actor ticking.
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
	 * The default native implementation stops movement when deactivated and does nothing when activated.
	 * @param bInState Activation state to apply.
	 */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "Apply Active State")
	void ApplyActiveState(bool bInState);
	virtual void ApplyActiveState_Implementation(bool bInState);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostInitializeComponents() override;

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif
};
