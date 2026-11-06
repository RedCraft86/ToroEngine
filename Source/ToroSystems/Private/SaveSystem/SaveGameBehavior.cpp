// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "SaveSystem/SaveGameBehavior.h"
#include "ActorBehaviors/Misc/IdentityBehavior.h"
#include "SaveSystem/SaveGameSubsystem.h"
#include "Containers/Ticker.h"

USaveGameBehavior::USaveGameBehavior()
{
	IdentityRef = FActorBehaviorRef::MakeFor<UIdentityBehavior>(true);
}

bool USaveGameBehavior::SaveActor()
{
	if (Subsystem.IsValid())
	{
		if (TOptional<FTaggedIdentity> Identity = GetIdentity())
		{
			return Subsystem->GetSlotted().GetActorRecord(Identity.GetValue()).Serialize(GetOwner());
		}
	}

	return false;
}

bool USaveGameBehavior::LoadActor()
{
	if (Subsystem.IsValid())
	{
		if (TOptional<FTaggedIdentity> Identity = GetIdentity())
		{
			return Subsystem->GetSlotted().GetActorRecord(Identity.GetValue()).Deserialize(GetOwner());
		}
	}

	return false;
}

void USaveGameBehavior::OnLoadedDifferentSlot()
{
	LoadActor();
}

TOptional<FTaggedIdentity> USaveGameBehavior::GetIdentity()
{
	const UIdentityBehavior* Identity = IdentityRef.Get<UIdentityBehavior>();
	if (IsValid(Identity) && Identity->Identity.IsValid())
	{
		return Identity->Identity;
	}

	return TOptional<FTaggedIdentity>();
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
		Subsystem->OnSlotRefreshed().AddUObject(this, &USaveGameBehavior::OnLoadedDifferentSlot);
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
		Subsystem->OnSlotRefreshed().RemoveAll(this);
	}
	Subsystem.Reset();
	Super::EndPlay(EndPlayReason);
}
