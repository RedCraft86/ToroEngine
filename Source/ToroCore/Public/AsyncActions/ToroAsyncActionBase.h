// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "ToroAsyncActionBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FToroAsyncActionDelegate);
DECLARE_MULTICAST_DELEGATE(FToroAsyncActionDelegateNative);

/**
 * Abstract base for async actions that resolve a world from a weak context reference.
 * The context is not kept alive by this action. Derived actions manage activation,
 * game-instance registration when needed, and completion through SetReadyToDestroy().
 * World lookup requires the game thread and may use a fallback world when the context
 * is unavailable or cannot resolve a world.
 */
UCLASS(Abstract, NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroAsyncActionBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	/** Transient world-context reference that does not keep the referenced object alive. */
	UPROPERTY(Transient)
	TWeakObjectPtr<const UObject> WorldContext;

protected:

	/** Returns the stored context, or nullptr when unset, expired, or otherwise invalid. */
	FORCEINLINE const UObject* GetWorldContext() const
	{
		return WorldContext.IsValid() ? WorldContext.Get() : nullptr;
	}

	/**
	 * Replaces the stored weak context only when the supplied object is valid.
	 * @param InContext World-context object. Null or invalid inputs leave the previous context unchanged.
	 */
	void SetWorldContext(const UObject* InContext);

	virtual UWorld* GetWorld() const override;
};
