// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Action/ActorActionBehaviors.h"
#include "Interfaces/ActivatableObject.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "CoreTypes.h"

void UVisibilityActionBehavior::OnTriggered()
{
	for (const TSoftObjectPtr<AActor>& SoftPtr : Targets)
	{
		if (AActor* ActorPtr = SoftPtr.Get())
		{
			ActorPtr->SetActorHiddenInGame(Type == EToroActionApplyType::Set
				? !bVisible : !ActorPtr->IsHidden());
		}
	}
}

void UCollisionActionBehavior::OnTriggered()
{
	for (const TSoftObjectPtr<AActor>& SoftPtr : Targets)
	{
		if (AActor* ActorPtr = SoftPtr.Get())
		{
			ActorPtr->SetActorEnableCollision(Type == EToroActionApplyType::Set
				? bCollisionEnabled : !ActorPtr->GetActorEnableCollision());
		}
	}
}

void UActorEnabledActionBehavior::OnTriggered()
{
	for (const TSoftObjectPtr<AActor>& SoftPtr : Targets)
	{
		if (AActor* ActorPtr = SoftPtr.Get())
		{
			IActivatableObject::SetActiveState(ActorPtr, Type == EToroActionApplyType::Set
				? bEnabled : !IActivatableObject::GetActiveState(ActorPtr));
		}
	}
}

void UActorTagsActionBehavior::OnTriggered()
{
	for (const TSoftObjectPtr<AActor>& SoftPtr : Targets)
	{
		if (AActor* ActorPtr = SoftPtr.Get())
		{
			for (const FName& Tag : AddTags)
			{
				ActorPtr->Tags.AddUnique(Tag);
			}
			for (const FName& Tag : RemoveTags)
			{
				ActorPtr->Tags.Remove(Tag);
			}
		}
	}
}

void UPrimitiveDataActionBehavior::OnTriggered()
{
	for (const TSoftObjectPtr<AStaticMeshActor>& SoftPtr : Targets)
	{
		const AStaticMeshActor* ActorPtr = SoftPtr.Get();
		if (UStaticMeshComponent* MeshComp = ActorPtr ? ActorPtr->GetStaticMeshComponent() : nullptr)
		{
			for (const TPair<uint8, float>& Data : PrimitiveData)
			{
				if (bOverrideDefault)
				{
					MeshComp->SetDefaultCustomPrimitiveDataFloat(Data.Key, Data.Value);
				}
				else
				{
					MeshComp->SetCustomPrimitiveDataFloat(Data.Key, Data.Value);
				}
			}
		}
	}
}
