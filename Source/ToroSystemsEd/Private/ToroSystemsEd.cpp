// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ToroSystemsEd.h"
#include "Modules/ModuleManager.h"
#include "DetailsPanels/ActorBehaviorRefCustomization.h"

void FToroSystemsEdModule::StartupModule()
{
	FToroStructCustomization::Register<FActorBehaviorRefCustomization, FActorBehaviorRef>();
}

void FToroSystemsEdModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FToroSystemsEdModule, ToroSystemsEd)
