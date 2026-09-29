// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "CoreTypes.h"
#include "UObject/Object.h"
#include "Templates/Function.h"
#include "Templates/UnrealTemplate.h"
#include "Templates/UnrealTypeTraits.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "UObject/WeakObjectPtr.h"
#include "Misc/AssertionMacros.h"

/**
 * Lazily retrieves a UObject and caches it through a weak reference.
 * Get() reuses a valid cached object; Fetch() explicitly refreshes the cache.
 * Unsuccessful lookups are retried on subsequent Get() calls.
 *
 * The weak cache does not keep UObjects alive. Getter captures must remain
 * safe to invoke; a raw captured owner must outlive calls that execute the getter.
 *
 * @tparam T UObject-derived type returned by the getter.
 */
template <typename T>
class TCachedGetter final
{
	/** Weak reference to the most recently fetched object; does not keep it alive. */
	TWeakObjectPtr<T> Cache;

	/** Stored callable used to retrieve an object when the cache needs refreshing. */
	TFunction<T*()> Getter;

	/** Default construction is disabled; a getter argument must be supplied. */
	TCachedGetter() = delete;

public:

	/**
	 * Constructs an empty cache and stores the callable without invoking it.
	 * @param GetFunc Getter whose captures must remain safe whenever it is invoked.
	 */
	TCachedGetter(TFunction<T*()> GetFunc)
		: Getter(MoveTemp(GetFunc))
	{
		static_assert(TIsDerivedFrom<T, UObject>::Value, "T must be derived from UObject");
		Cache.Reset();
	}

	/**
	 * Retrieves the object through Get() and checks its validity with checkf.
	 * @return The retrieved object pointer.
	 */
	T* operator->()
	{
		T* Ptr = Get();
		checkf(::IsValid(Ptr), TEXT("Called TCachedGetter -> operator with an invalid object."));
		return Ptr;
	}

	/** Returns whether the current cache is valid without invoking the getter. */
	FORCEINLINE bool IsValid() const
	{
		return Cache.IsValid();
	}

	/**
	 * Invokes the bound getter and replaces the cache, even if it is already valid.
	 * An empty getter triggers an ensure diagnostic and leaves the cache unchanged.
	 * @return The resolved weak cache pointer, which may be null.
	 */
	T* Fetch()
	{
		if (ensureMsgf(Getter, TEXT("TCachedGetter has no getter bound.")))
		{
			Cache = Getter();
		}

		return Cache.Get();
	}

	/**
	 * Reuses a valid cached object until it expires or Fetch() is called.
	 * Otherwise, fetches and may return null; failed lookups are retried on later calls.
	 */
	T* Get()
	{
		return IsValid() ? Cache.Get() : Fetch();
	}
};
