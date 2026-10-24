// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UObject/Class.h"
#include "UObject/Interface.h"
#include "Templates/UnrealTypeTraits.h"

/**
 * Checks if an interface is implemented on the target object.
 * @tparam T Reflected UInterface subclass, such as UActivatableObject, rather than its native I... type.
 * @param Target Object to inspect; may be null.
 * @return True when Target is valid and implements T; false for null, invalid, or non-implementing objects.
 */
template<typename T>
TOROENGINE_API bool ObjectImplementsInterface(const UObject* Target)
{
	static_assert(TIsDerivedFrom<T, UInterface>::Value, "T must derive from UInterface");
	return IsValid(Target) && Target->Implements<T>();
}
