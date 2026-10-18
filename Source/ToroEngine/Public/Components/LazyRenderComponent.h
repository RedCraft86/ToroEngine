// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "DataTypes/AccumulatorSet.h"
#include "Components/ToroActorComponent.h"
#include "LazyRenderComponent.generated.h"

/**
 * Controls the owner's hidden-in-game state through unique, weakly held render requests.
 * While active, hides the owner when no requests remain and unhides it when requests exist.
 * Deactivation unhides the owner without clearing requests; activation reapplies the retained set.
 * Visibility updates overwrite the owner's hidden state rather than restoring a previous value.
 * Only one instance of this component should exist on an actor.
 */
UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Optimization), meta = (BlueprintSpawnableComponent))
class ULazyRenderComponent final : public UToroActorComponent
{
	GENERATED_BODY()

public:

	ULazyRenderComponent();

	/**
	 * Adds a render request to the first active lazy-render component on the target actor.
	 * Does nothing for an invalid target/requester or when no active component is found.
	 * @param Target Actor whose active lazy-render component should receive the request.
	 * @param InRequester Request identity, stored weakly; repeated additions do not increase a count.
	 */
	UFUNCTION(BlueprintCallable, Category = LazyRendering, meta = (DefaultToSelf = Target))
	static TOROENGINE_API void AddRenderRequest(const AActor* Target, const UObject* InRequester);

	/**
	 * Removes a request from the first active lazy-render component on the target actor.
	 * Does nothing for an invalid target/requester, an absent request, or no active component.
	 * @param Target Actor whose active lazy-render component should release the request.
	 * @param InRequester Request identity to remove; one removal undoes repeated additions of that identity.
	 */
	UFUNCTION(BlueprintCallable, Category = LazyRendering, meta = (DefaultToSelf = Target))
	static TOROENGINE_API void RemoveRenderRequest(const AActor* Target, const UObject* InRequester);

	/**
	 * Adds a unique render request without retaining the requester through garbage collection.
	 * Null or invalid requesters and duplicate additions are ignored. Requests can be stored while
	 * inactive; visibility is reapplied on activation. Expired entries are cleaned after garbage collection.
	 * @param InRequester Object identifying this request; requests are not reference-counted.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = LazyRendering)
	TOROENGINE_API void AddRequest(const UObject* InRequester) const;

	/**
	 * Removes a stored request, including while the component is inactive.
	 * A null requester or absent entry does nothing. Removing the last request while active hides the owner.
	 * @param InRequester Object identifying the request to remove; one removal is sufficient for that identity.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = LazyRendering)
	TOROENGINE_API void RemoveRequest(const UObject* InRequester) const;

protected:

	TSharedRef<TAccumulatorSet<TWeakObjectPtr<const UObject>>> Requests;

	void OnRequestChanged() const;
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;
};
