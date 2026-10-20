// Copyright (C) RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ToroEngine.h"
#include "Engine/World.h"
#include "Utilities/WorldGetter.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogToroEngine)

void FToroEngineModule::StartupModule()
{
	WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddStatic(&OnWorldCleanup);
}

void FToroEngineModule::ShutdownModule()
{
	if (WorldCleanupHandle.IsValid())
	{
		FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
		WorldCleanupHandle.Reset();
	}

	FWorldGetter::Reset();
}

void FToroEngineModule::OnWorldCleanup(UWorld*, bool, bool)
{
	FWorldGetter::Reset();
}

IMPLEMENT_MODULE(FToroEngineModule, ToroEngine)
