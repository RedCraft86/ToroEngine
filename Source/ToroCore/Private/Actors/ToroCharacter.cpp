// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Actors/ToroCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AToroCharacter::AToroCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

#if WITH_EDITORONLY_DATA
	bRunConstructionScriptOnDrag = true;
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLOD assuming this is a gameplay actor
	SetCanBeDamaged(false);
}

bool AToroCharacter::GetLookTarget_Implementation(FVector& Location) const
{
	Location = FVector::ZeroVector;
	return false;
}

FVector AToroCharacter::GetFocusPoint_Implementation() const
{
	return GetActorLocation() + (GetActorScale3D() * GetActorUpVector() * BaseEyeHeight);
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
	if (Super::TeleportTo(DestLocation, DestRotation, bIsATest, bNoCheck))
	{
		if (!bIsATest)
		{
			SetControlRotation(DestRotation, true);
		}

		return true;
	}

	return false;
}

void AToroCharacter::ApplyActiveState_Implementation(const bool bInState)
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
		TGuardValue<bool> AllowScript(GAllowActorScriptExecutionInEditor, true);
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
