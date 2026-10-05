// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "DataTypes/ToroMeshPreset.h"
#include "Components/SplineMeshComponent.h"
#include "ToroCore.h"

namespace
{
	bool ApplyMeshPreset(const FToroBaseMeshPreset& Preset, UStaticMeshComponent& Target)
	{
		UStaticMesh* RequestedMesh = Preset.StaticMesh.LoadSynchronous();

		Target.SetStaticMesh(RequestedMesh);
		if (Target.GetStaticMesh() != RequestedMesh)
		{
			UE_LOG(LogToroCore, Warning, TEXT("Could not apply mesh preset to %s: the component rejected the requested mesh."),
				*Target.GetPathName());
			return false;
		}

		Target.EmptyOverrideMaterials();
		const int32 NumOverrides = FMath::Min(Preset.Materials.Num(), Target.GetNumMaterials());
		for (int32 Idx = 0; Idx < NumOverrides; ++Idx)
		{
			if (UMaterialInterface* Material = Preset.Materials[Idx].LoadSynchronous(); IsValid(Material))
			{
				Target.SetMaterial(Idx, Material);
			}
		}

		Target.SetOverlayMaterial(Preset.OverlayMaterial.LoadSynchronous());
		Target.SetCastShadow(Preset.bCastShadows);
		if (Preset.bUseTransform)
		{
			Target.SetWorldTransform(Preset.Transform);
		}

		return true;
	}
}

void FToroBaseMeshPreset::FillMaterials(const bool bOverwrite)
{
	if (bOverwrite)
	{
		Materials.Empty();
	}

	const int32 BaseIdx = Materials.Num() - 1;
	if (StaticMesh.LoadSynchronous())
	{
		const TArray<FStaticMaterial>& StaticMats = StaticMesh->GetStaticMaterials();
		Materials.Reserve(StaticMats.Num());

		for (int32 Idx = 0; Idx < StaticMats.Num(); Idx++)
		{
			if (Idx > BaseIdx)
			{
				Materials.Add(StaticMats[Idx].MaterialInterface);
			}
			else if (!::IsValid(Materials[Idx].LoadSynchronous()))
			{
				Materials[Idx] = StaticMats[Idx].MaterialInterface;
			}
		}
	}
}

bool FToroBaseMeshPreset::Equals(const FToroBaseMeshPreset& Other, const bool bCheckTransform) const
{
	if (StaticMesh != Other.StaticMesh
		|| bCastShadows != Other.bCastShadows
		|| OverlayMaterial != Other.OverlayMaterial
		|| Materials.Num() != Other.Materials.Num())
	{
		return false;
	}

	if (bCheckTransform && (bUseTransform != Other.bUseTransform || !Transform.Equals(Other.Transform)))
	{
		return false;
	}

	for (int32 i = 0; i < Materials.Num(); i++)
	{
		if (Materials[i] != Other.Materials[i])
		{
			return false;
		}
	}

	return true;
}

void FToroBaseMeshPreset::FromMeshComponent(const UStaticMeshComponent* Target, const bool bIncludeTransform)
{
	if (::IsValid(Target))
	{
		StaticMesh = Target->GetStaticMesh();
		bCastShadows = Target->CastShadow;
		bUseTransform = bIncludeTransform;
		Transform = bUseTransform ? Target->GetComponentTransform() : FTransform::Identity;
		OverlayMaterial = Target->GetOverlayMaterial();

		Materials.SetNum(Target->GetNumMaterials());
		for (int32 i = 0; i < Target->GetNumMaterials(); i++)
		{
			Materials[i] = Target->GetMaterial(i);
		}

		FillMaterials(false);
	}
}

void FToroBaseMeshPreset::ToMeshComponent(UStaticMeshComponent* Target) const
{
	if (::IsValid(Target))
	{
		ApplyMeshPreset(*this, *Target);
	}
}

