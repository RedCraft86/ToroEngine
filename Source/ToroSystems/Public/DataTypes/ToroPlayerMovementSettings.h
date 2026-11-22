// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Engine/EngineTypes.h"
#include "ToroPlayerMovementSettings.generated.h"

class USoundBase;

/** Sprint stamina tuning. Rates are fractions of a full bar per second. */
USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FToroPlayerStaminaSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Stamina)
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, Category = Stamina, meta = (ClampMin = "0.0"))
	float DrainRate = 0.15f;

	UPROPERTY(EditAnywhere, Category = Stamina, meta = (ClampMin = "0.0"))
	float RecoveryRate = 0.1f;

	/** Fraction that must recover after exhaustion before sprinting is available again. */
	UPROPERTY(EditAnywhere, Category = Stamina, meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float SprintRecoveryThreshold = 0.5f;
};

/** Ground footstep cadence and sound selection. */
USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FToroPlayerFootstepSettings
{
	GENERATED_BODY()

	/** Sound used for surfaces without an override. */
	UPROPERTY(EditAnywhere, Category = Footsteps)
	TObjectPtr<USoundBase> DefaultSound;

	UPROPERTY(EditAnywhere, Category = Footsteps)
	TMap<TEnumAsByte<EPhysicalSurface>, TObjectPtr<USoundBase>> SurfaceSounds;

	UPROPERTY(EditAnywhere, Category = Footsteps)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, Category = Footsteps, meta = (ClampMin = "0.01", Units = "s"))
	float WalkInterval = 0.5f;

	UPROPERTY(EditAnywhere, Category = Footsteps, meta = (ClampMin = "0.01", Units = "s"))
	float RunInterval = 0.35f;

	UPROPERTY(EditAnywhere, Category = Footsteps, meta = (ClampMin = "0.01", Units = "s"))
	float CrouchInterval = 0.6f;

	UPROPERTY(EditAnywhere, Category = Footsteps, meta = (ClampMin = "0.0"))
	float Volume = 1.0f;

	UPROPERTY(EditAnywhere, Category = Footsteps, meta = (ClampMin = "0.0"))
	float RunVolumeMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, Category = Footsteps, meta = (ClampMin = "0.0"))
	float CrouchVolumeMultiplier = 0.25f;
};
