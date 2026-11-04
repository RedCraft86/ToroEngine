// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "RemoteEventActionBehavior.generated.h"

/**
 * Dispatches a named remote event to the world's Level Blueprints through Toro's world helper.
 * Uses the action component as world context.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class URemoteEventActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

	/** Level Blueprint remote event name. */
	UPROPERTY(EditAnywhere, Category = Settings)
	FName EventName = NAME_None;

	virtual void OnTriggered() override;
};
