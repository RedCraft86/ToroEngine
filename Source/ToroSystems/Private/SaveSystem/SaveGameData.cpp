// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "SaveSystem/SaveGameData.h"
#include "SaveSystem/SaveGameArchive.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "SaveSystem/SaveSerializeInterface.h"
#include "ActorBehaviors/ActorBehaviorBase.h"
#include "Utilities/ToroLogging.h"
#include "GameFramework/Actor.h"

bool FObjectSaveData::Serialize(UObject* Target)
{
	Binary.Empty();
	if (IsValid(Target))
	{
		FMemoryWriter Writer(Binary, true);
		FSaveGameProxyArchive Ar(Writer);

		ISaveSerializeInterface::PreSerialize(Target, false);
		if (!IsValid(Target))
		{
			TORO_LOG_ERROR(TEXT("Cannot serialize an object invalidated by its pre-serialization notification"));
			return false;
		}

		Target->SerializeScriptProperties(Ar);
		ISaveSerializeInterface::PostSerialize(Target, false);

		return !Ar.IsError();
	}

	TORO_LOG_ERROR(TEXT("Cannot serialize an invalid object"));
	return false;
}

bool FObjectSaveData::Deserialize(UObject* Target) const
{
	if (IsValid(Target))
	{
		FMemoryReader Reader(Binary, true);
		FSaveGameProxyArchive Ar(Reader);

		ISaveSerializeInterface::PreSerialize(Target, true);
		if (!IsValid(Target))
		{
			TORO_LOG_ERROR(TEXT("Cannot deserialize an object invalidated by its pre-serialization notification"));
			return false;
		}

		Target->SerializeScriptProperties(Ar);
		ISaveSerializeInterface::PostSerialize(Target, true);

		return !Ar.IsError();
	}

	TORO_LOG_ERROR(TEXT("Cannot deserialize an invalid object"));
	return false;
}

bool FActorSaveRecord::Serialize(AActor* Target)
{
	ActorData.Reset();
	ComponentData.Reset();
	if (IsValid(Target))
	{
		if (ActorData.Serialize(Target))
		{
			TArray<UActorBehaviorBase*> Components;
			Target->GetComponents<UActorBehaviorBase>(Components);
			for (UActorBehaviorBase* Component : Components)
			{
				if (!IsValid(Component) || !Component->EligibleForSaving())
				{
					continue;
				}

				const FGuid& BehaviorId = Component->GetBehaviorId();
				if (!BehaviorId.IsValid())
				{
					TORO_LOG_ERROR(
						TEXT("Cannot serialize component '%s': behavior GUID is invalid"),
						*GetNameSafe(Component)
					);
					return false;
				}

				if (ComponentData.Contains(BehaviorId))
				{
					TORO_LOG_ERROR(TEXT("Cannot serialize component '%s': duplicate behavior GUID '%s'"),
						*GetNameSafe(Component), *BehaviorId.ToString());
					return false;
				}

				FObjectSaveData& Data = ComponentData.Add(BehaviorId);
				if (!Data.Serialize(Component))
				{
					TORO_LOG_ERROR(TEXT("Failed to serialize component '%s'"), *GetNameSafe(Component));
					return false;
				}
			}

			return true;
		}

		TORO_LOG_ERROR(TEXT("Failed to serialize actor '%s'"), *GetNameSafe(Target));
		return false;
	}

	TORO_LOG_ERROR(TEXT("Cannot serialize an invalid actor"));
	return false;
}

bool FActorSaveRecord::Deserialize(AActor* Target) const
{
	if (IsValid(Target))
	{
		if (ActorData.Deserialize(Target))
		{
			TSet<FGuid> SeenBehaviorIds;
			TArray<UActorBehaviorBase*> Components;
			Target->GetComponents<UActorBehaviorBase>(Components);
			for (UActorBehaviorBase* Component : Components)
			{
				if (!IsValid(Component) || !Component->EligibleForSaving())
				{
					continue;
				}

				const FGuid BehaviorId = Component->GetBehaviorId();
				if (!BehaviorId.IsValid())
				{
					TORO_LOG_ERROR(TEXT("Cannot deserialize component '%s': behavior GUID is invalid"),
						*GetNameSafe(Component));
					return false;
				}

				if (SeenBehaviorIds.Contains(BehaviorId))
				{
					TORO_LOG_ERROR(TEXT("Cannot deserialize component '%s': duplicate behavior GUID '%s'"),
						*GetNameSafe(Component), *BehaviorId.ToString());
					return false;
				}
				SeenBehaviorIds.Add(BehaviorId);

				if (const FObjectSaveData* Data = ComponentData.Find(BehaviorId))
				{
					if (!Data->Deserialize(Component))
					{
						TORO_LOG_ERROR(TEXT("Failed to deserialize component '%s'"), *GetNameSafe(Component));
						return false;
					}
				}
			}

			return true;
		}

		TORO_LOG_ERROR(TEXT("Failed to deserialize actor '%s'"), *GetNameSafe(Target));
		return false;
	}

	TORO_LOG_ERROR(TEXT("Cannot deserialize an invalid actor"));
	return false;
}
