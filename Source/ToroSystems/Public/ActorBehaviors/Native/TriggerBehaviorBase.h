// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ActorBehaviors/ActorBehaviorBase.h"
#include "TriggerBehaviorBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTriggerBehaviorDelegate)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTriggerBehaviorTickDelegate, float, ElapsedTime, float, DeltaTime)

DECLARE_MULTICAST_DELEGATE(FTriggerBehaviorDelegateNative)
DECLARE_MULTICAST_DELEGATE_TwoParams(FTriggerBehaviorTickDelegateNative, float /* ElapsedTime */, float /* DeltaTime */)

/**
 * Base behavior for a trigger with begin, end, and elapsed-time tick notifications.
 * Native listeners run before Blueprint listeners. Broadcasts are synchronous, so ending
 * during a native begin callback does not cancel the subsequent Blueprint begin notification.
 * Editor validation permits only one trigger behavior, including subclasses, per actor.
 */
UCLASS(Abstract)
class TOROSYSTEMS_API UTriggerBehaviorBase : public UActorBehaviorBase
{
	GENERATED_BODY()

public:

	UTriggerBehaviorBase();

	/** Broadcasts after native begin listeners, with state and ticking already committed. */
	UPROPERTY(BlueprintAssignable, Category = ActorBehaviors)
	FTriggerBehaviorDelegate OnBeginTrigger;

	/** Broadcasts after native end listeners; state, ticking, and elapsed time are reset before notifications. */
	UPROPERTY(BlueprintAssignable, Category = ActorBehaviors)
	FTriggerBehaviorDelegate OnEndTrigger;

	/**
	 * Broadcasts after native tick listeners on a triggered tick.
	 * ElapsedTime is accumulated tick time in seconds; DeltaTime is the current tick's duration in seconds.
	 */
	UPROPERTY(BlueprintAssignable, Category = ActorBehaviors)
	FTriggerBehaviorTickDelegate OnTriggerTick;

	DECLARE_MULTICAST_DELEGATE(FTriggerBehaviorDelegateNative)

	/** Native begin notification, emitted before OnBeginTrigger. */
	FTriggerBehaviorDelegateNative OnBeginTriggerNative;

	/** Native end notification, emitted before OnEndTrigger. */
	FTriggerBehaviorDelegateNative OnEndTriggerNative;

	/** Native tick notification with accumulated elapsed seconds and tick delta seconds, emitted before OnTriggerTick. */
	FTriggerBehaviorTickDelegateNative OnTriggerTickNative;

protected:

	/**
	 * Starts a stopped trigger, resetting elapsed time and enabling ticking before begin notifications.
	 * Does nothing if already triggered. Native begin listeners run before Blueprint listeners.
	 */
	UFUNCTION(BlueprintCallable, Category = ActorBehaviors)
	void StartTrigger();

	/**
	 * Ends a running trigger, disabling ticking and resetting elapsed time before end notifications.
	 * Does nothing if already stopped. Native end listeners run before Blueprint listeners.
	 */
	UFUNCTION(BlueprintCallable, Category = ActorBehaviors)
	void EndTrigger();

private:

	bool bTriggered;
	float ElapsedTime;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunc) override;
#if WITH_EDITOR
	virtual TSubclassOf<UActorBehaviorBase> DuplicateCheckClass() const override final
	{
		return UTriggerBehaviorBase::StaticClass();
	}
#endif
};
