// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "SaveSystem/SaveGameSerializer.h"
#include "SaveSystem/SaveSerializeInterface.h"
#include "Serialization/NameAsStringProxyArchive.h"
#include "ActorBehaviors/ActorBehaviorBase.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "UObject/SoftObjectPtr.h"
#include "Utilities/ToroLogging.h"
#include "GameFramework/Actor.h"

#define UOBJECT_SERIALIZE_ERROR() \
	SetError(); \
	TORO_LOG_ERROR(TEXT("Serializing object pointers is unsupported. Use Obj->SerializeScriptProperties(Ar)")); \
	return *this;

struct FSaveGameProxyArchive final : FNameAsStringProxyArchive
{
	FSaveGameProxyArchive(FArchive& Ar)
		: FNameAsStringProxyArchive(Ar)
	{
		ArIsSaveGame = true; // Serialize only properties marked by the SaveGame specifier
		ArNoDelta = true; // Save all properties, even if they are unchanged from CDO
	}

	virtual FArchive& operator<<(FSoftObjectPath& Value) override
	{
		FString Path;
		if (IsSaving())
		{
			Path = Value.ToString();
		}

		*this << Path;

		if (IsLoading() && !IsError())
		{
			Value.SetPath(Path);
		}

		return *this;
	}

	virtual FArchive& operator<<(FSoftObjectPtr& Value) override
	{
		FSoftObjectPath Path;
		if (IsSaving())
		{
			Path = Value.ToSoftObjectPath();
		}

		*this << Path;

		if (IsLoading() && !IsError())
		{
			Value = FSoftObjectPtr(Path);
		}

		return *this;
	}

	virtual FArchive& operator<<(FWeakObjectPtr& Obj) override
	{
		UOBJECT_SERIALIZE_ERROR();
	}

	virtual FArchive& operator<<(FObjectPtr& Obj) override
	{
		UOBJECT_SERIALIZE_ERROR();
	}

	virtual FArchive& operator<<(UObject*& Obj) override
	{
		UOBJECT_SERIALIZE_ERROR();
	}
};
#undef UOBJECT_SERIALIZE_ERROR

const FObjectSaveData FObjectSaveData::Empty = {};

bool FObjectSaveData::Serialize(UObject* Target)
{
	Reset();
	if (IsValid(Target))
	{
		FMemoryWriter Writer(Binary, true);
		FSaveGameProxyArchive Ar(Writer);

		ISaveSerializeInterface::PreSerialize(Target, false);
		if (!IsValid(Target))
		{
			TORO_LOG_ERROR(TEXT("Cannot serialize an object invalidated by its pre-serialize event"));
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
			TORO_LOG_ERROR(TEXT("Cannot deserialize an object invalidated by its pre-serialize event"));
			return false;
		}

		Target->SerializeScriptProperties(Ar);
		ISaveSerializeInterface::PostSerialize(Target, true);

		return !Ar.IsError();
	}

	TORO_LOG_ERROR(TEXT("Cannot deserialize an invalid object"));
	return false;
}

const FActorSaveRecord FActorSaveRecord::Empty = {};

bool FActorSaveRecord::Serialize(AActor* Target)
{
	ActorBinary.Reset();
	BehaviorBinary.Reset();
	if (!IsValid(Target))
	{
		TORO_LOG_ERROR(TEXT("Cannot serialize an invalid actor"));
		return false;
	}

	if (!ActorBinary.Serialize(Target))
	{
		TORO_LOG_ERROR(
			TEXT("Failed to serialize actor '%s'"),
			*GetNameSafe(Target)
		);
		return false;
	}

	bool bSuccess = true;
	TArray<UActorBehaviorBase*> Behaviors;
	Target->GetComponents<UActorBehaviorBase>(Behaviors);
	for (UActorBehaviorBase* Behavior : Behaviors)
	{
		if (!IsValid(Behavior) || !Behavior->EligibleForSaving())
		{
			continue;
		}

		const FGuid& Identity = Behavior->GetBehaviorId();
		if (!Identity.IsValid())
		{
			bSuccess = false;
			TORO_LOG_ERROR(
				TEXT("Cannot serialize behavior '%s[%s]' due to invalid guid"),
				*GetNameSafe(Target), *GetNameSafe(Behavior)
			);
			continue;
		}

		if (BehaviorBinary.Contains(Identity))
		{
			bSuccess = false;
			TORO_LOG_ERROR(
				TEXT("Cannot serialize behavior '%s[%s]' due to duplicate guid"),
				*GetNameSafe(Target), *GetNameSafe(Behavior)
			);
			continue;
		}

		if (!BehaviorBinary.Add(Identity).Serialize(Behavior))
		{
			bSuccess = false;
			TORO_LOG_ERROR(
				TEXT("Failed to serialize behavior '%s[%s]'"),
				*GetNameSafe(Target), *GetNameSafe(Behavior)
			);
		}
	}

	return bSuccess;
}

bool FActorSaveRecord::Deserialize(AActor* Target) const
{
	if (!IsValid(Target))
	{
		TORO_LOG_ERROR(TEXT("Cannot deserialize an invalid actor"));
		return false;
	}

	if (!ActorBinary.Deserialize(Target))
	{
		TORO_LOG_ERROR(
			TEXT("Failed to deserialize actor '%s'"),
			*GetNameSafe(Target)
		);
		return false;
	}

	bool bSuccess = true;
	TSet<FGuid> SeenBehaviorIds;
	TArray<UActorBehaviorBase*> Behaviors;
	Target->GetComponents<UActorBehaviorBase>(Behaviors);
	for (UActorBehaviorBase* Behavior : Behaviors)
	{
		if (!IsValid(Behavior) || !Behavior->EligibleForSaving())
		{
			continue;
		}

		const FGuid& Identity = Behavior->GetBehaviorId();
		if (!Identity.IsValid())
		{
			bSuccess = false;
			TORO_LOG_ERROR(
				TEXT("Cannot deserialize behavior '%s[%s]' due to invalid guid"),
				*GetNameSafe(Target), *GetNameSafe(Behavior)
			);
			continue;
		}

		if (SeenBehaviorIds.Contains(Identity))
		{
			bSuccess = false;
			TORO_LOG_ERROR(
				TEXT("Cannot deserialize behavior '%s[%s]' due to duplicate guid"),
				*GetNameSafe(Target), *GetNameSafe(Behavior)
			);
			continue;
		}

		SeenBehaviorIds.Add(Identity);
		const FObjectSaveData* Data = BehaviorBinary.Find(Identity);
		if (Data && !Data->Deserialize(Behavior))
		{
			bSuccess = false;
			TORO_LOG_ERROR(
				TEXT("Failed to deserialize behavior '%s[%s]'"),
				*GetNameSafe(Target), *GetNameSafe(Behavior)
			);
		}
	}

	return bSuccess;
}
