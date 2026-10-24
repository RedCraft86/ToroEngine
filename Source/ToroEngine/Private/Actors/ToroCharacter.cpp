// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Actors/ToroCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#if WITH_EDITOR
#include "UObject/Script.h"
#endif

AToroCharacter::AToroCharacter(): bActivated(true)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

#if WITH_EDITORONLY_DATA
	bRunConstructionScriptOnDrag = true;
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLOD assuming this is a gameplay actor
	SetCanBeDamaged(false);
}

bool AToroCharacter::GetLookTargetLocation_Implementation(FVector& Location) const
{
	Location = FVector::ZeroVector;
	return false;
}

FVector AToroCharacter::GetLookAnchorLocation_Implementation() const
{
	return GetActorTransform().TransformPosition(FVector(0.0, 0.0, BaseEyeHeight));
}

void AToroCharacter::SetControlRotation(const FRotator& NewRotation, bool bIgnoreRoll) const
{
	if (AController* Cont = GetController())
	{
		FRotator DestRotation = NewRotation;
		if (bIgnoreRoll)
		{
			DestRotation.Roll = GetControlRotation().Roll;
		}

		Cont->SetControlRotation(DestRotation);
	}
}

bool AToroCharacter::GetActiveState_Implementation() const
{
	return bActivated;
}

void AToroCharacter::SetActiveState_Implementation(bool bNewState)
{
	if (bActivated != bNewState)
	{
		bActivated = bNewState;
		ApplyActiveState(bActivated);
	}
}

bool AToroCharacter::TeleportTo(const FVector& DestLocation, const FRotator& DestRotation, bool bIsATest, bool bNoCheck)
{
	return Super::TeleportTo(DestLocation, DestRotation, bIsATest, bNoCheck);
}

void AToroCharacter::ApplyActiveState_Implementation(bool bInState)
{
	if (!bInState)
	{
		GetCharacterMovement()->StopMovementImmediately();
	}
}

void AToroCharacter::BeginPlay()
{
	Super::BeginPlay();
	ApplyActiveState(bActivated);
}

void AToroCharacter::Tick(float DeltaSeconds)
{
#if WITH_EDITOR
	if (bTickInEditor && GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
	{
		FEditorScriptExecutionGuard ScriptGuard;
		Super::Tick(DeltaSeconds);
		return;
	}
#endif
	Super::Tick(DeltaSeconds);
}

void AToroCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	BaseEyeHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight_WithoutHemisphere();
}

#if WITH_EDITOR
bool AToroCharacter::ShouldTickIfViewportsOnly() const
{
	return bTickInEditor;
}
#endif