// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "ToroCoreEd.h"
#include "MiscEditor/ToroSlateStyle.h"
#include "MiscEditor/ToroToolbarButton.h"
#include "ComponentVis/ToroComponentVisualizer.h"
#include "DetailsPanel/ToroStructCustomization.h"
#include "DetailsPanel/ToroClassCustomization.h"

DEFINE_LOG_CATEGORY(LogToroCoreEd);

#define LOCTEXT_NAMESPACE "FToroCoreEdModule"

void FToroCoreEdModule::StartupModule()
{
	
}

void FToroCoreEdModule::ShutdownModule()
{
	FToroToolbarButton::UnregisterAll();
	FToroClassCustomization::UnregisterAll();
	FToroStructCustomization::UnregisterAll();
	FToroComponentVisualizer::UnregisterAll();
	FToroSlateStyle::UnregisterAll();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FToroCoreEdModule, ToroCoreEd)