// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Misc/EnumClassFlags.h"
#include "Actors/ToroCharacter.h"
#include "DataTypes/Accumulators.h"
#include "DataTypes/ToroPlayerSettings.h"
#include "ToroPlayerCharacter.generated.h"

class UInteractTriggerBehavior;
class UIdentityBehavior;
class USaveGameBehavior;
class UInventoryBehavior;
class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class APlayerCameraManager;
class UCameraShakeBase;
struct FInputActionValue;

/** Player actions that can be individually restricted. */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EPlayerCapability : uint8
{
	None = 0 UMETA(Hidden),
	Move = 1 << 0,
	Look = 1 << 1,
	Run = 1 << 2,
	Crouch = 1 << 3,
	Lean = 1 << 4,
	Interact = 1 << 5,
	All = (1 << 6) - 1 UMETA(Hidden),
};
ENUM_CLASS_FLAGS(EPlayerCapability)

/** Active player actions. Moving tracks movement input; IsMoving also checks actual movement. */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EPlayerStateFlags : uint8
{
	None = 0 UMETA(Hidden),
	Moving = 1 << 0,
	Running = 1 << 1,
	Crouching = 1 << 2,
	Leaning = 1 << 3,
	Interacting = 1 << 4,
};
ENUM_CLASS_FLAGS(EPlayerStateFlags)

/** Requested lean direction relative to the player. */
UENUM(BlueprintType)
enum class EPlayerLeanState : uint8
{
	None,
	Left,
	Right,
};

/**
 * A source's contribution to movement speed and camera field of view.
 * Speed multipliers multiply together and FOV offsets add together.
 */
struct FPlayerMovementModifier
{
	/** Factor applied to walk, run, and crouch speeds. Negative factors are treated as zero. */
	float SpeedMultiplier = 1.0f;

	/** Additive field-of-view offset in degrees. */
	float FOVOffset = 0.0f;
};

/**
 * First-person player pawn with keyboard and mouse movement, equipment, and an interaction target.
 * The camera trace selects interactables while the cursor is inactive; cursor hits select them while it is active.
 * Configure input mappings on the level and capability defaults on the character class.
 */
UCLASS(Blueprintable, BlueprintType, PrioritizeCategories = (Settings))
class TOROSYSTEMS_API AToroPlayerCharacter : public AToroCharacter
{
	GENERATED_BODY()

public:

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractTargetChanged, const UInteractTriggerBehavior*)

	AToroPlayerCharacter();

	/**
	 * Retrieves the active player character if it matches the requested class.
	 * @param Class Optional class filter; a null or invalid class disables type filtering.
	 * @param PlayerIdx Index in the player controller list, with local players first, then available remote players.
	 * @return The found object, or nullptr if unavailable, invalid, or rejected by a valid class filter.
	 */
	UFUNCTION(BlueprintPure, Category = ToroEngine, DisplayName = "Get Player Character",
		meta = (WorldContext = ContextObject, DeterminesOutputType = Class))
	[[nodiscard]] static ACharacter* Get(const UObject* ContextObject,
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<ACharacter> Class, int32 PlayerIdx = 0);

	/**
	 * Retrieves the active player character as the requested type.
	 * @tparam T Type derived from ACharacter.
	 * @param PlayerIdx Index in the player controller list, with local players first, then available remote players.
	 * @return The matching object cast to T, or nullptr if unavailable, invalid, or of a different type.
	 */
	template<typename T = AToroPlayerCharacter>
	[[nodiscard]] static T* Get(const UObject* ContextObject, int32 PlayerIdx = 0)
	{
		static_assert(TIsDerivedFrom<T, ACharacter>::Value,
			"T must derive from ACharacter");
		return Cast<T>(Get(ContextObject, T::StaticClass(), PlayerIdx));
	}

	/** Returns the player's identity behavior. */
	[[nodiscard]] FORCEINLINE UIdentityBehavior* GetIdentity()
	{
		return Identity;
	}

	/** Returns the player's inventory and equipment behavior. */
	[[nodiscard]] FORCEINLINE UInventoryBehavior* GetInventory()
	{
		return Inventory;
	}

	// Capabilities
	/** Returns whether all requested capabilities are available after applying active blocks. */
	UFUNCTION(BlueprintPure, Category = "Player|Capabilities")
	[[nodiscard]] bool HasCapability(EPlayerCapability Capability) const;

	/** Replaces this weakly tracked source's capability block; an empty mask releases it. */
	void BlockCapabilities(const UObject* Source, EPlayerCapability Mask) const;

	/** Releases this source's capability block. Blocks from other sources still apply. */
	void ReleaseCapabilityBlock(const UObject* Source) const;

	// Player state
	/** Returns whether all requested action states are active. */
	UFUNCTION(BlueprintPure, Category = "Player|State")
	[[nodiscard]] bool HasPlayerState(EPlayerStateFlags State) const;

	/** Returns active movement actions and whether the current target's interaction trigger is running. */
	[[nodiscard]] EPlayerStateFlags GetPlayerStateFlags() const;

	/** Whether movement input is active and horizontal speed exceeds 1 cm/s. */
	UFUNCTION(BlueprintPure, Category = "Player|State")
	[[nodiscard]] bool IsMoving() const;

	// Movement
	/** Replaces this source's speed multiplier and additive FOV offset. */
	void SetMovementModifier(const UObject* Source, const FPlayerMovementModifier& Modifier) const;

	/** Removes this source's movement modifier. */
	void RemoveMovementModifier(const UObject* Source) const;

	// Stamina
	/** Returns sprint stamina as a fraction from zero to one. */
	[[nodiscard]] float GetStamina() const { return Stamina; }

	/** Returns whether sprinting is blocked while stamina recovers. */
	[[nodiscard]] bool IsStaminaExhausted() const { return bStaminaExhausted; }

	/** Returns whether sprint stamina is enabled for this character. */
	[[nodiscard]] bool IsStaminaEnabled() const { return StaminaSettings.bEnabled; }

	// Interaction
	/** Returns the current interaction target, or nullptr when no valid target is selected. */
	[[nodiscard]] const UInteractTriggerBehavior* GetInteractionTarget() const;

	/** Subscribes to interaction target changes. The event receives nullptr when the target is cleared. */
	[[nodiscard]] FORCEINLINE FOnInteractTargetChanged::RegistrationType& OnInteractTargetChanged()
	{
		return OnInteractTargetChangedEvent;
	}

	virtual bool TeleportTo(const FVector& DestLocation, const FRotator& DestRotation,
		bool bIsATest = false, bool bNoCheck = false) override;

