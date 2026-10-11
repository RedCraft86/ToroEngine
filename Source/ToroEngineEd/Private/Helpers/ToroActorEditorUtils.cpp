// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Helpers/ToroActorEditorUtils.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/SplineMeshActor.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "ProceduralMeshComponent.h"
#include "EditorDirectories.h"
#include "AssetToolsModule.h"
#include "PhysicsEngine/BodySetup.h"
#include "ProceduralMeshConversion.h"
#include "ToroEngineEd.h"
#include "Actors/InstancedStaticMeshActor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Dialogs/DlgPickAssetPath.h"

#define LOCTEXT_NAMESPACE "ToroEngineEd"
bool FToroActorEditorUtils::DestroyActors(const TArray<AActor*>& Actors)
{
	if (IsValid(Subsystem.Get()))
	{
		const FScopedTransaction Transaction(LOCTEXT("DestroyActors", "Destroy Actors"));
		Subsystem->DestroyActors(Actors);
		return true;
	}

	return false;
}

TArray<AActor*> FToroActorEditorUtils::GetSelectedActors(bool bWarnEmpty)
{
	TArray<AActor*> Actors;
	if (IsValid(Subsystem.Get()))
	{
		Actors = Subsystem->GetSelectedLevelActors();
	}

	if (Actors.IsEmpty())
	{
		static const FNotificationInfo Info(LOCTEXT("NoActorsSelectedNotification", "No Actors Selected!"));
		FSlateNotificationManager::Get().AddNotification(Info);
	}

	return Actors;
}

TArray<AActor*> FToroActorEditorUtils::SplitActors(const TArray<AActor*>& Actors, bool bDestroySources)
{
	TArray<AActor*> Results;
	if (Actors.IsEmpty() || !IsValid(Subsystem.Get()))
	{
		return Results;
	}

	TArray<AActor*> Processed, Missed;
	const int32 SrcCount = Actors.Num();
	UE_LOG(LogToroEngineEd, Display, TEXT("[SplitActors] Starting split for %d actors..."), SrcCount);
	{
		const FScopedTransaction Transaction(LOCTEXT("SplitActors", "Split Actors"));

		Results.Reserve(SrcCount);
		Missed.Reserve(SrcCount / 2);
		for (int32 i = 0; i < SrcCount; i++)
		{
			if (IsValid(Actors[i]))
			{
				TArray<UMeshComponent*> Components;
				Actors[i]->GetComponents<UMeshComponent>(Components);
				for (const UMeshComponent* Comp : Components)
				{
					TArray<AActor*> BakeResult = BakeMeshComponent(Comp);
					if (!BakeResult.IsEmpty())
					{
						Processed.Add(Actors[i]);
						Results.Append(BakeResult);
					}
					else
					{
						Missed.Add(Actors[i]);
					}
				}
			}

			UE_LOG(LogToroEngineEd, Display, TEXT("[SplitActors] \t Processed [%d/%d] actors"), i + 1, SrcCount);
		}
	}

	const int32 CreatedNum = Results.Num();
	Results.Append(Missed);

	Subsystem->SetSelectedLevelActors(Results);

	if (bDestroySources)
	{
		DestroyActors(Processed);
	}

	UE_LOG(LogToroEngineEd, Display, TEXT("[SplitActors] Finished splitting with %d actors created!"), CreatedNum);
	return Results;
}

