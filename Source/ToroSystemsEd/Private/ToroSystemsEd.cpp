// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ToroSystemsEd.h"
#include "Modules/ModuleManager.h"

#include "ToroSystemsStyle.h"
#include "Visualizers/LightProbeVisualizer.h"
#include "DetailsPanels/ActorBehaviorRefCustomization.h"

void FToroSystemsEdModule::StartupModule()
{
	FToroSlateStyle::Register<FToroSystemsStyle>();

	FToroComponentVisualizer::Register<FLightProbeVisualizer, ULightProbeComponent>();

	FToroStructCustomization::Register<FActorBehaviorRefCustomization, FActorBehaviorRef>();
}

void FToroSystemsEdModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FToroSystemsEdModule, ToroSystemsEd)
