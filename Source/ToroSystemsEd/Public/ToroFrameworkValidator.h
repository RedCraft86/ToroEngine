// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "ToroFrameworkValidator.generated.h"

/**
 * Checks actual framework instances once after each PIE world's startup.
 * Native and Blueprint subclasses of the corresponding Toro bases are accepted.
 * Mismatches are reported to the PIE Message Log and request an orderly PIE shutdown.
 * Absent roles and spectator pawns are skipped; later spawns are not monitored.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class UToroFrameworkValidator final : public UWorldSubsystem
{
	GENERATED_BODY()

	bool DoValidation() const;

	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};
