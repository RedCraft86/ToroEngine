// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Components/ToroActorComponent.h"
#if WITH_EDITOR
#include "UObject/Script.h"
#endif

UToroActorComponent::UToroActorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bAllowAnyoneToDestroyMe = true;
}

void UToroActorComponent::OnConstruction()
{
#if WITH_EDITOR
	FEditorScriptExecutionGuard ScriptGuard;
#endif
	ReceiveConstruction();
}

void UToroActorComponent::OnRegister()
{
#if WITH_EDITORONLY_DATA
	bTickInEditor = bShouldTickInEditor;
#endif
	Super::OnRegister();
	OnConstruction();
}

#if WITH_EDITOR
void UToroActorComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (IsValid(this) && !IsTemplate() && !HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
	{
		OnConstruction();
	}
}
#endif