// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroClassLibrary.generated.h"

/**
 * Queries default objects and currently loaded derived classes.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroClassLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Returns the class default object, creating it if needed, or nullptr for an invalid class.
	 * @param InClass Class whose defaults are queried.
	 */
	UFUNCTION(BlueprintPure, Category = Class, meta = (DeterminesOutputType = InClass))
	[[nodiscard]] static const UObject* GetClassDefaultObject(TSubclassOf<UObject> InClass);

	/**
	 * Gets currently loaded derived classes; does not load assets.
	 * @param Results Receives additional matching classes in unspecified order.
	 * @param InClass Base class to search beneath; the base itself is excluded.
	 * @param bRecursive Include all descendants rather than only direct children.
	 */
	UFUNCTION(BlueprintPure, Category = Class, meta = (DeterminesOutputType = InClass, DynamicOutputParam = Results))
	static void GetDerivedClasses(TArray<UClass*>& Results, TSubclassOf<UObject> InClass, bool bRecursive = true);
};
