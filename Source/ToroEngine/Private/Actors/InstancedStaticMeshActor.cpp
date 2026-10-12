// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Actors/InstancedStaticMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Templates/UnrealTemplate.h"

AInstancedStaticMeshActor::AInstancedStaticMeshActor(const FObjectInitializer& ObjectInit)
	: Super(ObjectInit.SetDefaultSubobjectClass<UInstancedStaticMeshComponent>(StaticMeshComponentName))
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bCanBeInCluster = GetClass() == AInstancedStaticMeshActor::StaticClass();

	if (!IsRunningCommandlet() && !IsRunningDedicatedServer())
	{
		if (UStaticMeshComponent* MeshComp = GetMeshComponent())
		{
			static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(
				TEXT("/Engine/BasicShapes/Cube.Cube"));
			if (MeshFinder.Succeeded()) MeshComp->SetStaticMesh(MeshFinder.Object);

			static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(
				TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
			if (MaterialFinder.Succeeded()) MeshComp->SetMaterial(0, MaterialFinder.Object);
		}
	}
}

#if WITH_EDITOR
void AInstancedStaticMeshActor::UpdateInstances() const
{
	UInstancedStaticMeshComponent* ISMC = GetMeshComponent();
	if (ISMC && !bLockInstances)
	{
		ISMC->ClearInstances();
		ISMC->AddInstances(Instances, false);
	}
}

void AInstancedStaticMeshActor::CopyInstancesFromComponent()
{
	TGuardValue<bool> LockGuard(bLockInstances, true);
	if (const UInstancedStaticMeshComponent* ISMC = GetMeshComponent())
	{
		const int32 NumInstances = ISMC->GetNumInstances();
		Instances.Empty(NumInstances);

		for (int32 i = 0; i < NumInstances; i++)
		{
			FTransform Transform;
			if (ISMC->GetInstanceTransform(i, Transform, false))
			{
				Instances.Add(Transform);
			}
		}
	}
}

void AInstancedStaticMeshActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (!bLockInstances)
	{
		if (Instances.IsEmpty())
		{
			Instances.Add(FTransform::Identity);
		}

		UpdateInstances();
	}
}
#endif

AHierarchicalInstancedStaticMeshActor::AHierarchicalInstancedStaticMeshActor(const FObjectInitializer& ObjectInit)
	: Super(ObjectInit.SetDefaultSubobjectClass<UHierarchicalInstancedStaticMeshComponent>(StaticMeshComponentName))
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bCanBeInCluster = GetClass() == AHierarchicalInstancedStaticMeshActor::StaticClass();
}
