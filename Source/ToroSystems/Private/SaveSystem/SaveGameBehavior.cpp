// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "SaveSystem/SaveGameBehavior.h"
#include "ActorBehaviors/Misc/IdentityBehavior.h"
#include "SaveSystem/SaveGameSubsystem.h"
#include "GameFramework/Actor.h"
#include "Containers/Ticker.h"

USaveGameBehavior::USaveGameBehavior()
{
	IdentityRef = FActorBehaviorRef::MakeFor<UIdentityBehavior>(true);
}

bool USaveGameBehavior::SaveActor()
{
	const UIdentityBehavior* Behavior = IdentityRef.Get<UIdentityBehavior>();
	if (Subsystem.IsValid() && Subsystem->GetSlotted() && IsValid(GetOwner()) && IsValid(Behavior))
	{
		if (!Behavior->Identity.IsValid())
		{
			return false;
		}

		const FTaggedIdentity Identity = Behavior->Identity;

		FActorSaveRecord Candidate;
		if (!Candidate.Serialize(GetOwner()))
		{
			return false;
		}

		if (Subsystem.IsValid() && Subsystem->GetSlotted())
		{
			if (FActorSaveRecord* Record = Subsystem->GetSlotted()->GetActorRecord(Identity))
			{
				*Record = MoveTemp(Candidate);
				return true;
			}
		}
	}

	return false;
}

bool USaveGameBehavior::LoadActor()
{
	const UIdentityBehavior* Behavior = IdentityRef.Get<UIdentityBehavior>();
	if (Subsystem.IsValid() && Subsystem->GetSlotted() && IsValid(GetOwner()) && IsValid(Behavior))
	{
		if (!Behavior->Identity.IsValid())
		{
			return false;
		}

		if (const FActorSaveRecord* Record = Subsystem->GetSlotted()->FindActorRecord(Behavior->Identity))
		{
			return Record->Deserialize(GetOwner());
		}

		return true;
	}

	return false;
}

void USaveGameBehavior::BeginPlay()
{
	Super::BeginPlay();
	if (!IsValid(GetOwner()))
	{
		return;
	}

	Subsystem = USaveGameSubsystem::Get(this);
	if (Subsystem.IsValid())
	{
		Subsystem->RegisterSaveBehavior(this);
		ExecuteOnGameThread(*FString(TEXT("Load_") + GetOwner()->GetName()), [WeakThis = TWeakObjectPtr(this)]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->LoadActor();
			}
		});
	}
}

void USaveGameBehavior::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Subsystem.IsValid())
	{
		Subsystem->UnregisterSaveBehavior(this);
	}
	Subsystem.Reset();
	Super::EndPlay(EndPlayReason);
}
