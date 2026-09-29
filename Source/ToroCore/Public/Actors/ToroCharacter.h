// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "GameFramework/Character.h"
#include "Interfaces/ActivatableObject.h"
#include "ToroCharacter.generated.h"

/**
 * Blueprintable character class with look and focus points, an activatable state, and optional editor ticking.
 * Starts on active state. By default, it cannot be damaged and is excluded from automatic HLOD generation.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROCORE_API AToroCharacter : public ACharacter, public IActivatableObject
{
	GENERATED_BODY()

public:

	AToroCharacter();

	/**
	 * Gets the world-space location this character is currently looking at.
	 * @param Location Receives the target location when one is available.
	 * @return True if one exist, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PawnEntity)
	bool GetLookTarget(FVector& Location) const;
	virtual bool GetLookTarget_Implementation(FVector& Location) const;

	/**
	 * Gets the world-space point other actors should use when focusing on this character.
	 * The native default is based on the actor's estimated eye location.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PawnEntity)
	FVector GetFocusPoint() const;
	virtual FVector GetFocusPoint_Implementation() const;

	/**
	 * Sets the owning controller's control rotation when this character has a controller.
	 * @param NewRotation Desired control rotation.
	 * @param bIgnoreRoll If true, preserves the controller's current roll.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = Character)
	void SetControlRotation(const FRotator& NewRotation, bool bIgnoreRoll) const;

	/** Returns the current activation state. */
	virtual bool GetActiveState_Implementation() const override;

	/** Updates activation state and invokes ApplyActiveState only when the state changes. */
	virtual void SetActiveState_Implementation(bool bNewState) override;

	/**
	 * Used for adding actors to levels or teleporting them to a new location.
	 * The result of this function is independent of the actor's current location and rotation.
	 * If the actor doesn't fit exactly at the location specified, tries to slightly move it out of walls and such if bNoCheck is false.
	 * @param DestLocation The target destination point.
	 * @param DestRotation The target rotation at the destination.
	 * @param bIsATest is true if this is a test movement, which shouldn't cause any notifications (used by AI pathfinding, for example).
	 * @param bNoCheck is true if we should skip checking for encroachment in the world or other actors.
	 * @return true if the actor has been successfully moved, or false if it couldn't fit.
	 */
	virtual bool TeleportTo(const FVector& DestLocation, const FRotator& DestRotation, bool bIsATest = false, bool bNoCheck = false) override;

protected:

	/** Initial and current activation state; its behavior is applied at BeginPlay and on changes. */
	UPROPERTY(EditAnywhere, Category = Actor, meta = (DisplayPriority = -1))
	bool bActivated;

#if WITH_EDITORONLY_DATA
	/** Enables viewport ticking and Blueprint script execution while the editor world is active. */
	UPROPERTY(EditDefaultsOnly, Category = Tick)
	bool bTickInEditor = true;
#endif

	/**
	 * Applies the requested active state. The native implementation stops all active movements when deactivated.
	 * An override replaces the native behavior unless it calls the parent implementation.
	 */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "Apply Active State")
	void ApplyActiveState(const bool bInState);
	virtual void ApplyActiveState_Implementation(const bool bInState);

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
	virtual void PostInitializeComponents() override;

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif
};