TArray<AActor*> FToroActorEditorUtils::InstanceActors(const TArray<AActor*>& Actors, bool bUseHISM, bool bDestroySources)
{
	TArray<AActor*> Results;
	if (Actors.IsEmpty() || !IsValid(Subsystem.Get()))
	{
		return Results;
	}

	TMap<FToroBaseMeshPreset, TArray<FTransform>> Instances;
	TArray<const UMeshComponent*> RemainComponents;

	const int32 SrcCount = Actors.Num();
	UE_LOG(LogToroEngineEd, Display, TEXT("[InstanceActors] Starting instancing for %d actors..."), SrcCount);
	{
		const FScopedTransaction Transaction(LOCTEXT("InstanceActors", "Instance Actors"));

		RemainComponents.Reserve(SrcCount / 2);
		for (int32 i = 0; i < SrcCount; i++)
		{
			if (IsValid(Actors[i]))
			{
				TArray<UMeshComponent*> Components;
				Actors[i]->GetComponents<UMeshComponent>(Components);
				for (const UMeshComponent* Comp : Components)
				{
					if (!ExtractMeshComponent(Comp, Instances))
					{
						RemainComponents.Add(Comp);
					}
				}
			}

			UE_LOG(LogToroEngineEd, Display, TEXT("[InstanceActors] \t\t Processed [%d/%d] actors"), i + 1, SrcCount);
		}

		const int32 InstCount = Instances.Num();
		const int32 RemainCompCount = RemainComponents.Num();
		Instances.Reserve(InstCount + RemainCompCount);

		UE_LOG(LogToroEngineEd, Display, TEXT("[InstanceActors] \t %d Unique instances found"), InstCount);

		int32 InstIdx = 0;
		for (const TPair<FToroBaseMeshPreset, TArray<FTransform>>& Instance : Instances)
		{
			if (!Instance.Key.IsValid() || Instance.Value.IsEmpty())
			{
				continue;
			}

			if (Instance.Value.Num() == 1)
			{
				if (AStaticMeshActor* Actor = SpawnMeshActor(Instance.Value[0], nullptr, Instance.Key.StaticMesh.GetAssetName()))
				{
					Instance.Key.ToMeshComponent(Actor->GetStaticMeshComponent());
					Results.Add(Actor);
				}
			}
			else if (AInstancedStaticMeshActor* Actor = Cast<AInstancedStaticMeshActor>(SpawnMeshActor(
				bUseHISM ? AHierarchicalInstancedStaticMeshActor::StaticClass() : AInstancedStaticMeshActor::StaticClass(),
				Instance.Value[0], nullptr, Instance.Key.StaticMesh.GetAssetName())))
			{
				if (UInstancedStaticMeshComponent* Comp = Actor->GetMeshComponent<UInstancedStaticMeshComponent>())
				{
					Instance.Key.ToMeshComponent(Comp);

					Comp->ClearInstances();
					for (const FTransform Transform : Instance.Value)
					{
						Comp->AddInstance(Transform, true);
					}
				}

				Actor->CopyInstancesFromComponent();
				Results.Add(Actor);
			}

			UE_LOG(LogToroEngineEd, Display, TEXT("[InstanceActors] \t\t Processed [%d/%d] unique instances"), ++InstIdx, InstCount);
		}

		UE_LOG(LogToroEngineEd, Display, TEXT("[InstanceActors] \t %d Components remain"), RemainCompCount);

		for (int32 i = 0; i < RemainCompCount; i++)
		{
			TArray<AActor*> BakeResult = BakeMeshComponent(RemainComponents[i]);
			if (!BakeResult.IsEmpty())
			{
				Results.Append(BakeResult);
			}

			UE_LOG(LogToroEngineEd, Display, TEXT("[InstanceActors] \t\t Processed [%d/%d] remaining components"), i + 1, RemainCompCount);
		}
	}

	Subsystem->SetSelectedLevelActors(Results);

	if (bDestroySources)
	{
		DestroyActors(Actors);
	}

	UE_LOG(LogToroEngineEd, Display, TEXT("[InstanceActors] Finished instancing with %d actors created!"), Results.Num());
	return Results;
}

