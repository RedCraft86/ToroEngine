// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ToroEngineEd.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#include "SlateStyles/ToroEngineStyle.h"
#include "SlateStyles/ToroSlateStyle.h"

DEFINE_LOG_CATEGORY(LogToroEngineEd)

void FToroEngineEdModule::StartupModule()
{
	FToroSlateStyle::Register<FToroEngineStyle>();
}

void FToroEngineEdModule::ShutdownModule()
{
	FToroSlateStyle::UnregisterAll();
}

FString FToroEngineEdModule::GetPluginDir()
{
	static FString PluginDir = IPluginManager::Get().FindPlugin(UE_PLUGIN_NAME)->GetBaseDir();
	return PluginDir;
}

IMPLEMENT_MODULE(FToroEngineEdModule, ToroEngineEd)
