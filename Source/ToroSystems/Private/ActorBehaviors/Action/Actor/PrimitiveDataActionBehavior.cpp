// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Action/Actor/PrimitiveDataActionBehavior.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

void UPrimitiveDataActionBehavior::OnTriggered()
{
	for (const TSoftObjectPtr<AStaticMeshActor>& Target : Targets)
	{
		const AStaticMeshActor* Actor = Target.Get();
		UStaticMeshComponent* Mesh = IsValid(Actor) ? Actor->GetStaticMeshComponent() : nullptr;
		if (!IsValid(Mesh))
		{
			continue;
		}

		for (const TPair<uint8, float>& Data : PrimitiveData)
		{
			if (bOverrideDefault)
			{
				Mesh->SetDefaultCustomPrimitiveDataFloat(Data.Key, Data.Value);
			}
			else
			{
				Mesh->SetCustomPrimitiveDataFloat(Data.Key, Data.Value);
			}
		}
	}
}