bool FToroSplineMeshPreset::Equals(const FToroSplineMeshPreset& Other, const bool bCheckTransform) const
{
	return Equals(static_cast<const FToroBaseMeshPreset&>(Other), bCheckTransform)
		&& ForwardAxis == Other.ForwardAxis;
}

void FToroSplineMeshPreset::FromMeshComponent(const UStaticMeshComponent* Target, const bool bIncludeTransform)
{
	const USplineMeshComponent* SplineMesh = ::IsValid(Target) ? Cast<USplineMeshComponent>(Target) : nullptr;
	if (!SplineMesh)
	{
		UE_LOG(LogToroCore, Warning, TEXT("Called FToroSplineMeshPreset::FromMeshComponent without a valid USplineMeshComponent."));
		return;
	}

	FToroBaseMeshPreset::FromMeshComponent(SplineMesh, bIncludeTransform);
	ForwardAxis = SplineMesh->GetForwardAxis();
}

void FToroSplineMeshPreset::ToMeshComponent(UStaticMeshComponent* Target) const
{
	USplineMeshComponent* SplineMesh = ::IsValid(Target) ? Cast<USplineMeshComponent>(Target) : nullptr;
	if (!SplineMesh)
	{
		UE_LOG(LogToroCore, Warning, TEXT("Called FToroSplineMeshPreset::ToMeshComponent without a valid USplineMeshComponent."));
		return;
	}

	if (ApplyMeshPreset(*this, *SplineMesh))
	{
		SplineMesh->SetForwardAxis(ForwardAxis);
	}
}

bool UToroMeshPresetLibrary::IsMeshPresetValid(const FToroBaseMeshPreset& Target)
{
	return Target.IsValid();
}

bool UToroMeshPresetLibrary::IsEqualsMeshPreset(const FToroBaseMeshPreset& A,
	const FToroBaseMeshPreset& B, const bool bCheckTransform)
{
	return A.Equals(B, bCheckTransform);
}

void UToroMeshPresetLibrary::FillMeshPresetMaterials(FToroBaseMeshPreset& Target, const bool bOverwrite)
{
	Target.FillMaterials(bOverwrite);
}

void UToroMeshPresetLibrary::SetMeshPresetFromComponent(FToroBaseMeshPreset& Target,
	const UStaticMeshComponent* Component, const bool bIncludeTransform)
{
	Target.FromMeshComponent(Component, bIncludeTransform);
}

void UToroMeshPresetLibrary::ApplyMeshPresetToComponent(const FToroBaseMeshPreset& Target, UStaticMeshComponent* Component)
{
	Target.ToMeshComponent(Component);
}

FToroBaseMeshPreset UToroMeshPresetLibrary::SplineMeshPresetToBase(const FToroSplineMeshPreset& Target)
{
	return static_cast<FToroBaseMeshPreset>(Target);
}

bool UToroMeshPresetLibrary::IsSplineMeshPresetValid(const FToroSplineMeshPreset& Target)
{
	return Target.IsValid();
}

bool UToroMeshPresetLibrary::IsEqualsSplineMeshPreset(const FToroSplineMeshPreset& A,
	const FToroSplineMeshPreset& B, const bool bCheckTransform)
{
	return A.Equals(B, bCheckTransform);
}

void UToroMeshPresetLibrary::FillSplineMeshPresetMaterials(FToroSplineMeshPreset& Target, const bool bOverwrite)
{
	Target.FillMaterials(bOverwrite);
}

void UToroMeshPresetLibrary::SetSplineMeshPresetFromComponent(FToroSplineMeshPreset& Target,
	const USplineMeshComponent* Component, const bool bIncludeTransform)
{
	Target.FromMeshComponent(Component, bIncludeTransform);
}

void UToroMeshPresetLibrary::ApplySplineMeshPresetToComponent(const FToroSplineMeshPreset& Target, USplineMeshComponent* Component)
{
	Target.ToMeshComponent(Component);
}
