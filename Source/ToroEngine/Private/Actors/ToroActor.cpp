// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Actors/ToroActor.h"
#include "Components/ToroSceneComponent.h"
#include "Engine/World.h"
#if WITH_EDITOR
#include "UObject/Script.h"
#endif

AToroActor::AToroActor(): bActivated(true)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<UToroSceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

#if WITH_EDITORONLY_DATA
	bRunConstructionScriptOnDrag = true;
#endif

	bIsSpatiallyLoaded = false; // Stop World Partition from unloading this
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

void AToroActor::ApplyActiveState_Implementation(bool bInState)
{
	SetActorHiddenInGame(!bInState);
	SetActorEnableCollision(bInState);
	SetActorTickEnabled(PrimaryActorTick.bStartWithTickEnabled && bInState);
}

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
		FEditorScriptExecutionGuard ScriptGuard;
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