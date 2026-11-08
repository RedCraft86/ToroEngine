// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"

#define REQUEST_SET_CHECK_GAME_THREAD() \
	checkf(IsInGameThread(), TEXT("FRequestSet should not be accessed outside of game thread."))

/**
 * Tracks one request per UObject without retaining its requester.
 * Create through Make(); access and destruction must occur on the game thread.
 * Invalid weak references are pruned after garbage collection and successful additions/removals.
 * Reads do not prune entries, so stored counts may include invalid requesters until cleanup.
 * Changes notify listeners synchronously unless suppressed by bDiscrete; suppressed notifications are not deferred.
 * Remove requests explicitly when they end, even if their requester remains alive, such as a pooled widget.
 */
class FRequestSet final : public TSharedFromThis<FRequestSet>
{
	TSet<TWeakObjectPtr<const UObject>> InnerSet;
	FSimpleMulticastDelegate OnChangedEvent;
	FDelegateHandle GCHandle;

	FRequestSet() = default;

	void Initialize()
	{
		REQUEST_SET_CHECK_GAME_THREAD();
		GCHandle = FCoreUObjectDelegates::GetPostGarbageCollect().AddSP(
			AsShared(), &FRequestSet::Cleanup, false
		);
	}

	bool CleanupInternal()
	{
		bool bChanged = false;
		for (auto It = InnerSet.CreateIterator(); It; ++It)
		{
			if (!It->IsValid())
			{
				It.RemoveCurrent();
				bChanged = true;
			}
		}
		return bChanged;
	}

public:

	FRequestSet(const FRequestSet&) = delete;
	FRequestSet& operator=(const FRequestSet&) = delete;

	~FRequestSet()
	{
		REQUEST_SET_CHECK_GAME_THREAD();
		FCoreUObjectDelegates::GetPostGarbageCollect().Remove(GCHandle);
		OnChangedEvent.Clear();
		GCHandle.Reset();
	}

	/** Creates an empty, shared-owned request set with automatic GC cleanup on the game thread. */
	[[nodiscard]] static TSharedRef<FRequestSet> Make()
	{
		TSharedRef<FRequestSet> NewSet = MakeShareable(new FRequestSet());
		NewSet->Initialize();
		return NewSet;
	}

	[[nodiscard]] FORCEINLINE const TSet<TWeakObjectPtr<const UObject>>& operator*() const
	{
		REQUEST_SET_CHECK_GAME_THREAD();
		return InnerSet;
	}

	/**
	 * Provides subscription access to synchronous change notifications.
	 * Successful additions/removals notify after pruning invalid requesters. Clear and Cleanup
	 * notify only when entries are removed. Discrete operations suppress their notification.
	 * Register and unregister listeners on the game thread.
	 */
	[[nodiscard]] FORCEINLINE FSimpleMulticastDelegate::RegistrationType& OnChanged()
	{
		REQUEST_SET_CHECK_GAME_THREAD();
		return OnChangedEvent;
	}

	/** Returns the stored request count without pruning invalid requesters. */
	[[nodiscard]] FORCEINLINE int32 Num() const
	{
		REQUEST_SET_CHECK_GAME_THREAD();
		return InnerSet.Num();
	}

	/** Returns whether no requests are stored, without pruning invalid requesters. */
	[[nodiscard]] FORCEINLINE bool IsEmpty() const
	{
		REQUEST_SET_CHECK_GAME_THREAD();
		return InnerSet.IsEmpty();
	}

	/**
	 * Checks whether a request is stored for the object without pruning or validating requesters.
	 * @param Requester Object whose request identity is queried.
	 */
	[[nodiscard]] FORCEINLINE bool Contains(const UObject* Requester) const
	{
		REQUEST_SET_CHECK_GAME_THREAD();
		return InnerSet.Contains(Requester);
	}

	/**
	 * Adds one request for a valid object, then prunes invalid requesters.
	 * Invalid or duplicate requests skip cleanup and notification.
	 * @param Requester Object identifying the request; repeated requests do not stack or retain the object.
	 * @param bDiscrete Suppress this operation's change notification, including any pruning it performs.
	 * @return True if added; false if the requester is invalid or already present.
	 */
	bool Add(const UObject* Requester, bool bDiscrete = false)
	{
		REQUEST_SET_CHECK_GAME_THREAD();
		if (!IsValid(Requester) || Contains(Requester))
		{
			return false;
		}

		InnerSet.Add(Requester);
		CleanupInternal();
		if (!bDiscrete)
		{
			OnChangedEvent.Broadcast();
		}

		return true;
	}

	/**
	 * Removes a valid object's request, then prunes invalid requesters.
	 * Invalid or absent requesters skip cleanup and notification; use Cleanup to prune stale requests.
	 * @param Requester Object identifying the request to remove.
	 * @param bDiscrete Suppress this operation's change notification, including any pruning it performs.
	 * @return True if removed; false if the requester is invalid or absent.
	 */
	bool Remove(const UObject* Requester, bool bDiscrete = false)
	{
		REQUEST_SET_CHECK_GAME_THREAD();
		if (!IsValid(Requester) || !Contains(Requester))
		{
			return false;
		}

		InnerSet.Remove(Requester);
		CleanupInternal();
		if (!bDiscrete)
		{
			OnChangedEvent.Broadcast();
		}

		return true;
	}

	/**
	 * Removes all stored requests, notifying once only if entries existed and notification is enabled.
	 * @param bShrink Release reserved storage, even if already empty; otherwise retain it for reuse.
	 * @param bDiscrete Suppress this operation's change notification.
	 */
	void Clear(bool bShrink, bool bDiscrete = false)
	{
		REQUEST_SET_CHECK_GAME_THREAD();
		const bool bAlreadyEmpty = IsEmpty();
		if (bShrink)
		{
			InnerSet.Empty();
		}
		else
		{
			InnerSet.Reset();
		}

		if (!bDiscrete && !bAlreadyEmpty)
		{
			OnChangedEvent.Broadcast();
		}
	}

	/**
	 * Prunes invalid requesters, notifying once if entries were removed and notification is enabled.
	 * Automatic post-GC cleanup calls this with notifications enabled.
	 * @param bDiscrete Suppress this operation's change notification.
	 */
	void Cleanup(bool bDiscrete = false)
	{
		REQUEST_SET_CHECK_GAME_THREAD();
		if (CleanupInternal() && !bDiscrete)
		{
			OnChangedEvent.Broadcast();
		}
	}
};

#undef REQUEST_SET_CHECK_GAME_THREAD
