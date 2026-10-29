// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroWorldLibrary.h"
#include "Engine/LevelScriptActor.h"
#include "Engine/LevelStreaming.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/ToroUtilities.h"
#include "Misc/PackageName.h"
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

EToroLevelStreamState UToroWorldLibrary::GetLevelStreamState(const UObject* ContextObject, TSoftObjectPtr<UWorld> Level)
{
	const FName LevelName = FName(*FPackageName::ObjectPathToPackageName(Level.ToString()));
	const ULevelStreaming* StreamedLevel = UGameplayStatics::GetStreamingLevel(ContextObject, LevelName);
	if (!IsValid(StreamedLevel) || !StreamedLevel->ShouldBeLoaded())
	{
		return EToroLevelStreamState::Unloaded;
	}

	return StreamedLevel->ShouldBeVisible() ? EToroLevelStreamState::Visible : EToroLevelStreamState::Loaded;
}

FVoidCoroutine UToroWorldLibrary::SetLevelStreamState(FLatentActionInfo LatentInfo, const UObject* ContextObject,
	ULevelStreaming*& StreamedLevel, TSoftObjectPtr<UWorld> Level, EToroLevelStreamState State)
{
	const FName LevelName = FName(*FPackageName::ObjectPathToPackageName(Level.ToString()));
	StreamedLevel = UGameplayStatics::GetStreamingLevel(ContextObject, LevelName);
	if (!IsValid(StreamedLevel))
	{
		co_return;
	}

	if (State == EToroLevelStreamState::Unloaded)
	{
		co_await UE5Coro::Latent::ChainEx(&UGameplayStatics::UnloadStreamLevel,
			ContextObject, LevelName, std::placeholders::_2, false);
	}
	else // State == Loaded or Visible
	{
		const TWeakObjectPtr<ULevelStreaming> WeakLevel(StreamedLevel);
		if (State == EToroLevelStreamState::Loaded)
		{
			StreamedLevel->SetShouldBeVisible(false);
		}

		co_await UE5Coro::Latent::ChainEx(&UGameplayStatics::LoadStreamLevel, ContextObject,
			LevelName, State == EToroLevelStreamState::Visible, false, std::placeholders::_2);
	}
}
