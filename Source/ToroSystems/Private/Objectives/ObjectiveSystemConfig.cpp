// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Objectives/ObjectiveSystemConfig.h"
#include "ToroDevSettings.h"

namespace ObjectiveTags
{
	UE_DEFINE_GAMEPLAY_TAG(Base, "Objective");
}

FObjectiveSystemConfig::FObjectiveSystemConfig()
{
	AllowedOnMap = LevelTypeTags::Gameplay;
}

const FObjectiveSystemConfig* FObjectiveSystemConfig::Get()
{
	const UToroDevSettings* Settings = GetDefault<UToroDevSettings>();
	return IsValid(Settings) ? &Settings->Objectives : nullptr;
}
