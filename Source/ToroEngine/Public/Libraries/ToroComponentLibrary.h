// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroComponentLibrary.generated.h"

/**
 * Blueprint utilities for component destruction permissions and world-wide component lookup.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroComponentLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Gets whether external callers may destroy this component through Destroy Component.
	 * @param Target Component whose destruction permission is queried.
	 * @return True if the component is valid and allows external destruction; false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = Components, meta = (DefaultToSelf = Target))
	[[nodiscard]] static bool GetAllowAnyoneToDestroyMe(const UActorComponent* Target);

	/**
	 * Sets whether external callers may destroy this component through Destroy Component.
	 * Changing this permission does not destroy the component; invalid targets are ignored.
	 * @param Target Component whose destruction permission is changed.
	 * @param bAllow Whether to allow callers other than the component or its owning actor to destroy it.
	 */
	UFUNCTION(BlueprintCallable, Category = Components, meta = (DefaultToSelf = Target))
	static void SetAllowAnyoneToDestroyMe(UActorComponent* Target, bool bAllow);

	/**
	 * Collects valid loaded component instances matching the class or its subclasses in the context's world.
	 * Scans component objects across the world on the game thread, not just components owned by the context actor.
	 * Excludes class default objects; components need not be registered and result ordering is unspecified.
	 * @param InClass Component base class to match.
	 * @param Result Replaced with the matching components; empty for an invalid class or unavailable context world.
	 */
	UFUNCTION(BlueprintCallable, Category = Components, meta = (WorldContext = ContextObject, DeterminesOutputType = InClass, DynamicOutputParam = Result))
	static void GetAllComponentsOfClass(const UObject* ContextObject, TSubclassOf<UActorComponent> InClass, TArray<UActorComponent*>& Result);

	/**
	 * Finds the first valid loaded component matching the class or its subclasses in the context's world.
	 * Scans component objects across the world on the game thread, not just components owned by the context actor.
	 * Excludes class default objects; registration is not required and selection is unspecified when multiple matches exist.
	 * @param InClass Component base class to match.
	 * @return The first iterator match, or nullptr for an invalid class, unavailable context world, or no match.
	 */
	UFUNCTION(BlueprintCallable, Category = Components, meta = (WorldContext = ContextObject, DeterminesOutputType = InClass))
	[[nodiscard]] static UActorComponent* GetComponentOfClass(const UObject* ContextObject, TSubclassOf<UActorComponent> InClass);
};
