// Copyright (C) RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UObject/NameTypes.h"

class UObject;
class AActor;
class UWorld;

namespace ToroEngine::Validation
{
	/**
	 * Runs IsDataValid() on the object with the manual validation use case.
	 * Adds a log page, forwards validation issues without changing their severity, and
	 * opens the Message Log. Invalid targets are ignored.
	 * @param Target Object to validate.
	 * @param LogName Destination log; NAME_None selects the default data-validation log.
	 */
	TOROENGINEED_API void ValidateObject(const UObject* Target, FName LogName = NAME_None);

	/**
	 * Runs IsDataValid() on the asset with the manual validation use case.
	 * Adds an AssetCheck page and opens the Message Log. Invalid targets are ignored.
	 * @param Target Asset object to validate directly.
	 */
	TOROENGINEED_API void ValidateAsset(const UObject* Target);

	/**
	 * Runs IsDataValid() on the actor with the manual validation use case.
	 * Adds a MapCheck page and opens the Message Log. Invalid targets are ignored.
	 * @param Target Actor to validate.
	 */
	TOROENGINEED_API void ValidateActor(const AActor* Target);

	/**
	 * Runs world validation, including loaded actors through UWorld::IsDataValid().
	 * Uses the manual validation use case, adds a MapCheck page, and opens the Message
	 * Log without explicitly clearing existing messages. Invalid targets are ignored.
	 * @param Target World whose loaded contents are validated.
	 */
	TOROENGINEED_API void ValidateWorld(const UWorld* Target);
}
