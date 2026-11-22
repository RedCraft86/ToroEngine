// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

// ReSharper disable CppMemberFunctionMayBeConst
#include "Framework/ToroPlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ActorBehaviors/Misc/IdentityBehavior.h"
#include "ActorBehaviors/Trigger/InteractTriggerBehavior.h"
#include "Inventory/Widgets/ToroInventoryWidget.h"
#include "Inventory/InventoryBehavior.h"
#include "Characters/ToroCharacterDatabase.h"
#include "DataTypes/ToroLevelInfo.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "DataTypes/ToroPlayerCameraShakes.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "SaveSystem/SaveGameBehavior.h"
#include "UObject/ConstructorHelpers.h"
#include "UserSettings/ToroGameUserSettings.h"

AToroPlayerCharacter::AToroPlayerCharacter()
{
	CapabilityBlocks->OnChanged().AddUObject(this, &AToroPlayerCharacter::OnCapabilityBlocksChanged);
	MovementModifiers->OnChanged().AddUObject(this, &AToroPlayerCharacter::UpdateMovementSpeed);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(GetCapsuleComponent());
	CameraArm->SetRelativeLocation(FVector(0.0, 0.0, BaseEyeHeight));
	CameraArm->TargetArmLength = 0.0f;
	CameraArm->bDoCollisionTest = false;
	CameraArm->bUsePawnControlRotation = true;
	CameraArm->bEnableCameraLag = false;
	CameraArm->bEnableCameraRotationLag = false;
	CameraArm->bUseCameraLagSubstepping = true;
	CameraArm->CameraLagMaxTimeStep = 1.0f / 120.0f;
	CameraArm->CameraRotationLagSpeed = CameraRotationLagSpeed;

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	PlayerCamera->bUsePawnControlRotation = false;

	EquipmentRoot = CreateDefaultSubobject<USceneComponent>(TEXT("EquipmentRoot"));
	EquipmentRoot->SetupAttachment(PlayerCamera);

	WalkCameraShake = UToroWalkCameraShake::StaticClass();
	RunCameraShake = UToroRunCameraShake::StaticClass();
	PlayerCamera->bConstrainAspectRatio = false;

	Identity = CreateDefaultSubobject<UIdentityBehavior>(TEXT("Identity"));
	Identity->Identity.Group = ToroCharacterTags::Player.GetTag();
	Identity->Determinant = TEXT("MainPlayer");
	Identity->RebuildIdentity();

	SaveGame = CreateDefaultSubobject<USaveGameBehavior>(TEXT("SaveGame"));
	Inventory = CreateDefaultSubobject<UInventoryBehavior>(TEXT("Inventory"));

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->SetCrouchedHalfHeight(88.0f * 0.3f);

	// Inputs
	{
		static const ConstructorHelpers::FObjectFinder<UInputAction> MoveFinder(
		   TEXT("/ToroEngine/Inputs/IA_Move.IA_Move"));
		if (MoveFinder.Succeeded()) MoveAction = MoveFinder.Object;

		static const ConstructorHelpers::FObjectFinder<UInputAction> LookFinder(
			TEXT("/ToroEngine/Inputs/IA_Look.IA_Look"));
		if (LookFinder.Succeeded()) LookAction = LookFinder.Object;

		static const ConstructorHelpers::FObjectFinder<UInputAction> RunFinder(
			TEXT("/ToroEngine/Inputs/IA_Run.IA_Run"));
		if (RunFinder.Succeeded()) RunAction = RunFinder.Object;

		static const ConstructorHelpers::FObjectFinder<UInputAction> CrouchFinder(
			TEXT("/ToroEngine/Inputs/IA_Crouch.IA_Crouch"));
		if (CrouchFinder.Succeeded()) CrouchAction = CrouchFinder.Object;

		static const ConstructorHelpers::FObjectFinder<UInputAction> LeanFinder(
			TEXT("/ToroEngine/Inputs/IA_Lean.IA_Lean"));
		if (LeanFinder.Succeeded()) LeanAction = LeanFinder.Object;

		static const ConstructorHelpers::FObjectFinder<UInputAction> InteractFinder(
			TEXT("/ToroEngine/Inputs/IA_Interact.IA_Interact"));
		if (InteractFinder.Succeeded()) InteractAction = InteractFinder.Object;

		static const ConstructorHelpers::FObjectFinder<UInputAction> UseEquipmentFinder(
			TEXT("/ToroEngine/Inputs/IA_UseEquipment.IA_UseEquipment"));
		if (UseEquipmentFinder.Succeeded()) UseEquipmentAction = UseEquipmentFinder.Object;

		static const ConstructorHelpers::FObjectFinder<UInputAction> OpenInventoryFinder(
			TEXT("/ToroEngine/Inputs/IA_OpenInventory.IA_OpenInventory"));
		if (OpenInventoryFinder.Succeeded()) OpenInventoryAction = OpenInventoryFinder.Object;
	}

	AutoPossessPlayer = EAutoReceiveInput::Player0;

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false; // Stop World Partition from unloading this
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLOD
	SetCanBeDamaged(false);
}

