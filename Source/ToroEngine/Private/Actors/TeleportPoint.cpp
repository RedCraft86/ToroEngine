// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Actors/TeleportPoint.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Character.h"
#include "Utilities/ToroUtilities.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/ArrowComponent.h"
#endif

ATeleportPoint::ATeleportPoint() : bUseSupportPlane(true)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SupportPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SupportPlane"));
	SupportPlane->SetupAttachment(GetRootComponent());
	SupportPlane->SetRelativeScale3D_Direct(FVector(2.0f, 2.0f, 1.0f));
	SupportPlane->SetCollisionResponseToAllChannels(ECR_Ignore);
	SupportPlane->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	SupportPlane->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SupportPlane->SetCollisionObjectType(ECC_WorldStatic);
	SupportPlane->SetCanEverAffectNavigation(false);
	SupportPlane->SetGenerateOverlapEvents(false);
	SupportPlane->SetHiddenInGame(true);
	SupportPlane->SetCastShadow(false);

	// Support plane mesh
	{
		const ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(
		   TEXT("/Engine/BasicShapes/Plane.Plane"));
		if (PlaneFinder.Succeeded()) SupportPlane->SetStaticMesh(PlaneFinder.Object);

		const ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(
			TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (MaterialFinder.Succeeded()) SupportPlane->SetMaterial(0, MaterialFinder.Object);
	}

#if WITH_EDITOR
	PreviewCapsule = CreateEditorOnlyDefaultSubobject<UCapsuleComponent>(TEXT("PreviewCapsule"));
	if (PreviewCapsule)
	{
		PreviewCapsule->SetupAttachment(GetRootComponent());
		PreviewCapsule->InitCapsuleSize(34.0f, 88.0f);
	}

	if (UBillboardComponent* Sprite = GetSpriteComponent())
	{
		Sprite->bIsEditorOnly = true;
		Sprite->SetIsVisualizationComponent(true);
		Sprite->SetRelativeScale3D_Direct(FVector(0.5f));
	}

	if (UArrowComponent* Arrow = GetArrowComponent())
	{
		Arrow->ArrowSize = 1.0f;
		Arrow->bIsEditorOnly = true;
		Arrow->bTreatAsASprite = true;
		Arrow->SetIsVisualizationComponent(true);
	}
#endif

#if WITH_EDITORONLY_DATA
	bRunConstructionScriptOnDrag = false;
	bIsSpatiallyLoaded = false; // Stop World Partition from unloading this
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLODs
	SetCanBeDamaged(false);
}

ATeleportPoint* ATeleportPoint::Get(const UObject* ContextObject, FName SearchTag)
{
	const UWorld* World = ToroEngine::GetWorld(ContextObject);
	if (!IsValid(World) || SearchTag.IsNone())
	{
		return nullptr;
	}

	for (ATeleportPoint* Point : TActorRange<ATeleportPoint>(World))
	{
		if (IsValid(Point) && Point->ActorHasTag(SearchTag))
		{
			return Point;
		}
	}

	return nullptr;
}

void ATeleportPoint::Teleport(ACharacter* Target) const
{
	if (IsValid(Target))
	{
		Target->TeleportTo(GetActorLocation(), GetActorRotation(), false, true);
	}
}

void ATeleportPoint::TeleportPlayer(const int32 PlayerIdx) const
{
	Teleport(UGameplayStatics::GetPlayerCharacter(this, PlayerIdx));
}

void ATeleportPoint::BeginPlay()
{
	Super::BeginPlay();
	SetActorEnableCollision(bUseSupportPlane);
}

#if WITH_EDITOR
void ATeleportPoint::SnapToFloor()
{
	const UWorld* World = GetWorld();
	if (!World || !PreviewCapsule || FloorSearchRange <= 0.0f)
	{
		return;
	}

	const FVector CurrentCenter = PreviewCapsule->GetComponentLocation();
	const float CapsuleHalfHeight = PreviewCapsule->GetScaledCapsuleHalfHeight();
	const FCollisionQueryParams QueryParams(NAME_None, false, this);

	const float TraceOffsets[] = { 0.0f, CapsuleHalfHeight, -CapsuleHalfHeight };
	for (const float TraceOffset : TraceOffsets)
	{
		const FVector TraceStart = CurrentCenter + FVector(0.0, 0.0, TraceOffset);
		const FVector TraceEnd = TraceStart - FVector(0.0, 0.0, FloorSearchRange);

		FHitResult Hit;
		if (!World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			continue;
		}

		DrawDebugLine(World, Hit.TraceStart, Hit.ImpactPoint, FColor::Yellow, false, 5.0f, 0, 2.0f);
		DrawDebugLine(World, Hit.ImpactPoint, Hit.TraceEnd, FColor::Red, false, 5.0f, 0, 2.0f);
		DrawDebugPoint(World, Hit.ImpactPoint, 10.0f, FColor::Green, false, 5.0f, 0);

		constexpr float FloorClearance = 2.0f;
		const double VerticalOffset = Hit.ImpactPoint.Z + CapsuleHalfHeight + FloorClearance - CurrentCenter.Z;
		SetActorLocation(GetActorLocation() + FVector(0.0, 0.0, VerticalOffset));
		return;
	}

	// Draw failure line which is the final check
	const FVector BottomTraceStart = CurrentCenter - FVector(0.0, 0.0, CapsuleHalfHeight);
	const FVector BottomTraceEnd = BottomTraceStart - FVector(0.0, 0.0, FloorSearchRange);
	DrawDebugLine(World, BottomTraceStart, BottomTraceEnd, FColor::Red, false, 5.0f, 0, 2.0f);
}

void ATeleportPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetActorRotation(FRotator(0.0, GetActorRotation().Yaw, 0.0));
	SetActorScale3D(FVector::OneVector);

	if (PreviewCapsule)
	{
		PreviewCapsule->SetCapsuleRadius(CapsuleRadius);
		PreviewCapsule->SetCapsuleHalfHeight(CapsuleHeight);
		SupportPlane->SetRelativeLocation(FVector(0.0f, 0.0f, -PreviewCapsule->GetScaledCapsuleHalfHeight() - 5.0f));
		if (!LastPosition.Equals(PreviewCapsule->GetComponentLocation(), 0.01f))
		{
			SnapToFloor();
			LastPosition = PreviewCapsule->GetComponentLocation();
		}
	}

	SupportPlane->SetVisibility(bUseSupportPlane);
}

void ATeleportPoint::EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	Super::EditorApplyRotation(FRotator(0.0, DeltaRotation.Yaw, 0.0), bAltDown, bShiftDown, bCtrlDown);
}

void ATeleportPoint::EditorApplyScale(const FVector& DeltaScale,
	const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	// No scaling
}
#endif
