// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"
#include "Templates/Function.h"

#define ACCUMULATORS_CHECK_GAME_THREAD() \
	checkf(IsInGameThread(), TEXT("T(Ordered/Unordered)Accumulator should not be accessed outside of game thread."))

/**
 * Shares lifecycle and change notifications for one stored contribution per weak UObject source.
 * Create concrete accumulators through their Make() factories; access and destruction require the game thread.
 * Sources are not retained. Invalid sources are pruned after GC and successful additions/removals.
 * Reads do not prune entries and can include stale contributions until cleanup.
 * Notifications are synchronous; bDiscrete suppresses an operation's notification without deferring it.
 * Remove contributions explicitly when they end before their source's lifetime ends.
 * Values are stored for callers to combine; this class does not calculate an accumulated result.
 * @tparam T Copy-constructible and copy-assignable value type; equality comparison is not required.
 */
template<typename T>
class TOROENGINE_API TAccumulatorBase : public TSharedFromThis<TAccumulatorBase<T>>
{
	FSimpleMulticastDelegate OnChangedEvent;
	FDelegateHandle GCHandle;

protected:

	TAccumulatorBase() = default;

	void Initialize()
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		GCHandle = FCoreUObjectDelegates::GetPostGarbageCollect().AddSP(
			this->AsShared(), &TAccumulatorBase::Cleanup, false
		);
	}

	virtual void AddInternal(const UObject* Source, const T& Value) = 0;
	virtual bool RemoveInternal(const UObject* Source) = 0;
	virtual void ClearInternal(bool bShrink) = 0;
	virtual bool CleanupInternal() = 0;

public:

	virtual ~TAccumulatorBase()
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		FCoreUObjectDelegates::GetPostGarbageCollect().Remove(GCHandle);
		OnChangedEvent.Clear();
		GCHandle.Reset();
	}

	TAccumulatorBase(const TAccumulatorBase&) = delete;
	TAccumulatorBase& operator=(const TAccumulatorBase&) = delete;

	/**
	 * Provides game-thread subscription access to synchronous change notifications.
	 * Accepted additions, including replacements, notify after pruning without comparing values.
	 * Successful removals notify after pruning; Clear and Cleanup notify only if entries were removed.
	 * Discrete operations suppress their notification.
	 */
	[[nodiscard]] FORCEINLINE FSimpleMulticastDelegate::RegistrationType& OnChanged()
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		return OnChangedEvent;
	}

	/** Returns the stored contribution count without pruning invalid sources. */
	[[nodiscard]] virtual int32 Num() const = 0;

	/** Returns whether no contributions are stored, without pruning invalid sources. */
	[[nodiscard]] virtual bool IsEmpty() const = 0;

	/**
	 * Checks whether a contribution is stored for the object without pruning or validating sources.
	 * @param Source Object whose contribution identity is queried.
	 */
	[[nodiscard]] virtual bool Contains(const UObject* Source) const = 0;

	/**
	 * Visits stored contributions synchronously by const reference, without copying or pruning entries.
	 * Ordered accumulators preserve insertion order; unordered accumulators provide no order guarantee.
	 * @param Visitor Called for each visited value on the game thread; return true to continue or false to stop.
	 *        Must not mutate the accumulator or retain references to values after returning.
	 */
	virtual void ForEachValue(TFunctionRef<bool(const T&)> Visitor) const = 0;

	/**
	 * Copies stored contributions without pruning invalid sources.
	 * Ordered accumulators preserve insertion order; unordered accumulators provide no order guarantee.
	 * @param OutValues Receives copies of all stored values, replacing its previous contents.
	 */
	void GetValues(TArray<T>& OutValues) const
	{
		ACCUMULATORS_CHECK_GAME_THREAD();

		OutValues.Reset(Num());
		ForEachValue([&OutValues](const T& Value)
		{
			OutValues.Add(Value);
			return true;
		});
	}

	/**
	 * Adds or replaces a valid source's contribution, then prunes invalid sources.
	 * Replacements preserve position in ordered accumulators. Accepted additions notify even for equal values.
	 * Invalid sources skip storage changes, cleanup, and notification.
	 * @param Source Object identifying the contribution, without being retained.
	 * @param Value Contribution to copy into storage.
	 * @param bDiscrete Suppress this operation's notification, including any pruning it performs.
	 * @return True if accepted; false if the source is invalid.
	 */
	bool Add(const UObject* Source, const T& Value, bool bDiscrete = false)
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		if (!IsValid(Source))
		{
			return false;
		}

		AddInternal(Source, Value);
		CleanupInternal();
		if (!bDiscrete)
		{
			OnChangedEvent.Broadcast();
		}

		return true;
	}

	/**
	 * Removes a valid source's contribution, then prunes invalid sources.
	 * Invalid or absent sources skip cleanup and notification; use Cleanup to prune stale entries.
	 * @param Source Object identifying the contribution to remove.
	 * @param bDiscrete Suppress this operation's notification, including any pruning it performs.
	 * @return True if removed; false if the source is invalid or absent.
	 */
	bool Remove(const UObject* Source, bool bDiscrete = false)
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		if (!IsValid(Source))
		{
			return false;
		}

		if (RemoveInternal(Source))
		{
			CleanupInternal();
			if (!bDiscrete)
			{
				OnChangedEvent.Broadcast();
			}

			return true;
		}

		return false;
	}

	/**
	 * Removes all contributions, notifying once only if entries existed and notification is enabled.
	 * @param bShrink Release reserved storage even if already empty; otherwise retain it for reuse.
	 * @param bDiscrete Suppress this operation's change notification.
	 */
	void Clear(bool bShrink, bool bDiscrete = false)
	{
		ACCUMULATORS_CHECK_GAME_THREAD();

		const bool bAlreadyEmpty = IsEmpty();
		ClearInternal(bShrink);

		if (!bDiscrete && !bAlreadyEmpty)
		{
			OnChangedEvent.Broadcast();
		}
	}

	/**
	 * Prunes invalid sources, notifying once if entries were removed and notification is enabled.
	 * Automatic post-GC cleanup calls this with notifications enabled.
	 * @param bDiscrete Suppress this operation's change notification.
	 */
	void Cleanup(bool bDiscrete = false)
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		if (CleanupInternal() && !bDiscrete)
		{
			OnChangedEvent.Broadcast();
		}
	}
};

