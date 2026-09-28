// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "UObject/Interface.h"
#include "ExitableObject.generated.h"

/** Reflected Unreal interface corresponding to IExitableObject. */
UINTERFACE()
class UExitableObject : public UInterface
{
	GENERATED_BODY()
};

/**
 * Allows objects to receive requests to exit and explicit notifications after another object exits.
 * Accepting a request does not guarantee the exit has completed; notifications are dispatched
 * separately by the caller. Events can be implemented in native code or Blueprint.
 */
class TOROCORE_API IExitableObject
{
	GENERATED_BODY()

protected:

	/**
	 * Requests that this object exit what it is doing. The default implementation rejects the request.
	 * @param Requester Object initiating the request; may be null.
	 * @return True if accepted or handled, even if the exit completes later.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Exit)
	bool RequestExit(const UObject* Requester);
	virtual bool RequestExit_Implementation(const UObject* Requester) { return false; }

	/**
	 * Notifies this receiver that another object has exited; the default implementation does nothing.
	 * @param ExitedObject Object that exited; may be null and is forwarded unchanged.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Exit)
	void NotifyObjectExited(const UObject* ExitedObject);
	virtual void NotifyObjectExited_Implementation(const UObject* ExitedObject) {}

public:

	/**
	 * Dispatches RequestExit to a valid object implementing this interface.
	 * @param Target Object being asked to exit.
	 * @param Requester Initiating object; may be null.
	 * @return Event result, or false for an invalid or unsupported target.
	 */
	static bool RequestExit(UObject* Target, const UObject* Requester);

	/**
	 * Dispatches NotifyObjectExited to a valid receiver implementing this interface.
	 * Invalid or unsupported receivers are ignored; this does not initiate an exit.
	 * @param Receiver Object to notify.
	 * @param ExitedObject Object that exited; may be null and is forwarded unchanged.
	 */
	static void SendObjectExitedNotification(UObject* Receiver, const UObject* ExitedObject);
};
