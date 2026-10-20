// Copyright (C) RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Utilities/ToroActorEdUtils.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/SkeletalMeshActor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SplineMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Internationalization/Text.h"
#include "Materials/MaterialInterface.h"
#include "Math/IntPoint.h"
#include "Math/UnrealMathUtility.h"
#include "MeshDescription.h"
#include "Misc/DateTime.h"
#include "Misc/Guid.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "PhysicsEngine/BodySetup.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "AssetToolsModule.h"
#include "Dialogs/DlgPickAssetPath.h"
#include "Editor.h"
#include "EditorDirectories.h"
#include "FileHelpers.h"
#include "Framework/Notifications/NotificationManager.h"
#include "IAssetTools.h"
#include "ScopedTransaction.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralMeshConversion.h"
#include "Actors/InstancedStaticMeshActor.h"
#include "DataTypes/ToroMeshPreset.h"
#include "ToroEngineEd.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
namespace ToroEngine::ActorEdUtils
{
	namespace
	{
		template <typename T>
		T* SpawnMeshActor(const FTransform& Transform, const AActor* Owner, const FString& Label)
		{
			if (AActor* Actor = CreateActor(T::StaticClass(), Transform))
			{
				FString FolderStr;
				if (IsValid(Owner))
				{
					Actor->SetActorHiddenInGame(Owner->IsHidden());
					FolderStr.Appendf(TEXT("%s/%s"),
						*Owner->GetFolderPath().ToString().TrimStartAndEnd(),
						*Owner->GetActorNameOrLabel());
				}
				else
				{
					FolderStr.Appendf(TEXT("Group_%s"),
						*FDateTime::Now().ToString(TEXT("%S%M%H%d%m%Y")).Left(10));
				}
				Actor->SetFolderPath(*FolderStr);

				FString LabelStr = Label.TrimStartAndEnd();
				if (LabelStr.IsEmpty())
				{
					LabelStr.Appendf(TEXT("Spawned_%s"),
						*FGuid::NewGuid().ToString().Left(5));
				}
				Actor->SetActorLabel(LabelStr);

				return Cast<T>(Actor);
			}

			return nullptr;
		}

		TArray<AActor*> BakeInstancedMesh(const UInstancedStaticMeshComponent* Src)
		{
			TArray<AActor*> Actors;
			FToroStaticMeshPreset Preset;
			Preset.FromMeshComponent(Src);
			if (Preset.IsValid())
			{
				const int32 InstanceCount(Src->GetNumInstances());

				Actors.Reserve(InstanceCount);
				for (int32 i = 0; i < InstanceCount; i++)
				{
					FTransform Transform;
					if (Src->GetInstanceTransform(i, Transform, true))
					{
						if (AStaticMeshActor* Actor = SpawnMeshActor<AStaticMeshActor>(
							Transform, Src->GetOwner(), FString::Printf(TEXT("%s_%d"), *Preset.StaticMesh.GetAssetName(), i)))
						{
							UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent();
							if (!Comp || !Preset.ToMeshComponent(Comp))
							{
								DestroyActors({Actor});
								continue;
							}
							Comp->SetAffectIndirectLightingWhileHidden(Src->bAffectIndirectLightingWhileHidden);
							Actors.Add(Actor);
						}
					}
				}
			}

			return Actors;
		}

		AActor* BakeSplineMesh(const USplineMeshComponent* Src)
		{
			FToroSplineMeshPreset Preset;
			Preset.FromMeshComponent(Src, true);
			if (Preset.IsValid())
			{
				if (ASplineMeshActor* Actor = SpawnMeshActor<ASplineMeshActor>(
					Src->GetComponentTransform(), Src->GetOwner(), Preset.StaticMesh.GetAssetName()))
				{
					USplineMeshComponent* Comp = Actor->GetSplineMeshComponent();
					if (!Comp || !Preset.ToMeshComponent(Comp))
					{
						DestroyActors({Actor});
						return nullptr;
					}
					Comp->SetAffectIndirectLightingWhileHidden(Src->bAffectIndirectLightingWhileHidden);
					return Actor;
				}
			}

			return nullptr;
		}

