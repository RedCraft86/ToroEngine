// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Actors/ToroActor.h"
#include "Components/ToroSceneComponent.h"
#if WITH_EDITOR
#include "Utilities/ToroValidation.h"
#endif

AToroActor::AToroActor(): bActivated(true)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SceneRoot = CreateDefaultSubobject<UToroSceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

#if WITH_EDITORONLY_DATA
	bRunConstructionScriptOnDrag = true;
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLOD assuming this is a gameplay actor
	SetCanBeDamaged(false);
}

bool AToroActor::GetActiveState_Implementation() const
{
	return bActivated;
}

void AToroActor::SetActiveState_Implementation(bool bNewState)
{
	if (bActivated != bNewState)
	{
		bActivated = bNewState;
		ApplyActiveState(bActivated);
	}
}

void AToroActor::ApplyActiveState_Implementation(const bool bInState)
{
	SetActorHiddenInGame(!bInState);
	SetActorEnableCollision(bInState);
	SetActorTickEnabled(PrimaryActorTick.bStartWithTickEnabled && bInState);
}

#if WITH_EDITOR
void AToroActor::ValidateData() const
{
	ToroEngine::Validation::ValidateActor(this);
}
#endif

void AToroActor::BeginPlay()
{
	Super::BeginPlay();
	ApplyActiveState(bActivated);
}

void AToroActor::Tick(float DeltaSeconds)
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
bool AToroActor::ShouldTickIfViewportsOnly() const
{
	return bTickInEditor;
}
#endif