protected:

	// Subobjects
	/** Identifies the player for systems that use actor identity. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Subobjects)
	TObjectPtr<UIdentityBehavior> Identity;

	/** Captures the player's persistent state for save and restore. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Subobjects)
	TObjectPtr<USaveGameBehavior> SaveGame;

	/** Owns the player's items, archives, and equipment. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Subobjects)
	TObjectPtr<UInventoryBehavior> Inventory;

	// Input
	/** Axis2D movement input, with X for strafe and Y for forward. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Axis2D mouse delta, with X for yaw and Y for pitch. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> LookAction;

	/** Boolean action held to run. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> RunAction;

	/** Boolean action held to crouch. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> CrouchAction;

	/** Axis1D lean input. Negative values lean left and positive values lean right. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> LeanAction;

	/** Boolean action that starts interaction on press and ends it on release. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> InteractAction;

	/** Boolean action that uses the currently equipped item on press. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> UseEquipmentAction;

	/** Boolean action that opens the inventory menu on press. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> OpenInventoryAction;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleRun(const FInputActionValue& Value);
	void HandleCrouch(const FInputActionValue& Value);
	void HandleLean(const FInputActionValue& Value);
	void HandleInteract(const FInputActionValue& Value);
	void HandleUseEquipment();
	void HandleOpenInventory();

	// Capabilities
	/** Capabilities available before temporary blocks are applied. */
	UPROPERTY(EditDefaultsOnly, Category = Settings,
		meta = (Bitmask, BitmaskEnum = "/Script/ToroSystems.EPlayerCapability"))
	int32 DefaultCapabilities = static_cast<int32>(EPlayerCapability::All);

	EPlayerCapability BlockedCapabilities = EPlayerCapability::None;
	TSharedRef<TUnorderedAccumulator<EPlayerCapability>> CapabilityBlocks
		= TUnorderedAccumulator<EPlayerCapability>::Make();
	[[nodiscard]] EPlayerCapability GetEffectiveCapabilities() const;
	void OnCapabilityBlocksChanged();

	// Movement
	EPlayerLeanState LeanState = EPlayerLeanState::None;
	EPlayerStateFlags MovementStateFlags = EPlayerStateFlags::None;
	void SetMovementState(EPlayerStateFlags Flags, bool bActive);
	void SetLeanState(EPlayerLeanState Direction);

	/** Base walking speed in cm/s before movement modifiers. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Movement", meta = (ClampMin = 0.0f))
	float WalkSpeed = 350.0f;

	/** Base running speed in cm/s before movement modifiers. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Movement", meta = (ClampMin = 0.0f))
	float RunSpeed = 650.0f;

	/** Base crouched movement speed in cm/s before movement modifiers. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Movement", meta = (ClampMin = 0.0f))
	float CrouchSpeed = 180.0f;

	TSharedRef<TUnorderedAccumulator<FPlayerMovementModifier>> MovementModifiers
		= TUnorderedAccumulator<FPlayerMovementModifier>::Make();
	float MovementFOVOffset = 0.0f;
	void UpdateMovementSpeed();
	void ResetMovementActions(EPlayerCapability LostCapabilities);
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	// Stamina
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Stamina")
	FToroPlayerStaminaSettings StaminaSettings;

	/** Current sprint stamina as a fraction from zero to one. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Player|Stamina")
	float Stamina = 1.0f;

	/** Whether sprinting is blocked until enough stamina has recovered. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Player|Stamina")
	bool bStaminaExhausted = false;

	void UpdateStamina(float DeltaSeconds, bool bMoving);

	// Footsteps
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Footsteps")
	FToroPlayerFootstepSettings FootstepSettings;

	float FootstepProgress = 0.0f;
	void UpdateFootsteps(float DeltaSeconds, bool bMoving);
	void PlayFootstep(float VolumeMultiplier) const;

	// Camera
	/** Mounts the first-person camera and applies the player's turn smoothing. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Subobjects)
	TObjectPtr<USpringArmComponent> CameraArm;

	/** Provides the first-person view and its authored base field of view. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Subobjects)
	TObjectPtr<UCameraComponent> PlayerCamera;

	/** Additional field of view in degrees while running and moving. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Camera")
	float RunFOVOffset = 5.0f;

	/** Shake used for walking and crouched movement. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Camera|Shake")
	TSubclassOf<UCameraShakeBase> WalkCameraShake;

	/** Shake used for sprinting. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Camera|Shake")
	TSubclassOf<UCameraShakeBase> RunCameraShake;

	/** Scale of the walking shake while crouched. Zero disables it while crouched. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Camera|Shake", meta = (ClampMin = "0.0"))
	float CrouchCameraShakeScale = 0.4f;

	/** Rotation lag response at low turn rates. Lower values give more smoothing. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Camera", meta = (ClampMin = 0.1f))
	float CameraRotationLagSpeed = 12.0f;

	/** Rotation lag response during fast turns. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Camera", meta = (ClampMin = 0.1f))
	float FastTurnRotationLagSpeed = 55.0f;

	/** Maximum lateral camera offset in cm when leaning. Geometry can limit the offset. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Camera", meta = (ClampMin = 0.0f))
	float LeanDistance = 100.0f;

	/** Camera roll in degrees at full lean. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Camera")
	float LeanRoll = 8.0f;

	/** How quickly the camera settles after crouching or standing. Zero snaps to the new height. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Camera", meta = (ClampMin = 0.0f))
	float CrouchCameraInterpSpeed = 12.0f;

	float BaseCameraFOV = 90.0f;
	FTransform BaseCameraTransform;
	TOptional<FRotator> PreviousLookRotation;
	bool bSkipCameraRotationLag = false;
	float CurrentLean = 0.0f;
	TOptional<double> BaseCameraArmZ;
	float CrouchCameraOffset = 0.0f;
	TWeakObjectPtr<UCameraShakeBase> MovementCameraShake;
	void UpdatePlayerCamera(float DeltaSeconds);
	void UpdateMovementCameraShake(bool bMoving);
	void StopMovementCameraShake(APlayerCameraManager* CameraManager);
	void UpdateCameraRotationLag(float DeltaSeconds);
	void SetCrouchCameraOffset(float Offset);
	void UpdateCameraLean(float DeltaSeconds);
	float GetLeanClearance(float Direction) const;

	// Interaction
	/** Maximum range of camera-trace interaction. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Interaction", meta = (ClampMin = 0.0f, Units = "cm"))
	float InteractionReach = 250.0f;

	FOnInteractTargetChanged OnInteractTargetChangedEvent;
	TWeakObjectPtr<UInteractTriggerBehavior> InteractionTarget;
	UInteractTriggerBehavior* SearchInteractionTarget();
	void TriggerInteraction() const;
	void StopInteraction() const;

	// Lifecycle
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void UnPossessed() override;
	virtual void EndViewTarget(APlayerController* PC) override;
};