ACharacter* AToroPlayerCharacter::Get(const UObject* ContextObject, TSubclassOf<ACharacter> Class, int32 PlayerIdx)
{
	ACharacter* PC = UGameplayStatics::GetPlayerCharacter(ContextObject, PlayerIdx);
	return IsValid(PC) && (!IsValid(Class) || PC->IsA(Class)) ? PC : nullptr;
}

// Capabilities

bool AToroPlayerCharacter::HasCapability(EPlayerCapability Capability) const
{
	return EnumHasAllFlags(GetEffectiveCapabilities(), Capability);
}

void AToroPlayerCharacter::BlockCapabilities(const UObject* Source, EPlayerCapability Mask) const
{
	if (Mask == EPlayerCapability::None)
	{
		ReleaseCapabilityBlock(Source);
		return;
	}

	CapabilityBlocks->Add(Source, Mask);
}

void AToroPlayerCharacter::ReleaseCapabilityBlock(const UObject* Source) const
{
	CapabilityBlocks->Remove(Source);
}

// Player state

bool AToroPlayerCharacter::HasPlayerState(EPlayerStateFlags State) const
{
	return EnumHasAllFlags(GetPlayerStateFlags(), State);
}

EPlayerStateFlags AToroPlayerCharacter::GetPlayerStateFlags() const
{
	EPlayerStateFlags State = MovementStateFlags;
	if (const UInteractTriggerBehavior* Target = InteractionTarget.Get(); Target && Target->IsTriggered())
	{
		State |= EPlayerStateFlags::Interacting;
	}
	return State;
}

bool AToroPlayerCharacter::IsMoving() const
{
	return HasPlayerState(EPlayerStateFlags::Moving)
		&& GetVelocity().SizeSquared2D() > FMath::Square(WalkSpeed * 0.5f);
}

// Movement

void AToroPlayerCharacter::SetMovementModifier(const UObject* Source, const FPlayerMovementModifier& Modifier) const
{
	MovementModifiers->Add(Source, Modifier);
}

void AToroPlayerCharacter::RemoveMovementModifier(const UObject* Source) const
{
	MovementModifiers->Remove(Source);
}

// Interaction

const UInteractTriggerBehavior* AToroPlayerCharacter::GetInteractionTarget() const
{
	return InteractionTarget.Get();
}

// Camera

bool AToroPlayerCharacter::TeleportTo(const FVector& DestLocation, const FRotator& DestRotation,
	bool bIsATest, bool bNoCheck)
{
	if (bIsATest)
	{
		return Super::TeleportTo(DestLocation, DestRotation, bIsATest, bNoCheck);
	}

	const bool bWasRotationLagEnabled = CameraArm->bEnableCameraRotationLag;
	CameraArm->bEnableCameraRotationLag = false;
	const bool bTeleported = Super::TeleportTo(DestLocation, DestRotation, bIsATest, bNoCheck);
	if (bTeleported)
	{
		PreviousLookRotation.Reset();
		bSkipCameraRotationLag = true;
	}
	else
	{
		CameraArm->bEnableCameraRotationLag = bWasRotationLagEnabled && !bSkipCameraRotationLag;
	}
	return bTeleported;
}

