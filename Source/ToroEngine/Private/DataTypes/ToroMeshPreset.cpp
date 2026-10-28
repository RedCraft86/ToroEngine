// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DataTypes/ToroMeshPreset.h"
#include "Utilities/ToroLogging.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/StaticMesh.h"
#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif

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
		TORO_REPORT_ERROR(TEXT("SetStaticMesh did not accept the new mesh."));
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

#if WITH_EDITOR
bool FToroStaticMeshPreset::CanEditChange(const FEditPropertyChain& PropertyChain) const
{
	if (!bUseTransform)
	{
		for (const FProperty* Property : PropertyChain)
		{
			if (Property && Property->GetFName() == GET_MEMBER_NAME_CHECKED(FToroStaticMeshPreset, Transform))
			{
				return false;
			}
		}
	}

	return true;
}
#endif

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

#if WITH_EDITOR
bool FToroSplineMeshPreset::CanEditChange(const FEditPropertyChain& PropertyChain) const
{
	if (!bUseTransform)
	{
		for (const FProperty* Property : PropertyChain)
		{
			if (Property && (
				Property->GetFName() == GET_MEMBER_NAME_CHECKED(FToroSplineMeshPreset, Transform)
				|| Property->GetFName() == GET_MEMBER_NAME_CHECKED(FToroSplineMeshPreset, StartPos)
				|| Property->GetFName() == GET_MEMBER_NAME_CHECKED(FToroSplineMeshPreset, EndPos)
				|| Property->GetFName() == GET_MEMBER_NAME_CHECKED(FToroSplineMeshPreset, StartTangent)
				|| Property->GetFName() == GET_MEMBER_NAME_CHECKED(FToroSplineMeshPreset, EndTangent)))
			{
				return false;
			}
		}
	}

	return true;
}
#endif

bool UToroMeshPresetLibrary::IsStaticMeshPresetValid(const FToroStaticMeshPreset& Preset)
{
	return Preset.IsValid();
}

bool UToroMeshPresetLibrary::EqualStaticMeshPreset(const FToroStaticMeshPreset& A, const FToroStaticMeshPreset& B, bool bCheckTransform)
{
	return A.Equals(B, bCheckTransform);
}

bool UToroMeshPresetLibrary::NotEqualStaticMeshPreset(const FToroStaticMeshPreset& A, const FToroStaticMeshPreset& B, bool bCheckTransform)
{
	return !A.Equals(B, bCheckTransform);
}

void UToroMeshPresetLibrary::FillStaticMeshPresetMaterials(FToroStaticMeshPreset& Preset, bool bOverwrite)
{
	Preset.FillMaterials(bOverwrite);
}

bool UToroMeshPresetLibrary::CaptureStaticMeshPreset(FToroStaticMeshPreset& Preset, UStaticMeshComponent* Target, bool bIncludeTransform)
{
	return Preset.FromMeshComponent(Target, bIncludeTransform);
}

bool UToroMeshPresetLibrary::ApplyStaticMeshPreset(const FToroStaticMeshPreset& Preset, UStaticMeshComponent* Target)
{
	return Preset.ToMeshComponent(Target);
}

bool UToroMeshPresetLibrary::IsSplineMeshPresetValid(const FToroSplineMeshPreset& Preset)
{
	return Preset.IsValid();
}

bool UToroMeshPresetLibrary::EqualSplineMeshPreset(const FToroSplineMeshPreset& A, const FToroSplineMeshPreset& B, bool bCheckTransform)
{
	return A.Equals(B, bCheckTransform);
}

bool UToroMeshPresetLibrary::NotEqualSplineMeshPreset(const FToroSplineMeshPreset& A, const FToroSplineMeshPreset& B, bool bCheckTransform)
{
	return !A.Equals(B, bCheckTransform);
}

void UToroMeshPresetLibrary::FillSplineMeshPresetMaterials(FToroSplineMeshPreset& Preset, bool bOverwrite)
{
	Preset.FillMaterials(bOverwrite);
}

bool UToroMeshPresetLibrary::CaptureSplineMeshPreset(FToroSplineMeshPreset& Preset, USplineMeshComponent* Target, bool bIncludeTransform)
{
	return Preset.FromMeshComponent(Target, bIncludeTransform);
}

bool UToroMeshPresetLibrary::ApplySplineMeshPreset(const FToroSplineMeshPreset& Preset, USplineMeshComponent* Target)
{
	return Preset.ToMeshComponent(Target);
}
