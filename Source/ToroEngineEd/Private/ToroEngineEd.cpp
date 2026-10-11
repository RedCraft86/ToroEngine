// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "ToroEngineEd.h"
#include "ToroEngineStyle.h"
#include "ToolbarButtons/ActorLayoutButton.h"
#include "ToolbarButtons/ActorMergeButton.h"
#include "ToolbarButtons/ActorSplitButton.h"
#include "ToolbarButtons/ActorInstanceButton.h"
#include "ToolbarButtons/RestartEditorButton.h"

DEFINE_LOG_CATEGORY(LogToroEngineEd);

#define LOCTEXT_NAMESPACE "FToroEngineEd"

void FToroEngineEdModule::StartupModule()
{
	FToroSlateStyle::Register<FToroEngineStyle>();
	FToroToolbarButton::Register<FActorLayoutButton>();
	FToroToolbarButton::Register<FActorMergeButton>();
	FToroToolbarButton::Register<FActorSplitButton>();
	FToroToolbarButton::Register<FActorInstanceButton>();
	FToroToolbarButton::Register<FRestartEditorButton>();
}

void FToroEngineEdModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FToroEngineEdModule, ToroEngineEd)