// Input

void AToroPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		return;
	}
	const auto BindValue = [this, Input](const UInputAction* Action, ETriggerEvent Event, auto Handler, bool bBindRelease)
	{
		if (!Action)
		{
			return;
		}

		Input->BindAction(Action, Event, this, Handler);
		if (bBindRelease)
		{
			const auto Release = [this, Handler](const FInputActionValue& Value)
			{
				FInputActionValue ReleasedValue = Value;
				ReleasedValue.Reset();
				(this->*Handler)(ReleasedValue);
			};
			Input->BindActionValueLambda(Action, ETriggerEvent::Completed, Release);
			Input->BindActionValueLambda(Action, ETriggerEvent::Canceled, Release);
		}
	};

	BindValue(MoveAction, ETriggerEvent::Triggered, &AToroPlayerCharacter::HandleMove, true);
	BindValue(LookAction, ETriggerEvent::Triggered, &AToroPlayerCharacter::HandleLook, false);
	BindValue(RunAction, ETriggerEvent::Started, &AToroPlayerCharacter::HandleRun, true);
	BindValue(CrouchAction, ETriggerEvent::Started, &AToroPlayerCharacter::HandleCrouch, true);
	BindValue(LeanAction, ETriggerEvent::Triggered, &AToroPlayerCharacter::HandleLean, true);
	BindValue(InteractAction, ETriggerEvent::Started, &AToroPlayerCharacter::HandleInteract, true);

	if (UseEquipmentAction)
	{
		Input->BindAction(UseEquipmentAction, ETriggerEvent::Started, this, &AToroPlayerCharacter::HandleUseEquipment);
	}
	if (OpenInventoryAction)
	{
		Input->BindAction(OpenInventoryAction, ETriggerEvent::Started, this, &AToroPlayerCharacter::HandleOpenInventory);
	}
}

void AToroPlayerCharacter::HandleMove(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	if (Input.IsNearlyZero())
	{
		SetMovementState(EPlayerStateFlags::Moving, false);
		return;
	}
	if (!Controller || !HasCapability(EPlayerCapability::Move))
	{
		return;
	}

	const FRotator YawRotation(0.0, Controller->GetControlRotation().Yaw, 0.0);
	const FVector WorldInput = YawRotation.RotateVector(FVector(Input.Y, Input.X, 0.0));
	AddMovementInput(WorldInput.GetSafeNormal(), FMath::Min(WorldInput.Size(), 1.0));
	SetMovementState(EPlayerStateFlags::Moving, true);
}

void AToroPlayerCharacter::HandleLook(const FInputActionValue& Value)
{
	FVector2D Delta = Value.Get<FVector2D>();
	if (const auto* Settings = UToroGameUserSettings::Get())
	{
		Delta.X *= Settings->GetSensitivityX() * (Settings->GetInvertMouseX() ? -1.0f : 1.0f);
		Delta.Y *= Settings->GetSensitivityY() * (Settings->GetInvertMouseY() ? -1.0f : 1.0f);
	}
	if (Controller && !Controller->IsLookInputIgnored() && HasCapability(EPlayerCapability::Look))
	{
		AddControllerYawInput(Delta.X);
		AddControllerPitchInput(Delta.Y);
	}
}

void AToroPlayerCharacter::HandleRun(const FInputActionValue& Value)
{
	const bool bRunning = Value.Get<bool>() && Controller && HasCapability(EPlayerCapability::Run)
		&& (!StaminaSettings.bEnabled || !bStaminaExhausted)
		&& !bIsCrouched && !GetCharacterMovement()->bWantsToCrouch;
	if (HasPlayerState(EPlayerStateFlags::Running) != bRunning)
	{
		SetMovementState(EPlayerStateFlags::Running, bRunning);
		UpdateMovementSpeed();
	}
}

void AToroPlayerCharacter::HandleCrouch(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		UnCrouch();
		return;
	}
	if (!Controller || !HasCapability(EPlayerCapability::Crouch))
	{
		return;
	}
	if (!GetCharacterMovement()->bWantsToCrouch)
	{
		ResetMovementActions(EPlayerCapability::Run);
		Crouch();
	}
}

