// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Misc/Guid.h"
#include "CoreMinimal.h"
#include "Serialization/Archive.h"

class AActor;

struct TOROSYSTEMS_API FObjectSaveData final
{
	TArray<uint8> Binary;

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FObjectSaveData& Data)
	{
		return Ar << Data.Binary;
	}

	void Reset() { Binary.Reset(); }
	[[nodiscard]] bool Serialize(UObject* Target);
	[[nodiscard]] bool Deserialize(UObject* Target) const;
};

struct TOROSYSTEMS_API FActorSaveRecord final
{
	FObjectSaveData ActorData;
	TMap<FGuid, FObjectSaveData> ComponentData;

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FActorSaveRecord& Record)
	{
		return Ar << Record.ActorData << Record.ComponentData;
	}

	[[nodiscard]] bool Serialize(AActor* Target);
	[[nodiscard]] bool Deserialize(AActor* Target) const;
};
