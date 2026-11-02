// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ActorBehaviors/ActorBehaviorBase.h"
#include "ActorBehaviors/ActorBehaviorRef.h"
#include "ActionBehaviorBase.generated.h"

/**
 * Dispatches an action when a required sibling trigger broadcasts its native begin event.
 * Actions repeat by default. One-shot actions are consumed and unsubscribed before callbacks,
 * preventing recursive execution. The native hook runs before the Blueprint event.
 */
UCLASS(Abstract)
class TOROSYSTEMS_API UActionBehaviorBase : public UActorBehaviorBase
{
	GENERATED_BODY()

public:

	UActionBehaviorBase();

	/** Blueprint action hook, dispatched after OnTriggered(). */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Triggered")
	void ReceiveOnTriggered();

protected:

	/** Required sibling trigger reference, resolved and subscribed to at BeginPlay. */
	UPROPERTY(EditAnywhere, Category = Dependencies, DisplayName = "Trigger")
	FActorBehaviorRef TriggerRef;

	/** Whether to consume and unsubscribe the action on its first execution; false by default. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (DisplayPriority = -10))
	bool bOnlyOnce;

	/** Native action hook, dispatched before ReceiveOnTriggered(). */
	virtual void OnTriggered() {}

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	bool bTriggered;
	void DoTrigger();
	void UnbindTrigger();
};
