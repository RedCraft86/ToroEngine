// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Containers/Map.h"
#include "Templates/Models.h"
#include "Templates/Function.h"
#include "Templates/SharedPointer.h"
#include "Templates/UnrealTemplate.h"
#include "Concepts/EqualityComparable.h"
#include "Concepts/GetTypeHashable.h"
#include "Misc/AssertionMacros.h"
#include "Delegates/Delegate.h"

/**
 * Shared-owned map with optional validation and synchronous change notifications.
 *
 * Create instances with Make(). Successful additions and removals also prune invalid
 * entries before notifying listeners. Read access does not perform validation.
 * Access is not synchronized; callers must serialize use. Validators must not mutate
 * this accumulator while validation is in progress.
 *
 * @tparam K Copyable key type supporting GetTypeHash and equality comparisons.
 * @tparam V Copyable mapped value type; equality comparison is not required.
 */
template<typename K, typename V>
class TAccumulatorMap final : public TSharedFromThis<TAccumulatorMap<K, V>>
{
	static_assert(TModels_V<CGetTypeHashable, K>, "K must support GetTypeHash.");
	static_assert(TModels_V<CEqualityComparable, K>, "K must support equality comparisons.");

	/** Stored entries; validation is deferred until a mutating operation runs cleanup. */
	TMap<K, V> InnerMap;

	/** Optional predicate used to accept additions and identify entries to retain. */
	TFunction<bool(const K&, const V&)> Validate;

	/**
	 * Initializes an empty accumulator for the shared-ownership factory.
	 * @param Validator Optional predicate returning true for entries to retain.
	 */
	TAccumulatorMap(TFunction<bool(const K&, const V&)> Validator)
		: InnerMap(), Validate(MoveTemp(Validator))
	{}

	/**
	 * Removes entries rejected by the validator without broadcasting OnChanged.
	 * @return True if any entries were removed; false also when no validator is set.
	 */
	bool CleanupInternal()
	{
		if (Validate)
		{
			bool bChanged = false;
			for (auto It = InnerMap.CreateIterator(); It; ++It)
			{
				if (!Validate(It.Key(), It.Value()))
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
	 * or after explicit Cleanup() removes entries.
	 * Accepted additions notify even when replacing a value with an equal value.
	 */
	FOnChangedDelegate OnChanged;

	TAccumulatorMap() = delete;
	TAccumulatorMap(const TAccumulatorMap&) = delete;
	TAccumulatorMap& operator=(const TAccumulatorMap&) = delete;

	/**
	 * Creates an empty, shared-owned accumulator.
	 * @param Validator Optional predicate returning true to accept or retain entries.
	 *        It may run more than once per addition and must not mutate this accumulator.
	 *        An empty predicate accepts all additions and disables automatic pruning.
	 * @return A shared reference owning the new accumulator.
	 */
	[[nodiscard]] static TSharedRef<TAccumulatorMap<K, V>> Make(TFunction<bool(const K&, const V&)> Validator = nullptr)
	{
		return MakeShareable(new TAccumulatorMap<K, V>(MoveTemp(Validator)));
	}

	/**
	 * Provides read-only access to the stored map without running validation.
	 * The reference is valid only while this accumulator lives. Mutations affect the
	 * same container and may invalidate references and iterators to its contents.
	 */
	[[nodiscard]] FORCEINLINE const TMap<K, V>& GetInnerMap() const
	{
		return InnerMap;
	}

	/** Returns the stored entry count without running validation. */
	[[nodiscard]] FORCEINLINE int32 Num() const
	{
		return InnerMap.Num();
	}

	/** Returns whether the stored map is empty without running validation. */
	[[nodiscard]] FORCEINLINE bool IsEmpty() const
	{
		return InnerMap.IsEmpty();
	}

	/**
	 * Returns a const iterator to the first stored entry without running validation.
	 * The accumulator must outlive iteration, and mutations may invalidate the iterator.
	 */
	[[nodiscard]] FORCEINLINE auto begin() const
	{
		return InnerMap.begin();
	}

	/**
	 * Returns the end sentinel for read-only iteration without running validation.
	 * The accumulator must outlive iteration, and mutations may invalidate the sentinel.
	 */
	[[nodiscard]] FORCEINLINE auto end() const
	{
		return InnerMap.end();
	}

	/**
	 * Adds or replaces a key/value pair if accepted by the validator.
	 * An accepted pair is stored, invalid entries are pruned, and OnChanged broadcasts
	 * once, even if the supplied value equals the previous value. A rejected pair
	 * leaves existing entries untouched and skips cleanup and notification.
	 * @param Key Key to insert or replace.
	 * @param Value Value to copy into the map.
	 */
	void Add(const K& Key, const V& Value)
	{
		if (!Validate || Validate(Key, Value))
		{
			InnerMap.Add(Key, Value);
			CleanupInternal();
			OnChanged.Broadcast();
		}
	}

	/**
	 * Removes the matching entry, then prunes invalid entries and broadcasts OnChanged once.
	 * If no match exists, cleanup and notification are skipped.
	 * @param Key Key of the entry to remove.
	 */
	void Remove(const K& Key)
	{
		if (InnerMap.Remove(Key) > 0)
		{
			CleanupInternal();
			OnChanged.Broadcast();
		}
	}

	/**
	 * Prunes all entries rejected by the validator and broadcasts OnChanged once if any were removed.
	 * Requires a validator. Without one, triggers an ensure diagnostic and leaves the map unchanged.
	 */
	void Cleanup()
	{
		if (ensureMsgf(Validate, TEXT("TAccumulatorMap::Cleanup called without a validator.")))
		{
			if (CleanupInternal())
			{
				OnChanged.Broadcast();
			}
		}
	}
};