void AToroPlayerCharacter::HandleLean(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();
	if (FMath::IsNearlyZero(Input))
	{
		SetLeanState(EPlayerLeanState::None);
		return;
	}
	if (!Controller || !HasCapability(EPlayerCapability::Lean))
	{
		return;
	}

	SetLeanState(Input < 0.0f ? EPlayerLeanState::Left : EPlayerLeanState::Right);
}

void AToroPlayerCharacter::HandleInteract(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		TriggerInteraction();
	}
	else
	{
		StopInteraction();
	}
}

void AToroPlayerCharacter::HandleUseEquipment()
{
	Inventory->UseEquipment();
}

void AToroPlayerCharacter::HandleOpenInventory()
{
	UToroInventoryWidget::Push(this);
}

// Capability updates

EPlayerCapability AToroPlayerCharacter::GetEffectiveCapabilities() const
{
	EPlayerCapability Effective = static_cast<EPlayerCapability>(DefaultCapabilities) & ~BlockedCapabilities;
	if (!EnumHasAnyFlags(Effective, EPlayerCapability::Move))
	{
		Effective &= ~EPlayerCapability::Run;
	}
	return Effective;
}

void AToroPlayerCharacter::OnCapabilityBlocksChanged()
{
	const EPlayerCapability Previous = GetEffectiveCapabilities();
	BlockedCapabilities = EPlayerCapability::None;
	CapabilityBlocks->ForEachValue([this](const EPlayerCapability& Mask)
	{
		BlockedCapabilities |= Mask;
		return true;
	});

	const EPlayerCapability Effective = GetEffectiveCapabilities();
	if (Previous == Effective)
	{
		return;
	}

	if (APlayerController* PC = GetController<APlayerController>(); PC && PC->IsLocalController())
	{
		PC->FlushPressedKeys();
	}

	const EPlayerCapability LostCapabilities = Previous & ~Effective;
	ResetMovementActions(LostCapabilities);
	if (EnumHasAnyFlags(LostCapabilities, EPlayerCapability::Interact))
	{
		SearchInteractionTarget();
	}
}

// Movement updates

void AToroPlayerCharacter::SetMovementState(EPlayerStateFlags Flags, bool bActive)
{
	constexpr EPlayerStateFlags MovementFlags =
		EPlayerStateFlags::Moving
		| EPlayerStateFlags::Running
		| EPlayerStateFlags::Crouching
		| EPlayerStateFlags::Leaning;

	Flags &= MovementFlags;
	if (bActive)
	{
		MovementStateFlags |= Flags;
	}
	else
	{
		MovementStateFlags &= ~Flags;
	}
}

void AToroPlayerCharacter::SetLeanState(EPlayerLeanState Direction)
{
	LeanState = Direction;
	SetMovementState(EPlayerStateFlags::Leaning, Direction != EPlayerLeanState::None);
}

void AToroPlayerCharacter::UpdateMovementSpeed()
{
	float SpeedMultiplier = 1.0f;
	MovementFOVOffset = 0.0f;
	MovementModifiers->ForEachValue([&](const FPlayerMovementModifier& Modifier)
	{
		SpeedMultiplier *= FMath::Max(0.0f, Modifier.SpeedMultiplier);
		MovementFOVOffset += Modifier.FOVOffset;
		return true;
	});

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = (HasPlayerState(EPlayerStateFlags::Running) ? RunSpeed : WalkSpeed) * SpeedMultiplier;
	Movement->MaxWalkSpeedCrouched = CrouchSpeed * SpeedMultiplier;
}

