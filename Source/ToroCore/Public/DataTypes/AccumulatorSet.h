// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToroCore.h"
#include "UObject/UObjectGlobals.h"
#include "Traits/UObjectPtrTraits.h"

/**
 * Stores unique entries with optional validation and change notifications.
 * Successful additions and removals also clean up entries rejected by the validator.
 * Accessors and iteration expose stored entries without performing validation.
 *
 * This container does not keep UObject entries alive through garbage collection.
 * Use TWeakObjectPtr with a validity validator for entries that should expire after GC;
 * raw UObject pointers and TObjectPtr entries here do not provide GC ownership.
 *
 * @tparam T Entry type supporting the hashing and equality operations required by TSet.
 */
template <typename T>
class TAccumulatorSet final : public TSharedFromThis<TAccumulatorSet<T>>
{
	/** Unique stored entries, which may include entries awaiting validation. */
	TSet<T> InnerSet;

	/** Optional predicate returning true for entries that may be added or retained. */
	TFunction<bool(const T&)> Validate;

	/** Constructs an empty set and takes ownership of the supplied validator. */
	TAccumulatorSet(TFunction<bool(const T&)> Validator)
		: InnerSet(), Validate(MoveTemp(Validator))
	{}

public:

	/**
	 * Broadcasts once after a successful addition or removal and its associated cleanup.
	 * Cleanup broadcasts only when it removes entries and notification is requested,
	 * including automatic post-GC cleanup.
	 */
	TMulticastDelegate<void()> OnChanged;

	/** Constructs an empty set without validation or automatic post-GC cleanup. */
	TAccumulatorSet()
		: InnerSet(), Validate(nullptr)
	{}

	TAccumulatorSet(const TAccumulatorSet&) = delete;
	TAccumulatorSet& operator=(const TAccumulatorSet&) = delete;

	~TAccumulatorSet()
	{
		FCoreUObjectDelegates::GetPostGarbageCollect().RemoveAll(this);
	}

	/** Creates a shared empty set without validation or automatic post-GC cleanup. */
	[[nodiscard]] static TSharedRef<TAccumulatorSet<T>> Create()
	{
		return MakeShareable(new TAccumulatorSet<T>());
	}

	/**
	 * Creates a shared empty set with optional validation.
	 * A valid validator enables automatic notifying post-GC cleanup when
	 * TIsUObjectType<T>::Value is true. An empty validator behaves like Create().
	 *
	 * @param Validator Predicate returning true for entries that may be added or retained.
	 * Validators must not mutate this accumulator or recursively invoke cleanup,
	 * because they can run while Cleanup is iterating the live set.
	 */
	[[nodiscard]] static TSharedRef<TAccumulatorSet<T>> Create(TFunction<bool(const T&)> Validator)
	{
		if (!Validator)
		{
			return Create();
		}

		TSharedRef<TAccumulatorSet<T>> Instance = MakeShareable(new TAccumulatorSet<T>(MoveTemp(Validator)));
		if constexpr (TIsUObjectType<T>::Value)
		{
			FCoreUObjectDelegates::GetPostGarbageCollect().AddSP(Instance, &TAccumulatorSet::Cleanup, true);
		}

		return Instance;
	}

	/** Returns a read-only reference to the live set without validating its entries. */
	[[nodiscard]] FORCEINLINE const TSet<T>& GetInnerSet() const
	{
		return InnerSet;
	}

	/** Returns the number of stored entries without performing validation. */
	[[nodiscard]] FORCEINLINE int32 Num() const
	{
		return InnerSet.Num();
	}

	/** Returns whether there are no stored entries, without performing validation. */
	[[nodiscard]] FORCEINLINE bool IsEmpty() const
	{
		return InnerSet.IsEmpty();
	}

	/**
	 * Visits stored entries without validating them, stopping when the callback returns false.
	 * A null callback logs an error and returns without visiting entries.
	 * Callbacks must not mutate this accumulator, including through Cleanup,
	 * because iteration uses the live set and mutations can invalidate its iterator.
	 *
	 * @param Func Callback returning true to continue iteration or false to stop.
	 */
	void ForEach(const TFunction<bool(const T&)>& Func) const
	{
		if (!Func)
		{
			UE_LOG(LogToroCore, Error, TEXT("Called TAccumulatorSet::ForEach without a valid loop function."));
			return;
		}

		for (const T& Value : InnerSet)
		{
			if (!Func(Value))
			{
				break;
			}
		}
	}

	/**
	 * Adds a value if accepted by the optional validator and not already present.
	 * A successful addition cleans up invalid entries, then broadcasts OnChanged once.
	 * Rejected or duplicate values cause no cleanup or notification.
	 */
	void Add(const T& Value)
	{
		if ((Validate && !Validate(Value)) || InnerSet.Contains(Value))
		{
			return;
		}

		InnerSet.Add(Value);

		Cleanup(false);
		OnChanged.Broadcast();
	}

	/**
	 * Removes a stored value, cleans up invalid entries, then broadcasts OnChanged once.
	 * If the value is absent, performs no cleanup or notification.
	 */
	void Remove(const T& Value)
	{
		if (InnerSet.Remove(Value) > 0)
		{
			Cleanup(false);
			OnChanged.Broadcast();
		}
	}

	/**
	 * Removes all stored entries rejected by the validator.
	 * Validators must not mutate this accumulator or recursively invoke cleanup.
	 * Without a validator, logs an error if notification is requested; otherwise does nothing.
	 *
	 * @param bNotifyChange Whether to broadcast OnChanged once if any entries are removed.
	 */
	void Cleanup(const bool bNotifyChange)
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

			if (bChanged && bNotifyChange)
			{
				OnChanged.Broadcast();
			}
		}
		else if (bNotifyChange)
		{
			// Internal cleanup calls do not request notification. An explicit cleanup request
			// without a validator is likely unintended, so report it rather than failing silently.
			UE_LOG(LogToroCore, Error, TEXT("Called TAccumulatorSet::Cleanup without a valid validator."));
		}
	}
};
