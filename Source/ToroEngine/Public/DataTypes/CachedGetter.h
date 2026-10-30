// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UObject/Object.h"
#include "Misc/AssertionMacros.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Templates/UnrealTypeTraits.h"
#include "Templates/UnrealTemplate.h"
#include "Templates/Function.h"

/**
 * Lazily resolves a UObject and reuses it while its weak cache remains valid.
 *
 * The cache does not keep the object alive. Invalid cached results are retried on
 * each Get() call. Getter captures must remain safe to use whenever it is invoked.
 * Access is not synchronized; callers must serialize use.
 *
 * @tparam T A complete UObject-derived type.
 */
template <typename T>
class TCachedGetter final
{
	static_assert(TIsDerivedFrom<T, UObject>::Value, "T must derive from UObject.");

	/** Last resolved object, held weakly so it can still be garbage collected. */
	TWeakObjectPtr<T> Cache;

	/** Owned lookup callable; captured dependencies must remain valid when invoked. */
	TFunction<T*()> GetFunc;

public:

	/** This constructor should not be used. */
	TCachedGetter() = default;

	TCachedGetter(const TCachedGetter&) = delete;
	TCachedGetter& operator=(const TCachedGetter&) = delete;

	/**
	 * Initializes an empty cache without invoking the supplied getter.
	 * @param Getter Bound lookup callable returning an object or nullptr when unavailable.
	 *        Any captured dependencies must remain safe to access whenever a lookup occurs.
	 */
	TCachedGetter(TFunction<T*()> Getter)
		: Cache(nullptr), GetFunc(MoveTemp(Getter))
	{}

	[[nodiscard]] T& operator*()
	{
		return *GetChecked();
	}

	[[nodiscard]] T* operator->()
	{
		return GetChecked();
	}

	/** Returns whether the cached object is currently valid without invoking the getter. */
	[[nodiscard]] FORCEINLINE bool IsCached() const
	{
		return Cache.IsValid();
	}

	/**
	 * Invokes the getter unconditionally and replaces the weak cache with its result.
	 * Requires a bound getter; an empty callable triggers an assertion when checks
	 * are enabled. An invalid result replaces any previously cached object.
	 * @return The resolved object if the updated weak cache is valid, otherwise nullptr.
	 */
	[[nodiscard]] T* Fetch()
	{
		checkf(GetFunc, TEXT("TCachedGetter::Fetch called without a getter."));
		Cache = GetFunc();
		return Cache.Get();
	}

	/**
	 * Returns the valid cached object, or invokes Fetch() if the cache is invalid.
	 * Null or invalid results cause another lookup on the next call.
	 * @return The cached or newly resolved object, or nullptr if no valid object is available.
	 */
	[[nodiscard]] T* Get()
	{
		return IsCached() ? Cache.Get() : Fetch();
	}

	/**
	 * Gets the object and asserts that it is valid when checks are enabled.
	 * Uses the cache and lookup behavior of Get(). Callers must ensure resolution
	 * succeeds; the assertion provides no recovery or validity guarantee when disabled.
	 * @return The pointer returned by Get().
	 */
	[[nodiscard]] T* GetChecked()
	{
		T* Ptr = Get();
		checkf(::IsValid(Ptr), TEXT("TCachedGetter's getter could not get the object."));
		return Ptr;
	}

	/**
	 * Clears the weak cache while retaining the getter for the next lookup.
	 * Does not invoke the getter or affect the lifetime of the previously cached object.
	 */
	FORCEINLINE void Reset()
	{
		Cache.Reset();
	}
};
