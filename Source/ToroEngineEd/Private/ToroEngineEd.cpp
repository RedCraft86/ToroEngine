// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ToroEngineEd.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#include "SlateStyles/ToroSlateStyle.h"
#include "SlateStyles/ToroEngineStyle.h"

#include "ToolbarButtons/ToroToolbarButton.h"
#include "ToolbarButtons/Asset/AssetRefreshButton.h"
#include "ToolbarButtons/Asset/AssetValidateButton.h"
#include "ToolbarButtons/Level/ActorInstanceButton.h"
#include "ToolbarButtons/Level/ActorLayoutButton.h"
#include "ToolbarButtons/Level/ActorMergeButton.h"
#include "ToolbarButtons/Level/ActorSplitButton.h"
#include "ToolbarButtons/Level/RestartEditorButton.h"
#include "ToolbarButtons/Level/MapValidateButton.h"

DEFINE_LOG_CATEGORY(LogToroEngineEd)

void FToroEngineEdModule::StartupModule()
{
	FToroSlateStyle::Register<FToroEngineStyle>();

	FToroToolbarButton::Register<FAssetRefreshButton>();
	FToroToolbarButton::Register<FAssetValidateButton>();
	FToroToolbarButton::Register<FActorLayoutButton>();
	FToroToolbarButton::Register<FActorMergeButton>();
	FToroToolbarButton::Register<FActorSplitButton>();
	FToroToolbarButton::Register<FActorInstanceButton>();
	FToroToolbarButton::Register<FRestartEditorButton>();
	FToroToolbarButton::Register<FMapValidateButton>();
}

void FToroEngineEdModule::ShutdownModule()
{
	FToroToolbarButton::UnregisterAll();
	FToroSlateStyle::UnregisterAll();
}

FString FToroEngineEdModule::GetPluginDir()
{
	static FString PluginDir = IPluginManager::Get().FindPlugin(UE_PLUGIN_NAME)->GetBaseDir();
	return PluginDir;
}

IMPLEMENT_MODULE(FToroEngineEdModule, ToroEngineEd)
