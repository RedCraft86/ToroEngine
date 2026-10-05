// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Objects/ToroObject.h"
#include "Utilities/WorldGetter.h"
#if WITH_EDITOR
#include "Misc/App.h"
#endif

UToroObject::UToroObject()
	: FTickableGameObject(ETickableTickType::Never)
	, bCanTick(false), bTickWhenPaused(false)
{
}

void UToroObject::SetTickEnabled(bool bEnabled)
{
	bCanTick = bEnabled;
}

bool UToroObject::IsTickEnabled() const
{
	return bCanTick;
}

UWorld* UToroObject::GetWorld() const
{
	return FWorldGetter::Get(GetOuter());
}

void UToroObject::OnConstruction()
{
	ReceiveConstruction();
}

void UToroObject::Tick(float DeltaTime)
{
	ReceiveTick(DeltaTime);
}

void UToroObject::BeginDestroy()
{
	SetTickableTickType(ETickableTickType::Never);
	Super::BeginDestroy();
}

void UToroObject::PostInitProperties()
{
	Super::PostInitProperties();

	OnConstruction();
	SetTickableTickType(ETickableTickType::Conditional);
}

bool UToroObject::IsTickable() const
{
	if (!bCanTick || !IsValid(this) || IsTemplate()
		|| HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
	{
		return false;
	}

	const UWorld* World = GetWorld();
	return IsValid(World) && !World->bIsTearingDown;
}

bool UToroObject::IsTickableWhenPaused() const
{
	return bTickWhenPaused;
}

UWorld* UToroObject::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

TStatId UToroObject::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ToroObject, STATGROUP_Tickables);
}

#if WITH_EDITOR
bool UToroObject::IsTickableInEditor() const
{
	const UWorld* World = GetWorld();
	return bTickInEditor && IsValid(World) && World->IsEditorWorld();
}

void UToroObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (!FApp::IsGame() && IsValid(this) && !IsTemplate()
		&& !HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
	{
		OnConstruction();
	}
}
#endif
