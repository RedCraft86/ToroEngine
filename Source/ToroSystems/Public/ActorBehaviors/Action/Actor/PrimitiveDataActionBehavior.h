// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "PrimitiveDataActionBehavior.generated.h"

class AStaticMeshActor;

/**
 * Updates custom primitive data on existing static-mesh actors; unavailable targets are skipped.
 * Selects default data or runtime overrides without creating dynamic material instances.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class UPrimitiveDataActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

	/** Existing static-mesh actors to update. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSet<TSoftObjectPtr<AStaticMeshActor>> Targets;

	/** Float values keyed by custom primitive data index. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TMap<uint8, float> PrimitiveData;

	/** Update default data when true; write runtime overrides otherwise. */
	UPROPERTY(EditAnywhere, Category = Settings)
	bool bOverrideDefault = true;

	virtual void OnTriggered() override;
};
