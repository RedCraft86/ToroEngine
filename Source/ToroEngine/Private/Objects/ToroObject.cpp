// Copyright (C) RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Objects/ToroObject.h"
#include "Utilities/WorldGetter.h"
#include "Engine/World.h"
#include "Stats/Stats.h"
#if WITH_EDITOR
#include "UObject/Script.h"
#endif

UToroObject::UToroObject()
	: FTickableGameObject(ETickableTickType::Never)
	, bCanEverTick(false)
	, bTickWhenPaused(false)
{
}

void UToroObject::SetTickEnabled(bool bEnabled)
{
	bCanEverTick = bEnabled;
}

bool UToroObject::IsTickEnabled() const
{
	return bCanEverTick;
}

UWorld* UToroObject::GetWorld() const
{
	return IsTemplate() ? nullptr : FWorldGetter::Get(GetOuter());
}

void UToroObject::OnConstruction()
{
#if WITH_EDITOR
	FEditorScriptExecutionGuard ScriptGuard;
#endif
	ReceiveConstruction();
}

void UToroObject::OnInitialize()
{
#if WITH_EDITOR
	FEditorScriptExecutionGuard ScriptGuard;
#endif
	ReceiveInitialize();
}

void UToroObject::Tick(float DeltaTime)
{
#if WITH_EDITOR
	FEditorScriptExecutionGuard ScriptGuard;
#endif
	ReceiveTick(DeltaTime);
}

void UToroObject::BeginDestroy()
{
	if (InitHandle.IsValid())
	{
		FTSTicker::RemoveTicker(InitHandle);
		InitHandle.Reset();
	}

	bConstructed = false;
	bInitialized = false;
	SetTickableTickType(ETickableTickType::Never);
	Super::BeginDestroy();
}

void UToroObject::PostInitProperties()
{
	Super::PostInitProperties();
	if (IsSafeObject())
	{
		InitHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UToroObject::TryInitialize)
		);
	}
}

bool UToroObject::IsTickable() const
{
	if (bConstructed && bCanEverTick && IsSafeObject())
	{
		const UWorld* World = GetWorld();
		return IsValid(World) && !World->bIsTearingDown && (!World->IsGameWorld() || bInitialized);
	}

	return false;
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
	if (bTickInEditor)
	{
		const UWorld* World = GetWorld();
		return IsValid(World) && !World->IsGameWorld();
	}

	return false;
}

void UToroObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (IsSafeObject())
	{
		const UWorld* World = GetWorld();
		if (IsValid(World) && !World->bIsTearingDown && !World->IsGameWorld())
		{
			OnConstruction();
		}
	}
}
#endif

bool UToroObject::TryInitialize(float)
{
	if (bInitialized || !IsSafeObject())
	{
		InitHandle.Reset();
		return false;
	}

	for (const UObject* Obj = this; Obj; Obj = Obj->GetOuter())
	{
		if (!IsValid(Obj) || Obj->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
		{
			InitHandle.Reset();
			return false;
		}

		if (Obj->HasAnyInternalFlags(EInternalObjectFlags_AsyncLoading)
			|| Obj->HasAnyFlags(RF_NeedInitialization | RF_NeedLoad | RF_NeedPostLoad
				| RF_NeedPostLoadSubobjects))
		{
			// Not ready to init
			return true;
		}
	}

	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return true;
	}
	if (World->bIsTearingDown)
	{
		InitHandle.Reset();
		return false;
	}

	if (!bConstructed)
	{
		bConstructed = true;
		OnConstruction();

		if (!IsSafeObject())
		{
			InitHandle.Reset();
			return false;
		}

		World = GetWorld();
		if (!IsValid(World))
		{
			return true;
		}
		if (World->bIsTearingDown)
		{
			InitHandle.Reset();
			return false;
		}
	}

	if (World->IsGameWorld())
	{
		if (!World->HasBegunPlay())
		{
			return true;
		}

		bInitialized = true;
		OnInitialize();

		if (!IsSafeObject())
		{
			InitHandle.Reset();
			return false;
		}

		World = GetWorld();
		if (!IsValid(World))
		{
			return true;
		}
		if (World->bIsTearingDown)
		{
			InitHandle.Reset();
			return false;
		}
	}

	SetTickableTickType(ETickableTickType::Conditional);
	InitHandle.Reset();
	return false;
}

bool UToroObject::IsSafeObject() const
{
	return IsValid(this) && !IsTemplate() && !HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed);
}

UToroInstancedObject::UToroInstancedObject()
{
}
