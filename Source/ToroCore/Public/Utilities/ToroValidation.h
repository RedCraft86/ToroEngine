// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "UObject/NameTypes.h"

class AActor;
class UObject;

/**
 * Editor-only helpers that run a target's own IsDataValid implementation and open its results log.
 * Additional validators registered with the editor validator subsystem are intentionally excluded.
 */
namespace ToroEngine::Validation
{
#if WITH_EDITOR
	/**
	 * Runs the actor's own data validation and displays issues and the result in MapCheck.
	 * Does not invoke CheckForErrors or perform a full map check.
	 */
	TOROCORE_API void ValidateActor(const AActor* Target);

	/** Runs the asset's own data validation and displays issues and the result in AssetCheck. */
	TOROCORE_API void ValidateAsset(const UObject* Target);

	/**
	 * Runs the object's own data validation and displays issues and the result in the requested log.
	 * @param Target Object to validate; null or invalid objects are ignored.
	 * @param LogName Destination message log; NAME_None selects AssetCheck.
	 */
	TOROCORE_API void ValidateObject(const UObject* Target, const FName LogName = NAME_None);
#endif
}
