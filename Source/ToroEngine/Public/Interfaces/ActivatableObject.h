// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UObject/Interface.h"
#include "ActivatableObject.generated.h"

/** Reflected interface type identifying objects that expose an activation state. */
UINTERFACE(MinimalAPI)
class UActivatableObject : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for querying and changing activation state through native or Blueprint implementations.
 * Implementations own their state; this interface stores no activation state.
 * The static helpers use reflected dispatch to support Blueprint-only implementations.
 */
class TOROENGINE_API IActivatableObject
{
	GENERATED_BODY()

protected:

	/** Implementation hook returning this object's activation state. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Activation)
	bool GetActiveState() const;

	/**
	 * Implementation hook for changing activation state.
	 * The implementation owns state storage and any resulting transitions.
	 * @param bNewState Requested activation state.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Activation)
	void SetActiveState(bool bNewState);

public:

	/**
	 * Queries activation state through reflected interface dispatch.
	 * @param Target Object to query; may be null.
	 * @return The implementation's activation state for a valid implementing object;
	 *         otherwise true, including for null, invalid, or non-implementing objects.
	 */
	static bool GetActiveState(const UObject* Target);

	/**
	 * Requests an activation state change through reflected interface dispatch.
	 * Does nothing for null, invalid, or non-implementing objects.
	 * @param Target Object whose activation state should change; may be null.
	 * @param bNewState Requested activation state.
	 */
	static void SetActiveState(UObject* Target, bool bNewState);
};
