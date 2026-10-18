// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Components/LazyRenderComponent.h"
// Fix: Actor lookup and visibility calls require the complete actor definition.
#include "GameFramework/Actor.h"

namespace
{
	/**
	 * Finds the first active lazy-render component in the actor's component order.
	 * @param Actor Actor to search; an invalid actor is ignored.
	 * @return Active component, or nullptr when no match is available.
	 */
	ULazyRenderComponent* FindComponentOnActor(const AActor* Actor)
	{
		if (IsValid(Actor))
		{
			TArray<ULazyRenderComponent*> Components;
			Actor->GetComponents<ULazyRenderComponent>(Components);
			for (ULazyRenderComponent* Component : Components)
			{
				if (Component && Component->IsActive())
				{
					return Component;
				}
			}
		}

		return nullptr;
	}
}

ULazyRenderComponent::ULazyRenderComponent()
	: Requests(TAccumulatorSet<TWeakObjectPtr<const UObject>>::Create(
		[](const TWeakObjectPtr<const UObject>& Object)
		{
			return Object.IsValid();
		}
	))
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;

#if WITH_EDITORONLY_DATA
	MaxInstancesPerActor = 1;
#endif
}

void ULazyRenderComponent::AddRenderRequest(const AActor* Target, const UObject* InRequester)
{
	const ULazyRenderComponent* Component = FindComponentOnActor(Target);
	if (Component && IsValid(InRequester))
	{
		Component->AddRequest(InRequester);
	}
}

void ULazyRenderComponent::RemoveRenderRequest(const AActor* Target, const UObject* InRequester)
{
	const ULazyRenderComponent* Component = FindComponentOnActor(Target);
	if (Component && IsValid(InRequester))
	{
		Component->RemoveRequest(InRequester);
	}
}

void ULazyRenderComponent::AddRequest(const UObject* InRequester) const
{
	if (InRequester)
	{
		Requests->Add(InRequester);
	}
}

void ULazyRenderComponent::RemoveRequest(const UObject* InRequester) const
{
	if (InRequester)
	{
		Requests->Remove(InRequester);
	}
}

void ULazyRenderComponent::OnRequestChanged() const
{
	if (AActor* Owner = GetOwner())
	{
		Owner->SetActorHiddenInGame(IsActive() && Requests->IsEmpty());
	}
}

void ULazyRenderComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
	Requests->OnChanged.RemoveAll(this);
	Requests->OnChanged.AddUObject(this, &ULazyRenderComponent::OnRequestChanged);
	OnRequestChanged();
}

void ULazyRenderComponent::Deactivate()
{
	Super::Deactivate();
	Requests->OnChanged.RemoveAll(this);
	OnRequestChanged();
}
