// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToroCore.h"
#include "UObject/UObjectGlobals.h"
#include "Traits/UObjectPtrTraits.h"

/**
 * Stores values under unique keys with optional key validation and change notifications.
 * Accepted additions and successful removals also clean up entries with rejected keys.
 * Accessors and iteration expose stored entries without performing validation.
 *
 * Stored UObject keys and values are not kept alive by this container.
 * Use TWeakObjectPtr for non-owning references; a key validity validator can remove
 * expired keys, but values are not validated and do not determine entry lifetime.
 *
 * @tparam K Key type supporting the hashing and equality operations required by TMap.
 * @tparam V Mapped value type. Value equality comparisons are not required.
 */
template <typename K, typename V>
class TAccumulatorMap final : public TSharedFromThis<TAccumulatorMap<K, V>>
{
	/** Stored key-value pairs, which may include entries awaiting key validation. */
	TMap<K, V> InnerMap;

	/** Optional predicate returning true for keys whose entries may be added or retained. */
	TFunction<bool(const K&)> Validate;

	/** Constructs an empty map and takes ownership of the supplied key validator. */
	TAccumulatorMap(TFunction<bool(const K&)> Validator)
		: InnerMap(), Validate(MoveTemp(Validator))
	{}

public:

	/**
	 * Broadcasts once after an accepted addition or successful removal and its cleanup.
	 * Additions that overwrite an existing value also notify, even if the value is equal.
	 * Cleanup broadcasts only when it removes entries and notification is requested,
	 * including automatic post-GC cleanup.
	 */
	TMulticastDelegate<void()> OnChanged;

	/** Constructs an empty map without validation or automatic post-GC cleanup. */
	TAccumulatorMap()
		: InnerMap(), Validate(nullptr)
	{}

	TAccumulatorMap(const TAccumulatorMap&) = delete;
	TAccumulatorMap& operator=(const TAccumulatorMap&) = delete;

	/** Removes any post-garbage-collection delegate bindings for this instance. */
	~TAccumulatorMap()
	{
		FCoreUObjectDelegates::GetPostGarbageCollect().RemoveAll(this);
	}

	/** Creates a shared empty map without validation or automatic post-GC cleanup. */
	[[nodiscard]] static TSharedRef<TAccumulatorMap<K, V>> Create()
	{
		return MakeShareable(new TAccumulatorMap<K, V>());
	}

	/**
	 * Creates a shared empty map with optional key validation.
	 * A valid validator enables automatic notifying post-GC cleanup only when
	 * TIsUObjectType<K>::Value is true, regardless of V. An empty validator behaves like Create().
	 *
	 * @param Validator Predicate returning true for keys whose entries may be added or retained.
	 * Validators must not mutate this map or recursively invoke Cleanup,
	 * because they can run while Cleanup is iterating the live map.
	 */
	[[nodiscard]] static TSharedRef<TAccumulatorMap<K, V>> Create(TFunction<bool(const K&)> Validator)
	{
		if (!Validator)
		{
			return Create();
		}

		TSharedRef<TAccumulatorMap<K, V>> Instance = MakeShareable(new TAccumulatorMap<K, V>(MoveTemp(Validator)));
		if constexpr (TIsUObjectType<K>::Value)
		{
			FCoreUObjectDelegates::GetPostGarbageCollect().AddSP(Instance, &TAccumulatorMap::Cleanup, true);
		}

		return Instance;
	}

	/** Returns a read-only reference to the live map without validating its keys. */
	[[nodiscard]] FORCEINLINE const TMap<K, V>& GetInnerMap() const
	{
		return InnerMap;
	}

	/** Returns the number of stored entries without performing validation. */
	[[nodiscard]] FORCEINLINE int32 Num() const
	{
		return InnerMap.Num();
	}

	/** Returns whether there are no stored entries, without performing validation. */
	[[nodiscard]] FORCEINLINE bool IsEmpty() const
	{
		return InnerMap.IsEmpty();
	}

	/**
	 * Visits stored key-value pairs without validating their keys.
	 * An empty callback logs an error and returns without visiting entries.
	 * Callbacks must not mutate this map, including through Cleanup,
	 * because iteration uses the live map and mutations can invalidate its iterator.
	 *
	 * @param Func Callback returning true to continue iteration or false to stop.
	 */
	void ForEach(const TFunction<bool(const K&, const V&)>& Func) const
	{
		if (!Func)
		{
			UE_LOG(LogToroCore, Error, TEXT("Called TAccumulatorMap::ForEach without a valid loop function."));
			return;
		}

		for (const TPair<K, V>& Pair : InnerMap)
		{
			if (!Func(Pair.Key, Pair.Value))
			{
				break;
			}
		}
	}

	/**
	 * Accepted keys insert or overwrite their values, then clean up and notify once.
	 * Equal-value replacements also notify; no value equality comparison is required.
	 * Rejected keys leave the map unchanged, without cleanup or notification.
	 */
	void Add(const K& Key, const V& Value)
	{
		if (Validate && !Validate(Key))
		{
			return;
		}

		InnerMap.Add(Key, Value);

		Cleanup(false);
		OnChanged.Broadcast();
	}

	/**
	 * Removes the entry for a key, cleans up rejected keys, then broadcasts OnChanged once.
	 * If the key is absent, performs no cleanup or notification.
	 */
	void Remove(const K& Key)
	{
		if (InnerMap.Remove(Key) > 0)
		{
			Cleanup(false);
			OnChanged.Broadcast();
		}
	}

	/**
	 * Returns a pointer to the stored value, or nullptr if the key is absent.
	 * Does not validate the key. Map mutations may invalidate the returned pointer.
	 */
	const V* Find(const K& Key) const
	{
		return InnerMap.Find(Key);
	}

	/**
	 * Removes all entries whose keys are rejected by the validator; values are not validated.
	 * Validators must not mutate this map or recursively invoke Cleanup.
	 * Without a validator, logs an error if notification is requested; otherwise does nothing.
	 *
	 * @param bNotifyChange Whether to broadcast OnChanged once if any entries are removed.
	 */
	void Cleanup(const bool bNotifyChange)
	{
		if (Validate)
		{
			bool bChanged = false;
			for (auto It = InnerMap.CreateIterator(); It; ++It)
			{
				if (!Validate(It.Key()))
				{
					It.RemoveCurrent();
					bChanged = true;
				}
			}

			if (bChanged && bNotifyChange)
			{
				OnChanged.Broadcast();
			}
		}
		else if (bNotifyChange)
		{
			// Internal cleanup calls do not request notification. An explicit cleanup request
			// without a validator is likely unintended, so report it rather than failing silently.
			UE_LOG(LogToroCore, Error, TEXT("Called TAccumulatorMap::Cleanup without a valid validator."));
		}
	}
};
