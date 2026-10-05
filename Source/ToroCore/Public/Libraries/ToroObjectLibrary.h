// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroObjectLibrary.generated.h"

/**
 * Named event invocation and queries over currently loaded UObject instances.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroObjectLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Invokes a named function using the engine's text-command parser; errors are discarded.
	 * @param Target Object to invoke on; invalid objects are ignored.
	 * @param EventName Function name or command text. NAME_None is ignored; omitted arguments use parser defaults.
	 */
	UFUNCTION(BlueprintCallable, Category = Object, meta = (DefaultToSelf = Target))
	static void CallObjectEvent(UObject* Target, FName EventName);

	/**
	 * Queries loaded objects of a class and its subclasses; this is not restricted to a world.
	 * @param Result Replaced with matching instances in unspecified order; default objects are excluded by the engine.
	 * @param InClass Class to query. Invalid classes and UObject itself yield an empty result; assets are not loaded.
	 */
	UFUNCTION(BlueprintCallable, Category = Object, meta = (DeterminesOutputType = InClass, DynamicOutputParam = Result))
	static void GetAllObjectsOfClass(TArray<UObject*>& Result, TSubclassOf<UObject> InClass);

	/**
	 * Returns the first valid loaded instance of a class or subclass in unspecified order.
	 * The search is global rather than world-scoped and does not load assets.
	 * @param InClass Class to query; invalid classes and UObject itself yield nullptr.
	 * @return Matching instance, or nullptr if none is found; default objects are excluded.
	 */
	UFUNCTION(BlueprintCallable, Category = Object, meta = (DeterminesOutputType = InClass))
	[[nodiscard]] static UObject* GetObjectOfClass(TSubclassOf<UObject> InClass);
};
