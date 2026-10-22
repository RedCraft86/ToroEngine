// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ToroEngine.h"
#include "Engine/World.h"
#include "Utilities/WorldGetter.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogToroEngine)

void FToroEngineModule::StartupModule()
{
	PostWorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddStatic(&OnPostWorldInit);
	WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddStatic(&OnWorldCleanup);
}

void FToroEngineModule::ShutdownModule()
{
	if (PostWorldInitHandle.IsValid())
	{
		FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitHandle);
		PostWorldInitHandle.Reset();
	}

	if (WorldCleanupHandle.IsValid())
	{
		FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
		WorldCleanupHandle.Reset();
	}

	FWorldGetter::Reset();
}

void FToroEngineModule::OnPostWorldInit(UWorld* World, FWorldInitializationValues)
{
	if (IsValid(World) && World->IsGameWorld())
	{
		FWorldGetter::SetWorld(World);
	}
}

void FToroEngineModule::OnWorldCleanup(UWorld*, bool, bool)
{
	FWorldGetter::Reset();
}

IMPLEMENT_MODULE(FToroEngineModule, ToroEngine)
