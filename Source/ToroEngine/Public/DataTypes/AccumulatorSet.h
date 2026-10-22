// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Containers/Set.h"
#include "Templates/Models.h"
#include "Templates/Function.h"
#include "Templates/SharedPointer.h"
#include "Concepts/EqualityComparable.h"
#include "Concepts/GetTypeHashable.h"
#include "Delegates/Delegate.h"

/**
 * Shared-owned set with optional validation and synchronous change notifications.
 *
 * Create instances with Make(). Successful additions and removals also prune invalid
 * elements before notifying listeners. Read access does not perform validation.
 * Access is not synchronized; callers must serialize use. Validators must not mutate
 * this accumulator while validation is in progress.
 *
 * @tparam T Copyable element type supporting GetTypeHash and equality comparisons.
 */
template<typename T>
class TAccumulatorSet final : public TSharedFromThis<TAccumulatorSet<T>>
{
	static_assert(TModels_V<CGetTypeHashable, T>, "T must support GetTypeHash.");
	static_assert(TModels_V<CEqualityComparable, T>, "T must support equality comparisons.");

	/** Stored elements; validation is deferred until a mutating operation runs cleanup. */
	TSet<T> InnerSet;

	/** Optional predicate used to accept additions and identify elements to retain. */
	TFunction<bool(const T&)> Validate;

	/**
	 * Initializes an empty accumulator for the shared-ownership factory.
	 * @param Validator Optional predicate returning true for elements to retain.
	 */
	TAccumulatorSet(TFunction<bool(const T&)> Validator)
		: InnerSet(), Validate(MoveTemp(Validator))
	{}

	/**
	 * Removes elements rejected by the validator without broadcasting OnChanged.
	 * @return True if any elements were removed; false also when no validator is set.
	 */
	bool CleanupInternal()
	{
		if (Validate)
		{
			bool bChanged = false;
			for (auto It = InnerSet.CreateIterator(); It; ++It)
			{
				if (!Validate(*It))
				{
					It.RemoveCurrent();
					bChanged = true;
				}
			}

			return bChanged;
		}

		return false;
	}

public:

	DECLARE_MULTICAST_DELEGATE(FOnChangedDelegate);

	/**
	 * Broadcasts synchronously after a successful addition or removal and its cleanup,
	 * or after explicit Cleanup() removes elements.
	 */
	FOnChangedDelegate OnChanged;

	TAccumulatorSet() = delete;
	TAccumulatorSet(const TAccumulatorSet&) = delete;
	TAccumulatorSet& operator=(const TAccumulatorSet&) = delete;

	/**
	 * Creates an empty, shared-owned accumulator.
	 * @param Validator Optional predicate returning true to accept or retain elements.
	 *        It may run more than once per addition and must not mutate this accumulator.
	 *        An empty predicate accepts all additions and disables automatic pruning.
	 * @return A shared reference owning the new accumulator.
	 */
	[[nodiscard]] static TSharedRef<TAccumulatorSet<T>> Make(TFunction<bool(const T&)> Validator = nullptr)
	{
		return MakeShareable(new TAccumulatorSet<T>(MoveTemp(Validator)));
	}

	/**
	 * Provides read-only access to the stored set without running validation.
	 * The reference is valid only while this accumulator lives. Mutations affect the
	 * same container and may invalidate references and iterators to its contents.
	 */
	[[nodiscard]] FORCEINLINE const TSet<T>& GetInnerSet() const
	{
		return InnerSet;
	}

	/** Returns the stored element count without running validation. */
	[[nodiscard]] FORCEINLINE int32 Num() const
	{
		return InnerSet.Num();
	}

	/** Returns whether the stored set is empty without running validation. */
	[[nodiscard]] FORCEINLINE bool IsEmpty() const
	{
		return InnerSet.IsEmpty();
	}

	/**
	 * Returns a const iterator to the first stored element without running validation.
	 * The accumulator must outlive iteration, and mutations may invalidate the iterator.
	 */
	[[nodiscard]] FORCEINLINE auto begin() const
	{
		return InnerSet.begin();
	}

	/**
	 * Returns the end sentinel for read-only iteration without running validation.
	 * The accumulator must outlive iteration, and mutations may invalidate the sentinel.
	 */
	[[nodiscard]] FORCEINLINE auto end() const
	{
		return InnerSet.end();
	}

	/**
	 * Adds a value if accepted by the validator and not already present.
	 * After insertion, invalid elements are pruned and OnChanged broadcasts once.
	 * Rejected or duplicate values skip cleanup and notification.
	 * @param Value Element to copy into the set.
	 */
	void Add(const T& Value)
	{
		if ((!Validate || Validate(Value)) && !InnerSet.Contains(Value))
		{
			InnerSet.Add(Value);
			CleanupInternal();
			OnChanged.Broadcast();
		}
	}

	/**
	 * Removes the matching element, then prunes invalid elements and broadcasts OnChanged once.
	 * If no match exists, cleanup and notification are skipped.
	 * @param Value Element to remove.
	 */
	void Remove(const T& Value)
	{
		if (InnerSet.Remove(Value) > 0)
		{
			CleanupInternal();
			OnChanged.Broadcast();
		}
	}

	/**
	 * Prunes all elements rejected by the validator and broadcasts OnChanged once if any were removed.
	 * Requires a validator. Without one, triggers an ensure diagnostic and leaves the set unchanged.
	 */
	void Cleanup()
	{
		if (ensureMsgf(Validate, TEXT("TAccumulatorSet::Cleanup called without a validator.")))
		{
			if (CleanupInternal())
			{
				OnChanged.Broadcast();
			}
		}
	}
};
