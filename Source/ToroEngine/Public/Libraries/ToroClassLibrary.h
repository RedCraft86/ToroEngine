// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroClassLibrary.generated.h"

/**
 * Blueprint utilities for class default objects, parent hierarchies, and loaded subclasses.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroClassLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Gets the shared class default object, creating it if necessary.
	 * This is not a newly created instance; treat the returned defaults as read-only.
	 * @param InClass Class whose default object is requested.
	 * @return The shared class default object, or nullptr for invalid input.
	 */
	UFUNCTION(BlueprintPure, Category = Class, meta = (DeterminesOutputType = InClass))
	[[nodiscard]] static const UObject* GetClassDefaultObject(TSubclassOf<UObject> InClass);

	/**
	 * Gets the immediate superclass.
	 * @param InClass Class whose parent is requested.
	 * @return The immediate parent class, or nullptr for invalid input or a root class.
	 */
	UFUNCTION(BlueprintPure, Category = Class)
	[[nodiscard]] static UClass* GetClassParent(TSubclassOf<UObject> InClass);

	/**
	 * Collects valid ancestors from the immediate parent toward the root, excluding the input class.
	 * @param Results Replaced with the ancestor classes in order from the immediate parent toward the root; empty for invalid input or a root class.
	 * @param InClass Class whose parent hierarchy is requested.
	 */
	UFUNCTION(BlueprintCallable, Category = Class)
	static void GetClassParentHierarchy(TArray<UClass*>& Results, TSubclassOf<UObject> InClass);

	/**
	 * Collects currently loaded subclasses, excluding the input class.
	 * Does not discover or load unloaded Blueprint assets; result ordering is unspecified.
	 * @param Results Replaced with the derived classes; empty for null input or when no subclasses are found.
	 * @param InClass Base class whose descendants are requested.
	 * @param bRecursive Whether to include all descendants rather than only direct subclasses.
	 */
	UFUNCTION(BlueprintCallable, Category = Class, meta = (DeterminesOutputType = InClass, DynamicOutputParam = Results))
	static void GetDerivedClasses(TArray<UClass*>& Results, TSubclassOf<UObject> InClass, bool bRecursive = true);
};
