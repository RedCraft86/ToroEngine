// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Actors/ToroVolume.h"
#if WITH_EDITOR
#include "Utilities/ToroValidation.h"
#endif

AToroVolume::AToroVolume(): bActivated(true)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

#if WITH_EDITORONLY_DATA
	bRunConstructionScriptOnDrag = true;
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLOD assuming this is a gameplay actor
	SetCanBeDamaged(false);
}

bool AToroVolume::GetActiveState_Implementation() const
{
	return bActivated;
}

void AToroVolume::SetActiveState_Implementation(bool bNewState)
{
	if (bActivated != bNewState)
	{
		bActivated = bNewState;
		ApplyActiveState(bActivated);
	}
}

void AToroVolume::ApplyActiveState_Implementation(const bool bInState)
{
	SetActorHiddenInGame(!bInState);
	SetActorEnableCollision(bInState);
	SetActorTickEnabled(PrimaryActorTick.bStartWithTickEnabled && bInState);
}

#if WITH_EDITOR
void AToroVolume::ValidateData() const
{
	ToroEngine::Validation::ValidateActor(this);
}
#endif

void AToroVolume::BeginPlay()
{
	Super::BeginPlay();
	ApplyActiveState(bActivated);
}

void AToroVolume::Tick(float DeltaSeconds)
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

#if WITH_EDITOR
bool AToroVolume::ShouldTickIfViewportsOnly() const
{
	return bTickInEditor;
}
#endif
