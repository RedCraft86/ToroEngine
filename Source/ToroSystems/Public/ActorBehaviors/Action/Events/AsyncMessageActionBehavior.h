// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "AsyncMessageId.h"
#include "StructUtils/InstancedStruct.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "AsyncMessageActionBehavior.generated.h"

/**
 * Queues a message and copied struct payload through the owning world's async gameplay message system.
 * Delivery follows listener scheduling; executing the action does not wait for listeners.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class UAsyncMessageActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

	/** Message identifier to broadcast. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ShowOnlyInnerProperties = true))
	FAsyncMessageId MessageId;

	/** Optional struct payload copied into the message queue. */
	UPROPERTY(EditAnywhere, Category = Settings)
	FInstancedStruct Payload;

	virtual void OnTriggered() override;
};
