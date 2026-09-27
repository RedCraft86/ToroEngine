// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "ToroCore.h"
#include "Utilities/WorldGetter.h"

DEFINE_LOG_CATEGORY(LogToroCore);

#define LOCTEXT_NAMESPACE "FToroCoreModule"

void FToroCoreModule::StartupModule()
{
    PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddStatic(PreLoadMap);
    PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddStatic(PostLoadMap);
}

void FToroCoreModule::ShutdownModule()
{
	FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
}

void FToroCoreModule::PreLoadMap(const FString&)
{
	FWorldGetter::Reset();
}

void FToroCoreModule::PostLoadMap(UWorld* World)
{
	if (IsValid(World))
	{
		FWorldGetter::SetWorld(World);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FToroCoreModule, ToroCore)