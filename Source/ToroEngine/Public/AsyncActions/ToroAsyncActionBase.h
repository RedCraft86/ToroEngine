// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Delegates/Delegate.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "ToroAsyncActionBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FToroAsyncActionDelegate);
DECLARE_MULTICAST_DELEGATE(FToroAsyncActionDelegateNative);

/**
 * Native async-action base providing weak world-context storage and configurable fallback world lookup.
 * World resolution is intended for the game thread.
 * Subclasses manage activation, completion, and cancellation where applicable, and register with a game
 * instance when needed to retain the action until SetReadyToDestroy() is called.
 */
UCLASS(Abstract, NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroAsyncActionBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UToroAsyncActionBase();

	virtual UWorld* GetWorld() const override;

protected:

	/**
	 * Allows world resolution to try the current play world and then GWorld when the context has no valid world.
	 * Does not filter world types; fallback selection is best-effort and may resolve an unrelated world.
	 */
	bool bAllowAnyWorld;

	/** Returns the stored context, or null when none is set or the weak reference has expired. */
	FORCEINLINE const UObject* GetWorldContext() const
	{
		return WorldContext.Get();
	}

	/**
	 * Stores a valid world context without extending its lifetime.
	 * Null or invalid inputs are ignored, preserving the previous context.
	 * @param InContext Object used to resolve the action's world.
	 */
	void SetWorldContext(const UObject* InContext);

private:

	/** Transient weak context reference; does not keep the context alive and may expire independently of the action. */
	UPROPERTY(Transient)
	TWeakObjectPtr<const UObject> WorldContext;
};
