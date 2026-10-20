// Copyright (C) RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DataTypes/ToroMeshPreset.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/StaticMesh.h"

bool FToroStaticMeshPreset::Equals(const FToroStaticMeshPreset& Other, bool bCheckTransform) const
{
	if (StaticMesh != Other.StaticMesh
		|| bCastShadows != Other.bCastShadows
		|| bAffectIndirectLighting != Other.bAffectIndirectLighting
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

void FToroStaticMeshPreset::FillMaterials(bool bOverwrite)
{
	if (bOverwrite)
	{
		Materials.Empty();
	}

	if (UStaticMesh* Mesh = StaticMesh.LoadSynchronous())
	{
		const TArray<FStaticMaterial>& StaticMaterials = Mesh->GetStaticMaterials();
		Materials.SetNum(StaticMaterials.Num());

		for (int32 i = 0; i < StaticMaterials.Num(); i++)
		{
			if (!Materials[i].LoadSynchronous())
			{
				Materials[i] = StaticMaterials[i].MaterialInterface;
			}
		}
	}
}

bool FToroStaticMeshPreset::FromMeshComponent(const UStaticMeshComponent* Target, bool bIncludeTransform)
{
	if (!::IsValid(Target))
	{
		return false;
	}

	StaticMesh = Target->GetStaticMesh();

	bCastShadows = Target->CastShadow;
	bAffectIndirectLighting = Target->bAffectDynamicIndirectLighting;

	OverlayMaterial = Target->GetOverlayMaterial();
	Materials.SetNum(Target->GetNumMaterials());
	for (int32 i = 0; i < Materials.Num(); i++)
	{
		Materials[i] = Target->GetMaterial(i);
	}

	bUseTransform = bIncludeTransform;
	Transform = bIncludeTransform ? Target->GetComponentTransform() : FTransform::Identity;

	return true;
}

bool FToroStaticMeshPreset::ToMeshComponent(UStaticMeshComponent* Target) const
{
	if (!::IsValid(Target))
	{
		return false;
	}

	UStaticMesh* TargetMesh = StaticMesh.LoadSynchronous();
	if (Target->GetStaticMesh() != TargetMesh && !Target->SetStaticMesh(TargetMesh))
	{
		FFrame::KismetExecutionMessage(TEXT("SetStaticMesh did not accept the new mesh."), ELogVerbosity::Error);
		return false;
	}

	Target->SetCastShadow(bCastShadows);
	Target->SetAffectDynamicIndirectLighting(bAffectIndirectLighting);

	Target->SetOverlayMaterial(OverlayMaterial.LoadSynchronous());
	const int32 NumMaterials = FMath::Min(Materials.Num(), Target->GetNumMaterials());
	for (int32 i = 0; i < NumMaterials; i++)
	{
		if (UMaterialInterface* Material = Materials[i].LoadSynchronous())
		{
			Target->SetMaterial(i, Material);
		}
	}

	if (bUseTransform)
	{
		Target->SetWorldTransform(Transform);
	}

	return true;
}

bool FToroSplineMeshPreset::Equals(const FToroSplineMeshPreset& Other, bool bCheckTransform) const
{
	if (!Equals(static_cast<const FToroStaticMeshPreset&>(Other), bCheckTransform) || ForwardAxis != Other.ForwardAxis)
	{
		return false;
	}

	return !bCheckTransform || (
		StartPos.Equals(Other.StartPos)
		&& EndPos.Equals(Other.EndPos)
		&& StartTangent.Equals(Other.StartTangent)
		&& EndTangent.Equals(Other.EndTangent)
	);
}

bool FToroSplineMeshPreset::FromMeshComponent(const UStaticMeshComponent* Target, bool bIncludeTransform)
{
	const USplineMeshComponent* SplineMeshComp = Cast<USplineMeshComponent>(Target);
	if (!::IsValid(SplineMeshComp))
	{
		return false;
	}

	if (FToroStaticMeshPreset::FromMeshComponent(Target, bIncludeTransform))
	{
		ForwardAxis = SplineMeshComp->GetForwardAxis();
		bUseTransform = bIncludeTransform;
		if (bIncludeTransform)
		{
			StartPos = SplineMeshComp->GetStartPosition();
			EndPos = SplineMeshComp->GetEndPosition();
			StartTangent = SplineMeshComp->GetStartTangent();
			EndTangent = SplineMeshComp->GetEndTangent();
		}
		else
		{
			StartPos = EndPos = StartTangent = EndTangent = FVector::ZeroVector;
		}

		return true;
	}

	return false;
}

bool FToroSplineMeshPreset::ToMeshComponent(UStaticMeshComponent* Target) const
{
	USplineMeshComponent* SplineMeshComp = Cast<USplineMeshComponent>(Target);
	if (!::IsValid(SplineMeshComp))
	{
		return false;
	}

	if (!FToroStaticMeshPreset::ToMeshComponent(Target))
	{
		return false;
	}

	SplineMeshComp->SetForwardAxis(ForwardAxis, false);
	if (bUseTransform)
	{
		SplineMeshComp->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent, false);
	}
	SplineMeshComp->UpdateMesh();

	return true;
}
