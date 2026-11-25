// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroWorldLibrary.h"
#include "Engine/LevelScriptActor.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/ToroUtilities.h"
#include "WorldPartition/DataLayer/DataLayerAsset.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "Engine/Level.h"
#include "Engine/World.h"

UWorld* UToroWorldLibrary::GetWorldFromObject(const UObject* Target, bool bTryFindPossible)
{
	return ToroEngine::GetWorld(Target, bTryFindPossible);
}

void UToroWorldLibrary::ReloadLevel(const UObject* ContextObject, bool bAbsolute, const FString& Options)
{
	if (IsValid(ContextObject))
	{
		UGameplayStatics::OpenLevel(
			ContextObject,
			*UGameplayStatics::GetCurrentLevelName(ContextObject),
			bAbsolute,
			Options
		);
	}
}

void UToroWorldLibrary::CallRemoteEvent(const UObject* ContextObject, FName EventName)
{
	if (!EventName.IsNone())
	{
		const UWorld* World = ToroEngine::GetWorld(ContextObject);
		if (ALevelScriptActor* LSA = IsValid(World) ? World->GetLevelScriptActor() : nullptr)
		{
			LSA->RemoteEvent(EventName);
		}
	}
}

bool UToroWorldLibrary::IsPersistentLevel(const ULevel* Target)
{
	return IsValid(Target)
		&& IsValid(Target->OwningWorld)
		&& Target->OwningWorld->PersistentLevel == Target;
}

EDataLayerRuntimeState UToroWorldLibrary::GetDataLayerState(const UObject* ContextObject,
	const UDataLayerAsset* DataLayer, bool bEffective)
{
	const UWorld* World = ToroEngine::GetWorld(ContextObject);
	const UDataLayerManager* Manager = World ? UDataLayerManager::GetDataLayerManager(World) : nullptr;
	const UDataLayerInstance* Instance = Manager && IsValid(DataLayer)
		? Manager->GetDataLayerInstanceFromAsset(DataLayer) : nullptr;
	if (!Instance || !Instance->IsRuntime())
	{
		return EDataLayerRuntimeState::Unloaded;
	}

	return bEffective ? Instance->GetEffectiveRuntimeState() : Instance->GetRuntimeState();
}

bool UToroWorldLibrary::SetDataLayerState(const UObject* ContextObject,
	const UDataLayerAsset* DataLayer, EDataLayerRuntimeState State)
{
	const UWorld* World = ToroEngine::GetWorld(ContextObject);
	UDataLayerManager* Manager = World ? UDataLayerManager::GetDataLayerManager(World) : nullptr;
	return Manager && IsValid(DataLayer) && Manager->SetDataLayerRuntimeState(DataLayer, State);
}