/**
 * Stores one contribution per weak UObject source in insertion order.
 * Replacing a value preserves its position; removal and cleanup preserve surviving order.
 * Removing and re-adding a source places its contribution at the end.
 * Create through Make(); access and destruction require the game thread.
 * @tparam T Copy-constructible and copy-assignable contribution type.
 */
template<typename T>
class TOROENGINE_API TOrderedAccumulator final : public TAccumulatorBase<T>
{
	TArray<TPair<TWeakObjectPtr<const UObject>, T>> InnerArray;

	TOrderedAccumulator() = default;

	virtual void AddInternal(const UObject* Source, const T& Value) override
	{
		if (TPair<TWeakObjectPtr<const UObject>, T>* Found = InnerArray.FindByPredicate([Source](const auto& Pair)
		{
			return Pair.Key == Source;
		}))
		{
			Found->Value = Value;
		}
		else
		{
			InnerArray.Emplace(Source, Value);
		}
	}

	virtual bool RemoveInternal(const UObject* Source) override
	{
		return InnerArray.RemoveAll([Source](const TPair<TWeakObjectPtr<const UObject>, T>& Pair) -> bool
		{
			return Pair.Key == Source;
		}) != 0;
	}

	virtual void ClearInternal(bool bShrink) override
	{
		if (bShrink)
		{
			InnerArray.Empty();
		}
		else
		{
			InnerArray.Reset();
		}
	}

	virtual bool CleanupInternal() override
	{
		return InnerArray.RemoveAll([](const TPair<TWeakObjectPtr<const UObject>, T>& Pair) -> bool
		{
			return !Pair.Key.IsValid();
		}) != 0;
	}

public:

	/** Creates an empty, shared-owned ordered accumulator with automatic GC cleanup on the game thread. */
	[[nodiscard]] static TSharedRef<TOrderedAccumulator<T>> Make()
	{
		TSharedRef<TOrderedAccumulator<T>> NewAccumulator = MakeShareable(new TOrderedAccumulator<T>());
		NewAccumulator->Initialize();
		return NewAccumulator;
	}