bool FToroActorEditorUtils::LayoutActors(const TArray<AActor*>& Actors, uint8 MaxColumns, float YawOffset, const FVector2D& XYOffset)
{
	if (Actors.IsEmpty() || MaxColumns == 0 || !IsValid(Subsystem.Get()))
	{
		return false;
	}

	struct FGridEntry final
	{
		TWeakObjectPtr<AActor> Actor;
		FVector Origin, Extent;

		FGridEntry(AActor* InActor): Actor(InActor)
		{
			InActor->GetActorBounds(false, Origin, Extent);
		}

		double GetArea() const
		{
			return Extent.X * Extent.Y;
		}
	};

	const int32 ActorCount = Actors.Num();
	UE_LOG(LogToroEngineEd, Display, TEXT("[LayoutActors] Starting layout for %d actors..."), ActorCount);

	TArray<FGridEntry> Entries;
	Entries.Reserve(ActorCount);

	FVector2D CellSize = FVector2D::ZeroVector;
	for (AActor* Actor : Actors)
	{
		if (IsValid(Actor))
		{
			const FGridEntry& Entry = Entries.Emplace_GetRef(Actor);
			CellSize.X = FMath::Max(Entry.Extent.X * 2.0f, CellSize.X);
			CellSize.Y = FMath::Max(Entry.Extent.Y * 2.0f, CellSize.Y);
		}
	}

	if (Entries.IsEmpty())
	{
		return false;
	}

	CellSize += XYOffset;

	UE_LOG(LogToroEngineEd, Display, TEXT("[LayoutActors] \t Cell Size: %.1f x %.1f"), CellSize.X, CellSize.Y);

	Entries.Sort([](const FGridEntry& A, const FGridEntry& B){
		return A.GetArea() > B.GetArea();
	});

	const int32 EntryCount = Entries.Num();
	for (int32 i = 0; i < EntryCount; i++)
	{
		const FIntPoint Pos(i % MaxColumns, i / MaxColumns);
		const FGridEntry& Entry = Entries[i];
		if (Entry.Actor.IsValid())
		{
			const FVector PivotToBounds = Entry.Actor->GetActorLocation() - Entry.Origin;
			const FVector CellOffset(Pos.X * CellSize.X, Pos.Y * CellSize.Y, Entry.Extent.Z);

			FTransform Transform = Entry.Actor->GetActorTransform();
			Transform.SetRotation(FRotator(0.0f, FMath::UnwindDegrees(YawOffset), 0.0f).Quaternion());
			Transform.SetLocation(PivotToBounds + CellOffset);

			Subsystem->SetActorTransform(Entry.Actor.Get(), Transform);
		}

		UE_LOG(LogToroEngineEd, Display, TEXT("[LayoutActors] \t Laid out [%d/%d] actor at (%d, %d)"), i + 1, EntryCount, Pos.X, Pos.Y);
	}

	UE_LOG(LogToroEngineEd, Display, TEXT("[LayoutActors] Finished laying out %d actors!"), EntryCount);
	return true;
}

TArray<AActor*> FToroActorEditorUtils::BakeMeshComponent(const UMeshComponent* Src)
{
	TArray<AActor*> Results;
	if (!IsValid(Src))
	{
		return Results;
	}
	if (const UInstancedStaticMeshComponent* ISM = Cast<UInstancedStaticMeshComponent>(Src))
	{
		Results = BakeInstancedMesh(ISM);
	}
	if (const USplineMeshComponent* SMC = Cast<USplineMeshComponent>(Src))
	{
		Results.Add(BakeSplineMesh(SMC));
	}
	if (const UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(Src))
	{
		Results.Add(BakeStaticMesh(SM));
	}
	if (const USkeletalMeshComponent* SKMC = Cast<USkeletalMeshComponent>(Src))
	{
		Results.Add(BakeSkeletalMesh(SKMC));
	}
	if (const UProceduralMeshComponent* PMC = Cast<UProceduralMeshComponent>(Src))
	{
		Results.Add(BakeProceduralMesh(PMC));
	}

	Results.RemoveAll([](const AActor* Element) -> bool { return !IsValid(Element); });
	return Results;
}