		AActor* BakeStaticMesh(const UStaticMeshComponent* Src)
		{
			FToroStaticMeshPreset Preset;
			Preset.FromMeshComponent(Src);
			if (Preset.IsValid())
			{
				if (AStaticMeshActor* Actor = SpawnMeshActor<AStaticMeshActor>(
					Src->GetComponentTransform(), Src->GetOwner(), Preset.StaticMesh.GetAssetName()))
				{
					UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent();
					if (!Comp || !Preset.ToMeshComponent(Comp))
					{
						DestroyActors({Actor});
						return nullptr;
					}
					Comp->SetAffectIndirectLightingWhileHidden(Src->bAffectIndirectLightingWhileHidden);
					return Actor;
				}
			}

			return nullptr;
		}

		AActor* BakeSkeletalMesh(const USkeletalMeshComponent* Src)
		{
			if (!Src || !Src->GetSkeletalMeshAsset())
			{
				return nullptr;
			}

			const TSoftObjectPtr<USkeletalMesh> SkeletalMesh(Src->GetSkeletalMeshAsset());
			if (ASkeletalMeshActor* Actor = SpawnMeshActor<ASkeletalMeshActor>(
				Src->GetComponentTransform(), Src->GetOwner(), SkeletalMesh.GetAssetName()))
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

		TArray<AActor*> BakeMeshComponent(const UMeshComponent* Src)
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
			else if (const USplineMeshComponent* SMC = Cast<USplineMeshComponent>(Src))
			{
				Results.Add(BakeSplineMesh(SMC));
			}
			else if (const UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(Src))
			{
				Results.Add(BakeStaticMesh(SM));
			}
			else if (const USkeletalMeshComponent* SKMC = Cast<USkeletalMeshComponent>(Src))
			{
				Results.Add(BakeSkeletalMesh(SKMC));
			}
			else if (const UProceduralMeshComponent* PMC = Cast<UProceduralMeshComponent>(Src))
			{
				if (UStaticMesh* StaticMesh = BakeProceduralMesh(PMC))
				{
					if (AStaticMeshActor* Actor = SpawnMeshActor<AStaticMeshActor>(
						Src->GetComponentTransform(), Src->GetOwner(), StaticMesh->GetName()))
					{
						UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent();
						if (Comp && (Comp->GetStaticMesh() == StaticMesh || Comp->SetStaticMesh(StaticMesh)))
						{
							Results.Add(Actor);
						}
						else
						{
							DestroyActors({Actor});
						}
					}
				}
			}

			Results.RemoveAll([](const AActor* Element) -> bool { return !IsValid(Element); });
			return Results;
		}