void AToroPlayerCharacter::ResetMovementActions(EPlayerCapability LostCapabilities)
{
	if (EnumHasAnyFlags(LostCapabilities, EPlayerCapability::Move))
	{
		SetMovementState(EPlayerStateFlags::Moving, false);
		ConsumeMovementInputVector();
	}
	if (EnumHasAnyFlags(LostCapabilities, EPlayerCapability::Run))
	{
		SetMovementState(EPlayerStateFlags::Running, false);
		UpdateMovementSpeed();
	}
	if (EnumHasAnyFlags(LostCapabilities, EPlayerCapability::Crouch))
	{
		UnCrouch();
	}
	if (EnumHasAnyFlags(LostCapabilities, EPlayerCapability::Lean))
	{
		SetLeanState(EPlayerLeanState::None);
	}
	if (EnumHasAnyFlags(LostCapabilities, EPlayerCapability::Look))
	{
		PreviousLookRotation.Reset();
	}
}

void AToroPlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (CrouchCameraInterpSpeed > 0.0f && GetCharacterMovement()->bCrouchMaintainsBaseLocation)
	{
		SetCrouchCameraOffset(CrouchCameraOffset + HalfHeightAdjust);
	}

	if (HasPlayerState(EPlayerStateFlags::Running))
	{
		ResetMovementActions(EPlayerCapability::Run);
	}
	SetMovementState(EPlayerStateFlags::Crouching, true);
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void AToroPlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (CrouchCameraInterpSpeed > 0.0f && GetCharacterMovement()->bCrouchMaintainsBaseLocation)
	{
		SetCrouchCameraOffset(CrouchCameraOffset - HalfHeightAdjust);
	}

	SetMovementState(EPlayerStateFlags::Crouching, false);
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

// Stamina

void AToroPlayerCharacter::UpdateStamina(float DeltaSeconds, bool bMoving)
{
	if (!StaminaSettings.bEnabled)
	{
		return;
	}

	if (bMoving && HasPlayerState(EPlayerStateFlags::Running))
	{
		Stamina = FMath::Max(0.0f, Stamina - StaminaSettings.DrainRate * DeltaSeconds);
		if (Stamina == 0.0f)
		{
			bStaminaExhausted = true;
			ResetMovementActions(EPlayerCapability::Run);
		}
		return;
	}

	Stamina = FMath::Min(1.0f, Stamina + StaminaSettings.RecoveryRate * DeltaSeconds);
	if (Stamina >= StaminaSettings.SprintRecoveryThreshold)
	{
		bStaminaExhausted = false;
	}
}

// Footsteps

void AToroPlayerCharacter::UpdateFootsteps(float DeltaSeconds, bool bMoving)
{
	if (!bMoving || !GetCharacterMovement()->IsMovingOnGround()
		|| (!FootstepSettings.DefaultSound && FootstepSettings.SurfaceSounds.IsEmpty()))
	{
		FootstepProgress = 0.0f;
		return;
	}

	float Interval = FootstepSettings.WalkInterval;
	float VolumeMultiplier = 1.0f;
	if (HasPlayerState(EPlayerStateFlags::Running))
	{
		Interval = FootstepSettings.RunInterval;
		VolumeMultiplier = FootstepSettings.RunVolumeMultiplier;
	}
	else if (HasPlayerState(EPlayerStateFlags::Crouching))
	{
		Interval = FootstepSettings.CrouchInterval;
		VolumeMultiplier = FootstepSettings.CrouchVolumeMultiplier;
	}

	FootstepProgress += DeltaSeconds / FMath::Max(0.01f, Interval);
	if (FootstepProgress >= 1.0f)
	{
		FootstepProgress = FMath::Fmod(FootstepProgress, 1.0f);
		PlayFootstep(VolumeMultiplier);
	}
}

void AToroPlayerCharacter::PlayFootstep(float VolumeMultiplier) const
{
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	const FVector Start = GetActorLocation();
	const FVector End = Start + Movement->GetGravityDirection()
		* (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 20.0);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(PlayerFootstep), true, this);
	Params.bReturnPhysicalMaterial = true;
	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, FootstepSettings.TraceChannel, Params))
	{
		Hit = Movement->CurrentFloor.HitResult;
	}

	const EPhysicalSurface Surface = UGameplayStatics::GetSurfaceType(Hit);
	USoundBase* Sound = FootstepSettings.SurfaceSounds.FindRef(Surface);
	if (!Sound)
	{
		Sound = FootstepSettings.DefaultSound;
	}
	if (Sound)
	{
		const FVector Location = Hit.IsValidBlockingHit() ? FVector(Hit.ImpactPoint) : End;
		UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, FootstepSettings.Volume * VolumeMultiplier);
	}
}

