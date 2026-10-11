// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "DataTypes/CachedGetter.h"
#include "DataTypes/ToroMeshPreset.h"
#include "Subsystems/EditorActorSubsystem.h"

class USplineMeshComponent;
class UProceduralMeshComponent;

class TOROENGINEED_API FToroActorEditorUtils final
{
public:

	static inline TCachedGetter<UEditorActorSubsystem> Subsystem{[] {
		return GEditor ? GEditor->GetEditorSubsystem<UEditorActorSubsystem>() : nullptr;
	}};

	template <typename T = AStaticMeshActor>
	static T* SpawnMeshActor(const FTransform& Transform, const AActor* Owner, const FString& Name)
	{
		static_assert(TIsDerivedFrom<T, AActor>::Value, "T must derive from AActor");
		return Cast<T>(SpawnMeshActor(T::StaticClass(), Transform, Owner, Name));
	}

	static bool DestroyActors(const TArray<AActor*>& Actors);
	static TArray<AActor*> GetSelectedActors(bool bWarnEmpty);

	static TArray<AActor*> SplitActors(const TArray<AActor*>& Actors, bool bDestroySources);
	static TArray<AActor*> InstanceActors(const TArray<AActor*>& Actors, bool bUseHISM, bool bDestroySources);
	static bool LayoutActors(const TArray<AActor*>& Actors, uint8 MaxColumns, float YawOffset, const FVector2D& XYOffset);

private:

	static TArray<AActor*> BakeMeshComponent(const UMeshComponent* Src);
	static TArray<AActor*> BakeInstancedMesh(const UInstancedStaticMeshComponent* Src);
	static AActor* BakeSplineMesh(const USplineMeshComponent* Src);
	static AActor* BakeStaticMesh(const UStaticMeshComponent* Src);
	static AActor* BakeSkeletalMesh(const USkeletalMeshComponent* Src);
	static AActor* BakeProceduralMesh(const UProceduralMeshComponent* Src);

	static bool ExtractMeshComponent(const UMeshComponent* Src, TMap<FToroBaseMeshPreset, TArray<FTransform>>& Instances);
	static bool ExtractInstancedMesh(const UInstancedStaticMeshComponent* Src, TMap<FToroBaseMeshPreset, TArray<FTransform>>& Instances);
	static bool ExtractStaticMesh(const UStaticMeshComponent* Src, TMap<FToroBaseMeshPreset, TArray<FTransform>>& Instances);

	static AActor* SpawnMeshActor(const TSubclassOf<AActor>& Class, const FTransform& Transform, const AActor* Owner, const FString& Name);
};
