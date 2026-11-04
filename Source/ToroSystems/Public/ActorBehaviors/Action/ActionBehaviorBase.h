// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Engine/TimerHandle.h"
#include "ActorBehaviors/ActorBehaviorBase.h"
#include "ActorBehaviors/ActorBehaviorRef.h"
#include "ActionBehaviorBase.generated.h"

/** How an action changes a target's boolean state. */
UENUM(BlueprintType)
enum class EToroActionApplyType : uint8
{
	Set,
	Toggle
};

/**
 * Dispatches an action when a required sibling trigger broadcasts its native begin event.
 * Actions repeat by default. One-shot actions are consumed and unsubscribed before scheduling or execution.
 * Delays below 0.1 seconds execute immediately; pending repeatable actions can optionally restart their delay.
 * The native hook runs before the Blueprint event. EndPlay cancels pending execution.
 * Persistent one-shot actions save their consumed state, but pending timers are not saved.
 */
UCLASS(Abstract, PrioritizeCategories = (Dependencies, Settings, Control))
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

	/** Whether to consume and unsubscribe on the first accepted trigger, before any delay; false by default. */
	UPROPERTY(EditAnywhere, Category = Control)
	bool bOneShot;

	/** Whether this component is eligible for saving its one-shot trigger status; false by default. */
	UPROPERTY(EditAnywhere, Category = Control, meta = (EditCondition = "bOneShot"))
	bool bPersistent;

	/** Delay in game-time seconds; values below 0.1 execute immediately. Defaults to zero. */
	UPROPERTY(EditAnywhere, Category = Control, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float Delay;

	/** Whether repeatable triggers restart a pending delay; otherwise they are ignored. False by default. */
	UPROPERTY(EditAnywhere, Category = Control)
	bool bRetriggerable;

	/** Native action hook, dispatched before ReceiveOnTriggered(). */
	virtual void OnTriggered() {}

	/** Whether a new trigger may be accepted before one-shot consumption or delay scheduling. */
	virtual bool CanTrigger() const { return true; }

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool EligibleForSaving_Implementation() const override
	{
		return bOneShot && bPersistent;
	}

private:

	UPROPERTY(SaveGame)
	bool bTriggered;

	FTimerHandle DelayTimer;

	void DoTrigger();
	void ExecuteAction();
	void UnbindTrigger();

#if WITH_EDITOR
	virtual TSubclassOf<UActorBehaviorBase> DuplicateCheckClass() const override
	{
		return nullptr;
	}

	virtual bool CanEditChange(const FProperty* InProperty) const override
	{
		const bool bSuper = Super::CanEditChange(InProperty);
		if (InProperty && InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UActionBehaviorBase, bRetriggerable))
		{
			return bSuper && !bOneShot && Delay > 0.1f;
		}

		return bSuper;
	}
#endif
};
