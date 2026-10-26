// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroObjectLibrary.generated.h"

/**
 * Blueprint utilities for parameterless reflected calls, template and destruction flags, and loaded-object searches.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroObjectLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Dispatches an exact-name reflected function with no parameters or return value.
	 * @param Target Object receiving the call.
	 * @param EventName Exact reflected function name, not command text or arguments.
	 * @return True after ProcessEvent dispatch; this does not report the function's own success.
	 *         False for an invalid target, empty name, missing function, or any input, output, or return parameters.
	 */
	UFUNCTION(BlueprintCallable, Category = Object, meta = (DefaultToSelf = Target))
	static bool CallObjectEvent(UObject* Target, FName EventName);

	/**
	 * Tests native template status, including class default objects and archetypes.
	 * @param Target Object to inspect.
	 * @return True if the target is valid and native IsTemplate() reports a template; false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = Object, meta = (DefaultToSelf = Target))
	static bool IsTemplateObject(UObject* Target);

	/**
	 * Tests RF_BeginDestroyed, indicating that BeginDestroy has been called.
	 * This flag differs from generic validity or garbage status and can remain set after destruction advances.
	 * @param Target Accessible allocated object to inspect; nullptr returns false. Freed storage cannot be safely inspected.
	 */
	UFUNCTION(BlueprintCallable, Category = Object, meta = (DefaultToSelf = Target))
	static bool IsObjectBeingDestroyed(UObject* Target);

	/**
	 * Tests RF_FinishDestroyed, indicating that FinishDestroy has been called.
	 * This flag differs from generic validity or garbage status and does not make freed storage safe to inspect.
	 * @param Target Accessible allocated object to inspect; nullptr returns false.
	 */
	UFUNCTION(BlueprintCallable, Category = Object, meta = (DefaultToSelf = Target))
	static bool HasObjectBeenDestroyed(UObject* Target);

	/**
	 * Collects valid loaded instances of a class and its subclasses, excluding class default objects.
	 * Does not load assets; result ordering is unspecified.
	 * @param Result Replaced with matching objects; empty for invalid classes or no matches.
	 * @param InClass Class to search. UObject itself is rejected with an error to avoid an unrestricted search.
	 * @param WorldFilter A valid world restricts matches to objects whose GetWorld() equals it;
	 *        null or invalid filters leave the search unrestricted across worlds and worldless objects.
	 */
	UFUNCTION(BlueprintCallable, Category = Object, meta = (DeterminesOutputType = InClass, DynamicOutputParam = Result))
	static void GetAllObjectsOfClass(TArray<UObject*>& Result, TSubclassOf<UObject> InClass, const UWorld* WorldFilter);

	/**
	 * Gets the first valid loaded instance of a class or its subclasses, excluding class default objects.
	 * Does not load assets; selection is unspecified when multiple objects match.
	 * @param InClass Class to search. UObject itself is rejected with an error to avoid an unrestricted search.
	 * @param WorldFilter A valid world restricts matches to objects whose GetWorld() equals it;
	 *        null or invalid filters leave the search unrestricted across worlds and worldless objects.
	 * @return The first matching object, or nullptr for an invalid or rejected class or no match.
	 */
	UFUNCTION(BlueprintCallable, Category = Object, meta = (DeterminesOutputType = InClass))
	[[nodiscard]] static UObject* GetObjectOfClass(TSubclassOf<UObject> InClass, const UWorld* WorldFilter);
};
