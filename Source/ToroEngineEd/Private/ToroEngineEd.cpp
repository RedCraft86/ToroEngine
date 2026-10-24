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

#include "Visualizers/ToroComponentVisualizer.h"

#include "DetailsPanels/ToroStructCustomization.h"
#include "DetailsPanels/Struct/ToroWrapperCustomization.h"

#include "DetailsPanels/ToroClassCustomization.h"
#include "DetailsPanels/Class/ToroDatabaseCustomization.h"

#include "DataTypes/InlineCurves.h"
#include "DataTypes/WrappedTypes.h"

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

	FToroStructCustomization::Register<FToroWrapperCustomization, FWrappedBool>();
	FToroStructCustomization::Register<FToroWrapperCustomization, FWrappedFloat>();
	FToroStructCustomization::Register<FToroWrapperCustomization, FWrappedByte>();
	FToroStructCustomization::Register<FToroWrapperCustomization, FWrappedInt32>();
	FToroStructCustomization::Register<FToroWrapperCustomization, FWrappedInt64>();
	FToroStructCustomization::Register<FToroWrapperCustomization, FWrappedName>();
	FToroStructCustomization::Register<FToroWrapperCustomization, FWrappedString>();
	FToroStructCustomization::Register<FToroWrapperCustomization, FInlineFloatCurve>();
	FToroStructCustomization::Register<FToroWrapperCustomization, FInlineVectorCurve>();
	FToroStructCustomization::Register<FToroWrapperCustomization, FInlineColorCurve>();

	FToroClassCustomization::Register<FToroDatabaseCustomization, UToroDatabase>();
}

void FToroEngineEdModule::ShutdownModule()
{
	FToroClassCustomization::UnregisterAll();
	FToroStructCustomization::UnregisterAll();
	FToroComponentVisualizer::UnregisterAll();
	FToroToolbarButton::UnregisterAll();
	FToroSlateStyle::UnregisterAll();
}

FString FToroEngineEdModule::GetPluginDir()
{
	static FString PluginDir = IPluginManager::Get().FindPlugin(UE_PLUGIN_NAME)->GetBaseDir();
	return PluginDir;
}

IMPLEMENT_MODULE(FToroEngineEdModule, ToroEngineEd)