TArray<AActor*> FToroActorEditorUtils::BakeInstancedMesh(const UInstancedStaticMeshComponent* Src)
{
	TArray<AActor*> Actors;

	FToroBaseMeshPreset Preset;
	Preset.FromMeshComponent(Src);
	if (Preset.IsValid())
	{
		const int32 InstanceCount(Src->GetNumInstances());

		Actors.Reserve(InstanceCount);
		for (int32 i = 0; i < InstanceCount; i++)
		{
			FTransform InstanceTransform;
			if (Src->GetInstanceTransform(i, InstanceTransform, true))
			{
				if (AStaticMeshActor* Actor = SpawnMeshActor(InstanceTransform, Src->GetOwner(),
					FString::Printf(TEXT("%s_%d"), *Preset.StaticMesh.GetAssetName(), i)))
				{
					UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent();
					Comp->SetAffectIndirectLightingWhileHidden(Src->bAffectIndirectLightingWhileHidden);
					Comp->SetAffectDynamicIndirectLighting(Src->bAffectDynamicIndirectLighting);
					Preset.ToMeshComponent(Comp);
					Actors.Add(Actor);
				}
			}
		}
	}

	return Actors;
}

AActor* FToroActorEditorUtils::BakeSplineMesh(const USplineMeshComponent* Src)
{
	FToroSplineMeshPreset Preset;
	Preset.FromMeshComponent(Src);
	if (Preset.IsValid())
	{
		if (ASplineMeshActor* Actor = SpawnMeshActor<ASplineMeshActor>(Src->GetComponentTransform(),
			Src->GetOwner(), Preset.StaticMesh.GetAssetName()))
		{
			USplineMeshComponent* Comp = Actor->GetSplineMeshComponent();
			Comp->SetAffectIndirectLightingWhileHidden(Src->bAffectIndirectLightingWhileHidden);
			Comp->SetAffectDynamicIndirectLighting(Src->bAffectDynamicIndirectLighting);
			Preset.ToMeshComponent(Comp);

			const FVector StartPos = Src->GetStartPosition();
			const FVector StartTan = Src->GetStartTangent();
			const FVector EndPos = Src->GetEndPosition();
			const FVector EndTan = Src->GetEndTangent();
			Comp->SetStartAndEnd(StartPos, StartTan, EndPos, EndTan, true);

			return Actor;
		}
	}

	return nullptr;
}

AActor* FToroActorEditorUtils::BakeStaticMesh(const UStaticMeshComponent* Src)
{
	FToroBaseMeshPreset Preset;
	Preset.FromMeshComponent(Src);
	if (Preset.IsValid())
	{
		if (AStaticMeshActor* Actor = SpawnMeshActor(Src->GetComponentTransform(), Src->GetOwner(), Preset.StaticMesh.GetAssetName()))
		{
			UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent();
			Comp->SetAffectIndirectLightingWhileHidden(Src->bAffectIndirectLightingWhileHidden);
			Comp->SetAffectDynamicIndirectLighting(Src->bAffectDynamicIndirectLighting);
			Preset.ToMeshComponent(Comp);
			return Actor;
		}
	}

	return nullptr;
}

AActor* FToroActorEditorUtils::BakeSkeletalMesh(const USkeletalMeshComponent* Src)
{
	if (!Src || !Src->GetSkeletalMeshAsset())
	{
		return nullptr;
	}

	const TSoftObjectPtr<USkeletalMesh> SkeletalMesh(Src->GetSkeletalMeshAsset());
	if (ASkeletalMeshActor* Actor = SpawnMeshActor<ASkeletalMeshActor>(Src->GetComponentTransform(), Src->GetOwner(), SkeletalMesh.GetAssetName()))
	{
		USkeletalMeshComponent* Comp = Actor->GetSkeletalMeshComponent();
		Comp->SetAffectIndirectLightingWhileHidden(Src->bAffectIndirectLightingWhileHidden);
		Comp->SetAffectDynamicIndirectLighting(Src->bAffectDynamicIndirectLighting);
		Comp->SetCastShadow(Src->CastShadow);

		Comp->SetSkeletalMesh(SkeletalMesh.LoadSynchronous());
		Comp->SetOverlayMaterial(Src->GetOverlayMaterial());
		for (int32 i = 0; i < Src->GetNumMaterials(); i++)
		{
			Comp->SetMaterial(i, Src->GetMaterial(i));
		}

		Comp->SetPlayRate(Src->GetPlayRate());
		Comp->GlobalAnimRateScale = Src->GlobalAnimRateScale;

		Comp->SetAnimationMode(Src->GetAnimationMode());
		if (const UAnimInstance* AnimInst = Src->GetAnimInstance())
		{
			Comp->SetAnimInstanceClass(AnimInst->GetClass());
		}
		if (const UAnimSingleNodeInstance* Node = Src->GetSingleNodeInstance(); Node && Node->GetAnimationAsset())
		{
			Comp->PlayAnimation(Node->GetAnimationAsset(), Node->IsLooping());
		}

		return Actor;
	}

	return nullptr;
}

