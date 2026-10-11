// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Engine/StaticMeshActor.h"
#include "InstancedStaticMeshActor.generated.h"

UCLASS(meta = (ChildCanTick = true))
class TOROENGINE_API AInstancedStaticMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:

	AInstancedStaticMeshActor(const FObjectInitializer& ObjectInit);

#if WITH_EDITORONLY_DATA
	UPROPERTY(Category = StaticMesh, EditAnywhere, meta = (MakeEditWidget, DisplayPriority = 1))
	TArray<FTransform> Instances = { FTransform::Identity };
#endif

	template<typename T = UInstancedStaticMeshComponent>
	T* GetMeshComponent() const
	{
		return Cast<T>(GetStaticMeshComponent());
	}

#if WITH_EDITOR
	void UpdateInstances() const;
	void CopyInstancesFromComponent();

protected:

	bool bLockInstances = false;

	virtual void OnConstruction(const FTransform& Transform) override;
#endif
};

UCLASS(meta = (ChildCanTick = true))
class TOROENGINE_API AHierarchicalInstancedStaticMeshActor : public AInstancedStaticMeshActor
{
	GENERATED_BODY()

public:

	AHierarchicalInstancedStaticMeshActor(const FObjectInitializer& ObjectInit);
};