// Camera updates

void AToroPlayerCharacter::UpdatePlayerCamera(float DeltaSeconds)
{
	UpdateCameraRotationLag(DeltaSeconds);

	const bool bSprinting = HasPlayerState(EPlayerStateFlags::Running) && IsMoving();
	const float SprintOffset = bSprinting ? RunFOVOffset : 0.0f;
	const float TargetFOV = FMath::Clamp(BaseCameraFOV + MovementFOVOffset + SprintOffset, 5.0f, 170.0f);
	PlayerCamera->SetFieldOfView(FMath::FInterpTo(PlayerCamera->FieldOfView, TargetFOV, DeltaSeconds, 10.0f));

	if (CrouchCameraOffset != 0.0f)
	{
		SetCrouchCameraOffset(FMath::FInterpTo(CrouchCameraOffset, 0.0f, DeltaSeconds, CrouchCameraInterpSpeed));
	}

	UpdateCameraLean(DeltaSeconds);
}

void AToroPlayerCharacter::UpdateMovementCameraShake(bool bMoving)
{
	const APlayerController* PC = GetController<APlayerController>();
	APlayerCameraManager* CM = PC ? PC->PlayerCameraManager.Get() : nullptr;
	if (!PC || !PC->IsLocalController() || !bMoving || !GetCharacterMovement()->IsMovingOnGround() || !CM
		|| CM->ViewTarget.Target != this || CM->PendingViewTarget.Target)
	{
		StopMovementCameraShake(CM);
		return;
	}

	const bool bRunning = HasPlayerState(EPlayerStateFlags::Running);
	const TSubclassOf<UCameraShakeBase> ShakeClass = bRunning ? RunCameraShake : WalkCameraShake;
	const float Scale = bIsCrouched ? CrouchCameraShakeScale : 1.0f;
	if (!ShakeClass || Scale <= 0.0f)
	{
		StopMovementCameraShake(CM);
		return;
	}

	if (UCameraShakeBase* Shake = MovementCameraShake.Get();
		Shake && Shake->IsActive() && Shake->GetClass() == ShakeClass.Get())
	{
		Shake->ShakeScale = Scale;
		return;
	}

	StopMovementCameraShake(CM);
	MovementCameraShake = CM->StartCameraShake(ShakeClass, Scale);
}

void AToroPlayerCharacter::StopMovementCameraShake(APlayerCameraManager* CameraManager)
{
	if (UCameraShakeBase* Shake = MovementCameraShake.Get(); CameraManager && Shake)
	{
		CameraManager->StopCameraShake(Shake, false);
	}
	MovementCameraShake.Reset();
}

void AToroPlayerCharacter::UpdateCameraRotationLag(float DeltaSeconds)
{
	const auto* Settings = UToroGameUserSettings::Get();
	const float Smoothing = Settings ? Settings->GetSmoothTurns() : 1.0f;
	CameraArm->bEnableCameraRotationLag =
		!bSkipCameraRotationLag
		&& IsLocallyControlled()
		&& Smoothing > UE_SMALL_NUMBER;
	bSkipCameraRotationLag = false;

	const FRotator ControlRotation = GetControlRotation();
	const FRotator RotationDelta = (ControlRotation - PreviousLookRotation.Get(ControlRotation)).GetNormalized();
	PreviousLookRotation = ControlRotation;
	if (!CameraArm->bEnableCameraRotationLag)
	{
		return;
	}

	const double TurnDelta = FMath::Max(FMath::Abs(RotationDelta.Yaw), FMath::Abs(RotationDelta.Pitch));
	const float TurnRate = static_cast<float>(TurnDelta / FMath::Max(DeltaSeconds, UE_SMALL_NUMBER));
	const FVector2D LagSpeedRange(CameraRotationLagSpeed, FastTurnRotationLagSpeed);
	const float BaseLagSpeed = FMath::GetMappedRangeValueClamped(FVector2D(540.0f, 900.0f), LagSpeedRange, TurnRate);
	const float LagSpeed = BaseLagSpeed / Smoothing;
	CameraArm->CameraRotationLagSpeed = FMath::Max(LagSpeed,
		FMath::FInterpTo(CameraArm->CameraRotationLagSpeed, LagSpeed, DeltaSeconds, 10.0f)
	);
}

