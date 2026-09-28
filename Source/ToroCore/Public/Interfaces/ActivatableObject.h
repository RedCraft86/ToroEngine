// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "UObject/Interface.h"
#include "ActivatableObject.generated.h"

/** Reflected Unreal interface corresponding to IActivatableObject. */
UINTERFACE()
class UActivatableObject : public UInterface
{
	GENERATED_BODY()
};

/**
 * Provides Blueprint and native events for querying and setting an object's active state.
 * Implementing classes must provide both events; neither event has a native default.
 */
class TOROCORE_API IActivatableObject
{
	GENERATED_BODY()

protected:

	/** Returns the implementing object's active state. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Activation)
	bool GetActiveState() const;

	/** Sets the implementing object's active state to the requested value. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Activation)
	void SetActiveState(const bool bNewState);

public:

	/**
	 * Queries the active state through the interface dispatcher.
	 * @param Target Object to query.
	 * @return Event result, or false if Target is invalid or does not implement this interface.
	 * Therefore, false does not distinguish an inactive object from an unsupported target.
	 */
	static bool GetActiveState(const UObject* Target);

	/**
	 * Dispatches SetActiveState to a valid object implementing this interface.
	 * Invalid or unsupported targets are ignored.
	 * @param Target Object whose state will be set.
	 * @param bNewState New active state to request.
	 */
	static void SetActiveState(UObject* Target, const bool bNewState);
};