AActor* FToroActorEditorUtils::BakeProceduralMesh(const UProceduralMeshComponent* Src)
{
	const FAssetToolsModule& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	FString DefaultPath;
	FPackageName::TryConvertFilenameToLongPackageName(
		FEditorDirectories::Get().GetLastDirectory(ELastDirectory::NEW_ASSET),
		DefaultPath
	);

	FString AssetName, PackageName = DefaultPath.IsEmpty() ? TEXT("/Game/Meshes") : DefaultPath;
	PackageName /= Src->GetOwner() ? Src->GetOwner()->GetActorLabel() : TEXT("SM_ProcMesh");
	AssetTools.Get().CreateUniqueAssetName(PackageName, TEXT("_0"), PackageName, AssetName);

	const TSharedPtr<SDlgPickAssetPath> PathWidget = SNew(SDlgPickAssetPath)
		.Title(LOCTEXT("SaveProceduralMeshLocation", "Choose New StaticMesh Location"))
		.DefaultAssetPath(FText::FromString(PackageName));

	if (PathWidget->ShowModal() != EAppReturnType::Ok)
	{
		return nullptr;
	}

	FString UserPkgName = PathWidget->GetFullAssetPath().ToString();
	FName MeshName = *FPackageName::GetLongPackageAssetName(UserPkgName);
	if (MeshName.IsNone())
	{
		UserPkgName = PackageName;
		MeshName = *AssetName;
	}

	FMeshDescription MeshDescription = BuildMeshDescription(const_cast<UProceduralMeshComponent*>(Src));
	if (MeshDescription.Polygons().Num() == 0)
	{
		return nullptr;
	}

	UPackage* Package = CreatePackage(*UserPkgName);
	checkf(Package, TEXT("Failed to create package for %s"), *UserPkgName);

	UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, MeshName, RF_Public | RF_Standalone);
	StaticMesh->InitResources();
	StaticMesh->SetLightingGuid();

	FStaticMeshSourceModel& SrcModel = StaticMesh->AddSourceModel();
	SrcModel.BuildSettings.bUseHighPrecisionTangentBasis = false;
	SrcModel.BuildSettings.bUseFullPrecisionUVs = false;
	SrcModel.BuildSettings.bGenerateLightmapUVs = true;
	SrcModel.BuildSettings.bRemoveDegenerates = false;
	SrcModel.BuildSettings.bRecomputeTangents = false;
	SrcModel.BuildSettings.bRecomputeNormals = false;
	SrcModel.BuildSettings.SrcLightmapIndex = 0;
	SrcModel.BuildSettings.DstLightmapIndex = 1;

	StaticMesh->CreateMeshDescription(0, MoveTemp(MeshDescription));
	StaticMesh->CommitMeshDescription(0);

	if (!Src->bUseComplexAsSimpleCollision)
	{
		StaticMesh->CreateBodySetup();
		UBodySetup* NewBodySetup = StaticMesh->GetBodySetup();
		if (Src->ProcMeshBodySetup)
		{
			NewBodySetup->AggGeom.ConvexElems = Src->ProcMeshBodySetup->AggGeom.ConvexElems;
		}

		NewBodySetup->CollisionTraceFlag = CTF_UseDefault;
		NewBodySetup->bGenerateMirroredCollision = false;
		NewBodySetup->BodySetupGuid = FGuid::NewGuid();
		NewBodySetup->bDoubleSidedGeometry = true;
		NewBodySetup->CreatePhysicsMeshes();
	}

	for (int32 i = 0; i < Src->GetNumSections(); i++)
	{
		StaticMesh->GetStaticMaterials().Add(FStaticMaterial(
			Src->GetMaterial(i), *FString::Printf(TEXT("Section_%d"), i)
		));
	}

	StaticMesh->SetImportVersion(EImportStaticMeshVersion::LastVersion);
	StaticMesh->Build(false);
	StaticMesh->PostEditChange();

	FAssetRegistryModule::AssetCreated(StaticMesh);

	if (AStaticMeshActor* Actor = SpawnMeshActor(Src->GetComponentTransform(), Src->GetOwner(), MeshName.ToString()))
	{
		Actor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
		return Actor;
	}

	return nullptr;
}

