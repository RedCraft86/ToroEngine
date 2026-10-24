// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UObject/Interface.h"
#include "ExitableObject.generated.h"

/** Reflected interface type identifying objects that support exit requests and notifications. */
UINTERFACE(MinimalAPI)
class UExitableObject : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for receiving exit requests and notifications through native or Blueprint implementations.
 * The static helpers validate the target and use reflected dispatch to support Blueprint-only implementations.
 * Requesting an exit does not automatically send an exit notification.
 */
class TOROENGINE_API IExitableObject
{
	GENERATED_BODY()

protected:

	/**
	 * Implementation hook for handling an exit request.
	 * The default native implementation returns false.
	 * @param Requester Object requesting the exit; callers may pass null.
	 * @return Whether the implementation accepts the exit request.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Exit)
	bool RequestExit(const UObject* Requester);
	virtual bool RequestExit_Implementation(const UObject* Requester)
	{
		return false;
	}

	/**
	 * Implementation hook for receiving notification that an object has exited.
	 * The default native implementation does nothing.
	 * @param ExitedObject Object reported as having exited; callers may pass null.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Exit)
	void NotifyObjectExited(const UObject* ExitedObject);
	virtual void NotifyObjectExited_Implementation(const UObject* ExitedObject)
	{
	}

public:

	/**
	 * Sends an exit request through reflected interface dispatch.
	 * @param Target Object receiving the request; may be null.
	 * @param Requester Object requesting the exit; forwarded unchanged without validation.
	 * @return The implementation's result, or false for null, invalid, or non-implementing targets.
	 */
	static bool RequestExit(UObject* Target, const UObject* Requester);

	/**
	 * Sends an exit notification through reflected interface dispatch.
	 * Does nothing for null, invalid, or non-implementing targets.
	 * @param Target Object receiving the notification; may be null.
	 * @param ExitedObject Object reported as having exited; forwarded unchanged without validation.
	 */
	static void NotifyObjectExited(UObject* Target, const UObject* ExitedObject);
};
