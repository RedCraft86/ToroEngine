// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "UObject/Interface.h"
#include "ToroExitInterface.generated.h"

/**
 * Reflected interface for requesting an object's exit and notifying another object of an exit.
 * See IToroExitInterface for event contracts and C++ dispatch helpers.
 */
UINTERFACE()
class UToroExitInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Supports exit requests and explicit notifications identifying an object that has exited.
 * Accepting a request does not guarantee that exit has completed. Neither event automatically
 * invokes the other; callers send notifications explicitly to the intended receiver.
 * Dispatch helpers support native and Blueprint implementations.
 */
class TOROCORE_API IToroExitInterface
{
	GENERATED_BODY()

protected:

	/**
	 * Requests that this object exit. The default implementation rejects the request.
	 * @param Requester Object initiating the request; may be null.
	 * @return True if the request is accepted or handled, even if exit completes later.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Exit)
	bool RequestExit(const UObject* Requester);
	virtual bool RequestExit_Implementation(const UObject* Requester)
	{
		return false;
	}

	/**
	 * Notifies this receiver that another object has exited; ignored by default.
	 * @param ExitedObject Object that exited; may be null and is not validated by the dispatcher.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Exit)
	void NotifyObjectExited(const UObject* ExitedObject);
	virtual void NotifyObjectExited_Implementation(const UObject* ExitedObject) {}

public:

	/**
	 * Dispatches RequestExit to a valid target implementing this interface.
	 * @param Target Object being asked to exit.
	 * @param Requester Initiating object, forwarded unchanged and allowed to be null.
	 * @return The receiver's acceptance result, or false for invalid or unsupported targets.
	 * A true result does not guarantee exit completion.
	 */
	static bool SendExitRequest(UObject* Target, const UObject* Requester);

	/**
	 * Dispatches NotifyObjectExited to a valid receiver implementing this interface.
	 * Invalid or unsupported receivers are ignored. Does not request or perform an exit.
	 * @param Receiver Object to notify.
	 * @param ExitedObject Object that exited, forwarded unchanged and allowed to be null.
	 */
	static void SendObjectExitedNotification(UObject* Receiver, const UObject* ExitedObject);
};
