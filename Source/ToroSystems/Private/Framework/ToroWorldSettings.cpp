// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroWorldSettings.h"
#include "PostProcess/WorldPostProcessComponent.h"

AToroWorldSettings::AToroWorldSettings()
{
	PostProcess = CreateDefaultSubobject<UWorldPostProcessComponent>(TEXT("PostProcess"));
}