	[[nodiscard]] FORCEINLINE const TArray<TPair<TWeakObjectPtr<const UObject>, T>>& operator*() const
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		return InnerArray;
	}

	[[nodiscard]] FORCEINLINE virtual int32 Num() const override
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		return InnerArray.Num();
	}

	[[nodiscard]] FORCEINLINE virtual bool IsEmpty() const override
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		return InnerArray.IsEmpty();
	}

	[[nodiscard]] virtual bool Contains(const UObject* Source) const override
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		return InnerArray.ContainsByPredicate([Source](const TPair<TWeakObjectPtr<const UObject>, T>& Pair) -> bool
		{
			return Pair.Key == Source;
		});
	}

	virtual void ForEachValue(TFunctionRef<bool(const T&)> Visitor) const override
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		for (const TPair<TWeakObjectPtr<const UObject>, T>& Pair : InnerArray)
		{
			if (!Visitor(Pair.Value))
			{
				break;
			}
		}
	}

	/**
	 * If this accumulator is non-empty, get the first entry of it.
	 * @return Pointer to the first entry or nullptr if the accumulator is empty.
	 */
	const T* GetFirst() const
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		if (!IsEmpty())
		{
			return &InnerArray[0];
		}

		return nullptr;
	}

	/**
	 * If this accumulator is non-empty, get the last entry of it.
	 * @return Pointer to the last entry or nullptr if the accumulator is empty.
	 */
	const T* GetLast() const
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		if (!IsEmpty())
		{
			return &InnerArray[Num() - 1];
		}

		return nullptr;
	}
};

/**
 * Stores one contribution per weak UObject source without an iteration-order guarantee.
 * Adding an existing source replaces its value. Invalid sources are pruned automatically after GC.
 * Create through Make(); access and destruction require the game thread.
 * @tparam T Copy-constructible and copy-assignable contribution type.
 */
template<typename T>
class TOROENGINE_API TUnorderedAccumulator final : public TAccumulatorBase<T>
{
	TMap<TWeakObjectPtr<const UObject>, T> InnerMap;

	TUnorderedAccumulator() = default;

	virtual void AddInternal(const UObject* Source, const T& Value) override
	{
		InnerMap.Add(Source, Value);
	}

	virtual bool RemoveInternal(const UObject* Source) override
	{
		return InnerMap.Remove(Source) != 0;
	}

	virtual void ClearInternal(bool bShrink) override
	{
		if (bShrink)
		{
			InnerMap.Empty();
		}
		else
		{
			InnerMap.Reset();
		}
	}

	virtual bool CleanupInternal() override
	{
		bool bChanged = false;
		for (auto It = InnerMap.CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid())
			{
				It.RemoveCurrent();
				bChanged = true;
			}
		}
		return bChanged;
	}

public:

	/** Creates an empty, shared-owned unordered accumulator with automatic GC cleanup on the game thread. */
	[[nodiscard]] static TSharedRef<TUnorderedAccumulator<T>> Make()
	{
		TSharedRef<TUnorderedAccumulator<T>> NewAccumulator = MakeShareable(new TUnorderedAccumulator<T>());
		NewAccumulator->Initialize();
		return NewAccumulator;
	}

	[[nodiscard]] FORCEINLINE const TMap<TWeakObjectPtr<const UObject>, T>& operator*() const
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		return InnerMap;
	}

	[[nodiscard]] FORCEINLINE virtual int32 Num() const override
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		return InnerMap.Num();
	}

	[[nodiscard]] FORCEINLINE virtual bool IsEmpty() const override
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		return InnerMap.IsEmpty();
	}

	[[nodiscard]] FORCEINLINE virtual bool Contains(const UObject* Source) const override
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		return InnerMap.Contains(Source);
	}

	virtual void ForEachValue(TFunctionRef<bool(const T&)> Visitor) const override
	{
		ACCUMULATORS_CHECK_GAME_THREAD();
		for (const TPair<TWeakObjectPtr<const UObject>, T>& Pair : InnerMap)
		{
			if (!Visitor(Pair.Value))
			{
				break;
			}
		}
	}
};

#undef ACCUMULATORS_CHECK_GAME_THREAD
