// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UObject/Interface.h"
#include "SaveSerializeInterface.generated.h"

/**
 * Reflected interface type identifying objects that receive save serialization notifications.
 */
UINTERFACE(MinimalAPI)
class USaveSerializeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for receiving notifications before and after saving or loading object data.
 * The static helpers validate the target and use reflected dispatch to support native and Blueprint implementations.
 * These helpers only send notifications; they do not perform serialization.
 */
class TOROSYSTEMS_API ISaveSerializeInterface
{
	GENERATED_BODY()

protected:

	/**
	 * Implementation hook for preparing an object before its data is saved or loaded.
	 * The default native implementation does nothing.
	 * @param bIsLoading True when loading data into the object; false when saving its data.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = SaveSystem)
	void OnPreSerialize(bool bIsLoading);
	virtual void OnPreSerialize_Implementation(bool bIsLoading) {}

	/**
	 * Implementation hook for responding after an object's data has been saved or loaded.
	 * The default native implementation does nothing. Notification does not imply serialization succeeded.
	 * @param bIsLoading True when loading data into the object; false when saving its data.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = SaveSystem)
	void OnPostSerialize(bool bIsLoading);
	virtual void OnPostSerialize_Implementation(bool bIsLoading) {}

public:

	/**
	 * Sends a pre-serialization notification through reflected interface dispatch.
	 * Does nothing for null, invalid, or non-implementing targets; does not perform serialization.
	 * @param Target Object receiving the notification; may be null.
	 * @param bIsLoading True when loading data into the object; false when saving its data.
	 */
	static void PreSerialize(UObject* Target, bool bIsLoading);

	/**
	 * Sends a post-serialization notification through reflected interface dispatch.
	 * Does nothing for null, invalid, or non-implementing targets; does not perform serialization.
	 * @param Target Object receiving the notification; may be null.
	 * @param bIsLoading True when loading data into the object; false when saving its data.
	 */
	static void PostSerialize(UObject* Target, bool bIsLoading);
};