		bool ExtractInstancedMesh(const UInstancedStaticMeshComponent* Src, TMap<FToroStaticMeshPreset, TArray<FTransform>>& Instances)
		{
			FToroStaticMeshPreset Preset;
			if (Preset.FromMeshComponent(Src) && Preset.IsValid())
			{
				TArray<FTransform>& Transforms = Instances.FindOrAdd(Preset);
				const int32 InstCount = Src->GetNumInstances();
				Transforms.Reserve(Transforms.Num() + InstCount);
				for (int32 i = 0; i < InstCount; i++)
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

		bool ExtractStaticMesh(const UStaticMeshComponent* Src, TMap<FToroStaticMeshPreset, TArray<FTransform>>& Instances)
		{
			FToroStaticMeshPreset Preset;
			if (Preset.FromMeshComponent(Src) && Preset.IsValid())
			{
				TArray<FTransform>& Transforms = Instances.FindOrAdd(Preset);
				Transforms.Add(Src->GetComponentTransform());
				return true;
			}

			return false;
		}

		bool ExtractMeshComponent(const UMeshComponent* Src, TMap<FToroStaticMeshPreset, TArray<FTransform>>& Instances)
		{
			if (!IsValid(Src))
			{
				return true;
			}
			if (const UInstancedStaticMeshComponent* ISM = Cast<UInstancedStaticMeshComponent>(Src))
			{
				return ExtractInstancedMesh(ISM, Instances);
			}
			if (Src->IsA<USplineMeshComponent>())
			{
				// Not handled
				return false;
			}
			if (const UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(Src))
			{
				return ExtractStaticMesh(SM, Instances);
			}

			for (auto It = Instances.CreateIterator(); It; ++It)
			{
				if (!It.Key().IsValid())
				{
					It.RemoveCurrent();
				}
			}

			return false;
		}
	}

	UEditorActorSubsystem* GetSubsystem()
	{
		UEditorActorSubsystem* Subsystem = GEditor ? GEditor->GetEditorSubsystem<UEditorActorSubsystem>() : nullptr;
		ensureMsgf(Subsystem, TEXT("ToroEngine::ActorEdUtils::GetSubsystem failed to get EditorActorSubsystem"));
		return Subsystem;
	}

	bool DestroyActors(const TArray<AActor*>& Actors)
	{
		if (UEditorActorSubsystem* Subsystem = GetSubsystem())
		{
			const FScopedTransaction Transaction(LOCTEXT("DestroyActors", "Destroy Actors"));
			return Subsystem->DestroyActors(Actors);
		}

		return false;
	}

	TArray<AActor*> GetSelectedActors(bool bWarnEmpty)
	{
		TArray<AActor*> Actors;
		if (UEditorActorSubsystem* Subsystem = GetSubsystem())
		{
			Actors = Subsystem->GetSelectedLevelActors();
		}

		if (bWarnEmpty && Actors.IsEmpty())
		{
			const FNotificationInfo Info(LOCTEXT("NoActorsSelectedNotification", "No Actors Selected!"));
			FSlateNotificationManager::Get().AddNotification(Info);
		}

		return Actors;
	}

	AActor* CreateActor(const TSubclassOf<AActor> Class, const FTransform& Transform)
	{
		if (UEditorActorSubsystem* Subsystem = GetSubsystem())
		{
			if (AActor* Actor = Subsystem->SpawnActorFromClass(Class, Transform.GetLocation(), Transform.Rotator()))
			{
				Actor->SetActorScale3D(Transform.GetScale3D());
				return Actor;
			}
		}

		return nullptr;
	}

	TArray<AActor*> SplitActors(const TArray<AActor*>& Actors, bool bDestroySource)
	{
		UEditorActorSubsystem* Subsystem = GetSubsystem();
		if (Actors.IsEmpty() || !Subsystem)
		{
			return TArray<AActor*>();
		}

		TArray<AActor*> Results;
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

		if (bDestroySource)
		{
			DestroyActors(Processed);
		}

		UE_LOG(LogToroEngineEd, Display, TEXT("[SplitActors] Finished splitting with %d actors created!"), CreatedNum);
		return Results;
	}

	TArray<AActor*> InstanceActors(const TArray<AActor*>& Actors, bool bUseHISM, bool bDestroySource)
	{
		UEditorActorSubsystem* Subsystem = GetSubsystem();
		if (Actors.IsEmpty() || !Subsystem)
		{
			return TArray<AActor*>();
		}

		TArray<AActor*> Results;
		TMap<FToroStaticMeshPreset, TArray<FTransform>> Instances;
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
			for (const TPair<FToroStaticMeshPreset, TArray<FTransform>>& Instance : Instances)
			{
				if (!Instance.Key.IsValid() || Instance.Value.IsEmpty())
				{
					continue;
				}

				if (Instance.Value.Num() == 1)
				{
					if (AStaticMeshActor* Actor = SpawnMeshActor<AStaticMeshActor>(
						Instance.Value[0], nullptr, Instance.Key.StaticMesh.GetAssetName()))
					{
						if (Instance.Key.ToMeshComponent(Actor->GetStaticMeshComponent()))
						{
							Results.Add(Actor);
						}
						else
						{
							DestroyActors({Actor});
						}
					}
				}
				else
				{
					AInstancedStaticMeshActor* Actor;
					if (bUseHISM)
					{
						Actor = SpawnMeshActor<AHierarchicalInstancedStaticMeshActor>(
							Instance.Value[0], nullptr, Instance.Key.StaticMesh.GetAssetName());
					}
					else
					{
						Actor = SpawnMeshActor<AInstancedStaticMeshActor>(
							Instance.Value[0], nullptr, Instance.Key.StaticMesh.GetAssetName());
					}

					if (Actor)
					{
						UInstancedStaticMeshComponent* Comp = Actor->GetMeshComponent<UInstancedStaticMeshComponent>();
						if (!Comp || !Instance.Key.ToMeshComponent(Comp))
						{
							DestroyActors({Actor});
							continue;
						}

						Comp->ClearInstances();
						for (const FTransform Transform : Instance.Value)
						{
							Comp->AddInstance(Transform, true);
						}

						Actor->CopyInstancesFromComponent();
						Results.Add(Actor);
					}
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

		if (bDestroySource)
		{
			DestroyActors(Actors);
		}

		UE_LOG(LogToroEngineEd, Display, TEXT("[InstanceActors] Finished instancing with %d actors created!"), Results.Num());
		return Results;
	}

	bool LayoutActors(const TArray<AActor*>& Actors, uint8 MaxColumns, float YawOffset, const FVector2D& XYOffset)
	{
		UEditorActorSubsystem* Subsystem = GetSubsystem();
		if (Actors.IsEmpty() || MaxColumns == 0 || !Subsystem)
		{
			return false;
		}

		struct FGridEntry final
		{
			TWeakObjectPtr<AActor> Actor;
			FVector Origin, Extent;

			FGridEntry(AActor* InActor)
				: Actor(InActor)
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

		Entries.Sort([](const FGridEntry& A, const FGridEntry& B) -> bool
			{
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

	UStaticMesh* BakeProceduralMesh(const UProceduralMeshComponent* Source)
	{
		if (!IsValid(Source))
		{
			return nullptr;
		}

		const FAssetToolsModule& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

		FString DefaultPath;
		FPackageName::TryConvertFilenameToLongPackageName(
			FEditorDirectories::Get().GetLastDirectory(ELastDirectory::NEW_ASSET),
			DefaultPath
		);

		FString AssetName, PackageName = DefaultPath.IsEmpty() ? TEXT("/Game/Meshes") : DefaultPath;
		PackageName /= Source->GetOwner() ? Source->GetOwner()->GetActorLabel() : TEXT("SM_ProcMesh");
		AssetTools.Get().CreateUniqueAssetName(PackageName, TEXT("_0"), PackageName, AssetName);

		const TSharedPtr<SDlgPickAssetPath> PathWidget = SNew(SDlgPickAssetPath)
			.Title(LOCTEXT("SaveProceduralMeshLocation", "Choose New StaticMesh Location"))
			.DefaultAssetPath(FText::FromString(PackageName));

		if (PathWidget->ShowModal() != EAppReturnType::Ok || !IsValid(Source))
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

		FMeshDescription MeshDescription = BuildMeshDescription(const_cast<UProceduralMeshComponent*>(Source));
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

		StaticMesh->CreateBodySetup();
		UBodySetup* NewBodySetup = StaticMesh->GetBodySetup();
		NewBodySetup->CollisionTraceFlag = Source->bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;
		if (!Source->bUseComplexAsSimpleCollision && Source->ProcMeshBodySetup)
		{
			NewBodySetup->AggGeom.ConvexElems = Source->ProcMeshBodySetup->AggGeom.ConvexElems;
		}
		NewBodySetup->bGenerateMirroredCollision = false;
		NewBodySetup->BodySetupGuid = FGuid::NewGuid();
		NewBodySetup->bDoubleSidedGeometry = true;

		for (int32 i = 0; i < Source->GetNumSections(); i++)
		{
			StaticMesh->GetStaticMaterials().Add(FStaticMaterial(
				Source->GetMaterial(i), *FString::Printf(TEXT("Section_%d"), i)
			));
		}

		StaticMesh->SetImportVersion(EImportStaticMeshVersion::LastVersion);
		StaticMesh->Build(false);
		NewBodySetup->CreatePhysicsMeshes();
		StaticMesh->PostEditChange();

		FAssetRegistryModule::AssetCreated(StaticMesh);
		StaticMesh->MarkPackageDirty();
		if (!UEditorLoadingAndSavingUtils::SavePackages({Package}, true))
		{
			UE_LOG(LogToroEngineEd, Warning, TEXT("Failed to save mesh package: %s"), *Package->GetName());
		}

		return StaticMesh;
	}
}
#undef LOCTEXT_NAMESPACE