void AToroPlayerCharacter::SetCrouchCameraOffset(float Offset)
{
	FVector ArmLocation = CameraArm->GetRelativeLocation();
	if (!BaseCameraArmZ.IsSet())
	{
		BaseCameraArmZ = ArmLocation.Z;
	}
	ArmLocation.Z = BaseCameraArmZ.GetValue() + Offset;
	CrouchCameraOffset = Offset;
	CameraArm->SetRelativeLocation(ArmLocation);
}

void AToroPlayerCharacter::UpdateCameraLean(float DeltaSeconds)
{
	float TargetLean = 0.0f;
	if (LeanState == EPlayerLeanState::Left)
	{
		TargetLean = -1.0f;
	}
	else if (LeanState == EPlayerLeanState::Right)
	{
		TargetLean = 1.0f;
	}

	if (TargetLean == 0.0f && FMath::IsNearlyZero(CurrentLean))
	{
		return;
	}

	CurrentLean = FMath::FInterpTo(CurrentLean, TargetLean, DeltaSeconds, 12.0f);
	const float VisibleLean = CurrentLean * GetLeanClearance(CurrentLean);
	PlayerCamera->SetRelativeLocationAndRotation(
		BaseCameraTransform.GetLocation() + FVector(0.0, LeanDistance * VisibleLean, 0.0),
		BaseCameraTransform.Rotator() + FRotator(0.0, 0.0, LeanRoll * VisibleLean));
}

float AToroPlayerCharacter::GetLeanClearance(float Direction) const
{
	const FTransform Socket = CameraArm->GetSocketTransform(USpringArmComponent::SocketName);
	const FVector CameraOrigin = BaseCameraTransform.GetLocation();
	const FVector Start = Socket.TransformPosition(CameraOrigin);
	const FVector End = Socket.TransformPosition(CameraOrigin + FVector(0.0, LeanDistance * Direction, 0.0));
	const FVector Sweep = End - Start;
	const double SweepDistance = Sweep.Size();
	if (SweepDistance <= UE_SMALL_NUMBER)
	{
		return 1.0f;
	}

	constexpr double WallPadding = 10.0;
	const FVector ProbeEnd = End + Sweep * (WallPadding / SweepDistance);
	FHitResult Hit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(PlayerLean), false, this);
	if (!GetWorld()->SweepSingleByChannel(Hit, Start, ProbeEnd, FQuat::Identity, ECC_Camera,
		FCollisionShape::MakeSphere(8.0f), Params))
	{
		return 1.0f;
	}

	return FMath::Clamp((Hit.Distance - WallPadding) / SweepDistance, 0.0, 1.0);
}

// Interaction

UInteractTriggerBehavior* AToroPlayerCharacter::SearchInteractionTarget()
{
	TWeakObjectPtr<UInteractTriggerBehavior> Candidate;
	if (HasCapability(EPlayerCapability::Interact))
	{
		FHitResult Hit;
		if (const APlayerController* PC = GetController<APlayerController>())
		{
			if (PC->ShouldShowMouseCursor())
			{
				PC->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), false, Hit);
			}
			else
			{
				FVector Start;
				FRotator Rotation;
				PC->GetPlayerViewPoint(Start, Rotation);
				const FVector End = Start + Rotation.Vector() * InteractionReach;
				GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility,
					FCollisionQueryParams(SCENE_QUERY_STAT(Interaction), false, this));
			}
		}

		if (const AActor* HitActor = Hit.GetActor())
		{
			UInteractTriggerBehavior* HitTarget = HitActor->FindComponentByClass<UInteractTriggerBehavior>();
			if (HitTarget && HitTarget->CanBeInteractedWith())
			{
				Candidate = HitTarget;
			}
		}
	}

	if (InteractionTarget.HasSameIndexAndSerialNumber(Candidate))
	{
		return InteractionTarget.Get();
	}

	// Ending an interaction can change the target or invalidate the next one.
	const TWeakObjectPtr<UInteractTriggerBehavior> PreviousTarget = InteractionTarget;

	StopInteraction();
	if (!InteractionTarget.HasSameIndexAndSerialNumber(PreviousTarget))
	{
		return InteractionTarget.Get();
	}

	UInteractTriggerBehavior* NextTarget = Candidate.Get();
	if (!HasCapability(EPlayerCapability::Interact) || !NextTarget || !NextTarget->CanBeInteractedWith())
	{
		NextTarget = nullptr;
	}

	InteractionTarget = NextTarget;
	OnInteractTargetChangedEvent.Broadcast(NextTarget);
	return InteractionTarget.Get();
}

