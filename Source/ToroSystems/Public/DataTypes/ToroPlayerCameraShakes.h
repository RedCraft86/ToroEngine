// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Camera/CameraShakeBase.h"
#include "ToroPlayerCameraShakes.generated.h"

/** Continuous first-person camera shake for walking. */
UCLASS(MinimalAPI)
class UToroWalkCameraShake : public UCameraShakeBase
{
	GENERATED_BODY()

public:

	UToroWalkCameraShake(const FObjectInitializer& ObjectInitializer);
};

/** Continuous first-person camera shake for sprinting. */
UCLASS(MinimalAPI)
class UToroRunCameraShake : public UCameraShakeBase
{
	GENERATED_BODY()

public:

	UToroRunCameraShake(const FObjectInitializer& ObjectInitializer);
};
