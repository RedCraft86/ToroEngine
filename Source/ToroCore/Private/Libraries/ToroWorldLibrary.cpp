// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Libraries/ToroWorldLibrary.h"
#include "Engine/LevelScriptActor.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/WorldGetter.h"
#include "Engine/LevelStreaming.h"
#include "Misc/PackageName.h"
#include "Engine/World.h"

UWorld* UToroWorldLibrary::GetPossibleWorld(const UObject* Context)
{
	return FWorldGetter::Get(Context);
}

void UToroWorldLibrary::ReloadLevel(const UObject* ContextObject, bool bAbsolute, const FString& Options)
{
	const UWorld* World = FWorldGetter::Get(ContextObject);
	if (IsValid(World))
	{
		UGameplayStatics::OpenLevel(World, *UGameplayStatics::GetCurrentLevelName(World), bAbsolute, Options);
	}
}

void UToroWorldLibrary::CallRemoteEvent(const UObject* ContextObject, FName EventName)
{
	if (!EventName.IsNone())
	{
		const UWorld* World = FWorldGetter::Get(ContextObject);
		if (ALevelScriptActor* LSA = IsValid(World) ? World->GetLevelScriptActor() : nullptr)
		{
			LSA->RemoteEvent(EventName);
		}
	}
}

EToroLevelStreamState UToroWorldLibrary::GetLevelStreamState(const UObject* ContextObject, const TSoftObjectPtr<UWorld>& Level)
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
	else // State == LoadOnly or LoadAndShow
	{
		co_await UE5Coro::Latent::ChainEx(&UGameplayStatics::LoadStreamLevel, ContextObject,
			LevelName, State == EToroLevelStreamState::Visible, false, std::placeholders::_2);
	}

	co_return;
}