void AToroPlayerCharacter::TriggerInteraction() const
{
	if (HasCapability(EPlayerCapability::Interact) && InteractionTarget.IsValid() && InteractionTarget->CanBeInteractedWith())
	{
		InteractionTarget->StartTrigger();
	}
}

void AToroPlayerCharacter::StopInteraction() const
{
	if (InteractionTarget.IsValid())
	{
		InteractionTarget->EndTrigger();
	}
}

// Lifecycle

void AToroPlayerCharacter::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);
	if (EquipmentRoot && EquipmentRoot->IsRegistered())
	{
		UpdateEquipmentRoot();
	}
}

void AToroPlayerCharacter::UpdateEquipmentRoot()
{
	const APlayerController* PC = GetController<APlayerController>();
	const APlayerCameraManager* CameraManager = PC && PC->IsLocalController() ? PC->PlayerCameraManager.Get() : nullptr;
	USceneComponent* CameraRoot = IsValid(CameraManager) ? CameraManager->GetRootComponent() : nullptr;
	USceneComponent* Parent = CameraRoot ? CameraRoot : PlayerCamera.Get();
	if (EquipmentRoot->GetAttachParent() != Parent)
	{
		EquipmentRoot->AttachToComponent(Parent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	const bool bHideEquipment = IsHidden() || !CameraRoot || CameraManager->GetViewTarget() != this;
	if (EquipmentRoot->bHiddenInGame != bHideEquipment)
	{
		EquipmentRoot->SetHiddenInGame(bHideEquipment, true);
	}
}

void AToroPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateEquipmentRoot();
	BaseCameraFOV = PlayerCamera->FieldOfView;
	BaseCameraTransform = PlayerCamera->GetRelativeTransform();
	CameraArm->AddTickPrerequisiteActor(this);
	UpdateMovementSpeed();

	if (const FToroLevelInfo* LevelInfo = FToroLevelInfo::Get(this);
		LevelInfo && LevelInfo->LevelType == LevelTypeTags::MainMenu)
	{
		BlockCapabilities(this, EPlayerCapability::All);
	}
}

void AToroPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const bool bLocalPlayer = IsLocallyControlled();
	const bool bMoving = bLocalPlayer && IsMoving();
	if (bLocalPlayer)
	{
		UpdateStamina(DeltaSeconds, bMoving);
		UpdateFootsteps(DeltaSeconds, bMoving);
	}

	UpdatePlayerCamera(DeltaSeconds);
	UpdateEquipmentRoot();
	UpdateMovementCameraShake(bMoving);
	if (bLocalPlayer)
	{
		SearchInteractionTarget();
	}
}

void AToroPlayerCharacter::UnPossessed()
{
	const APlayerController* PC = GetController<APlayerController>();
	StopMovementCameraShake(PC ? PC->PlayerCameraManager.Get() : nullptr);
	FootstepProgress = 0.0f;
	ResetMovementActions(EPlayerCapability::All);
	Super::UnPossessed();
	UpdateEquipmentRoot();
	SearchInteractionTarget();
}

void AToroPlayerCharacter::EndViewTarget(APlayerController* PC)
{
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StopAllCameraShakes(true);
	}
	if (PC == GetController())
	{
		MovementCameraShake.Reset();
	}
	Super::EndViewTarget(PC);
}