bool FToroActorEditorUtils::ExtractMeshComponent(const UMeshComponent* Src, TMap<FToroBaseMeshPreset, TArray<FTransform>>& Instances)
{
	if (!IsValid(Src))
	{
		return false;
	}
	if (const UInstancedStaticMeshComponent* ISM = Cast<UInstancedStaticMeshComponent>(Src))
	{
		return ExtractInstancedMesh(ISM, Instances);
	}
	if (const UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(Src))
	{
		return ExtractStaticMesh(SM, Instances);
	}
	return false;
}

bool FToroActorEditorUtils::ExtractInstancedMesh(const UInstancedStaticMeshComponent* Src, TMap<FToroBaseMeshPreset, TArray<FTransform>>& Instances)
{
	FToroBaseMeshPreset Preset;
	Preset.FromMeshComponent(Src);
	if (Preset.IsValid())
	{
		TArray<FTransform>& Transforms = Instances.FindOrAdd(Preset);
		for (int32 i = 0; i < Src->GetNumInstances(); i++)
		{
			FTransform Transform;
			if (Src->GetInstanceTransform(i, Transform, true))
			{
				Transforms.Add(Transform);
			}
		}

		return true;
	}

	return false;
}

bool FToroActorEditorUtils::ExtractStaticMesh(const UStaticMeshComponent* Src, TMap<FToroBaseMeshPreset, TArray<FTransform>>& Instances)
{
	FToroBaseMeshPreset Preset;
	Preset.FromMeshComponent(Src);
	if (Preset.IsValid())
	{
		TArray<FTransform>& Transforms = Instances.FindOrAdd(Preset);
		Transforms.Add(Src->GetComponentTransform());
		return true;
	}

	return false;
}

AActor* FToroActorEditorUtils::SpawnMeshActor(const TSubclassOf<AActor>& Class,
	const FTransform& Transform, const AActor* Owner, const FString& Name)
{
	if (IsValid(Class) && IsValid(Subsystem.Get()))
	{
		if (AActor* NewActor = Subsystem->SpawnActorFromClass(Class, Transform.GetTranslation(), Transform.Rotator(), false))
		{
			NewActor->SetActorScale3D(Transform.GetScale3D());

			const FString TrimmedName(Name.TrimStartAndEnd());
			NewActor->SetActorLabel(FString::Printf(TEXT("%s_%s"),
				TrimmedName.IsEmpty() ? TEXT("Unknown") : *TrimmedName,
				*FGuid::NewGuid().ToString().Left(5)));

			if (Owner)
			{
				NewActor->SetActorHiddenInGame(Owner->IsHidden());
				NewActor->SetFolderPath(*(Owner->GetFolderPath().ToString() / *Owner->GetActorLabel()));
			}
			else
			{
				NewActor->SetFolderPath(*FString::Printf(TEXT("Spawned_%s"),
					*FDateTime::Now().ToString(TEXT("%S%M%H%d%m%Y")).Left(5)
				));
			}

			return NewActor;
		}
	}

	return nullptr;
}
#undef LOCTEXT_NAMESPACE
