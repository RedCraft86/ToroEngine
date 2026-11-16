// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Actors/SplineBarrier.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "HAL/IConsoleManager.h"

static FName InvisibleWall_NAME("InvisibleWall");

static TAutoConsoleVariable<int32> CVarShowInvisibleWalls(
	TEXT("r.Debug.ShowInvisibleWalls"),
	0,
	TEXT("Show the invisible wall primivives produced by ASplineBarrier. 0: off, 1: on"),
	ECVF_Default
);

ASplineBarrier::ASplineBarrier()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	RootSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RootSpline"));
	RootSpline->SetMobility(EComponentMobility::Static);
	SetRootComponent(RootSpline);
#if WITH_EDITORONLY_DATA
	RootSpline->ScaleVisualizationWidth = 5.0f;
	RootSpline->bShouldVisualizeScale = true;
#endif

#if WITH_EDITORONLY_DATA
	bRunConstructionScriptOnDrag = true;
	bIsSpatiallyLoaded = false; // Stop World Partition from unloading this
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLODs
	SetCanBeDamaged(false);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void ASplineBarrier::OnVisualCVarChanged(IConsoleVariable* Variable)
{
	for (UBoxComponent* Section : WallSections)
	{
		if (IsValid(Section))
		{
			Section->SetHiddenInGame(Variable->GetInt() == 0);
		}
	}
}

void ASplineBarrier::BeginPlay()
{
	Super::BeginPlay();
	CVarShowInvisibleWalls->OnChangedDelegate().AddUObject(this, &ASplineBarrier::OnVisualCVarChanged);
	OnVisualCVarChanged(CVarShowInvisibleWalls.AsVariable());
}

void ASplineBarrier::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CVarShowInvisibleWalls->OnChangedDelegate().RemoveAll(this);
	Super::EndPlay(EndPlayReason);
}

void ASplineBarrier::OnConstruction(const FTransform& Transform)
{
	constexpr float BaseThickness = 10.0f;
	constexpr float BaseHeight = 200.0f;

	Super::OnConstruction(Transform);
	for (UBoxComponent* Section : WallSections)
	{
		if (IsValid(Section))
		{
			Section->DestroyComponent();
		}
	}
	WallSections.Reset();

	const int32 PointCount = RootSpline->GetNumberOfSplinePoints();
	for (int32 i = 0; i < PointCount; i++)
	{
		RootSpline->SetSplinePointType(i, ESplinePointType::Linear, false);
	}
	RootSpline->UpdateSpline();

	const int32 SectionCount = RootSpline->IsClosedLoop() ? PointCount : PointCount - 1;
	for (int32 i = 0; i < SectionCount; i++)
	{
		if (SkipIndices.Contains(i))
		{
			continue;
		}

		const FVector Start = RootSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
		const FVector End = RootSpline->GetLocationAtSplinePoint((i + 1) % PointCount, ESplineCoordinateSpace::Local);
		const FVector StartScale = RootSpline->GetScaleAtSplinePoint(i);
		const FVector EndScale = RootSpline->GetScaleAtSplinePoint((i + 1) % PointCount);

		const FVector Delta = End - Start;
		const float Length = Delta.Size();
		if (Length <= UE_SMALL_NUMBER)
		{
			continue;
		}

		UBoxComponent* Section = NewObject<UBoxComponent>(this);
		Section->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		Section->SetMobility(EComponentMobility::Static);
		Section->SetupAttachment(RootSpline);
		Section->SetLineThickness(2.0f);

		const float Thickness = FMath::Max(1.0f, BaseThickness * FMath::Max(StartScale.Y, EndScale.Y));
		const float Height = FMath::Max(1.0f, BaseHeight * FMath::Max(StartScale.Z, EndScale.Z));
		const FRotator Rotation = Delta.Rotation();
		const FVector Offset = Rotation.RotateVector(FVector::UpVector) * Height * 0.5f;

		Section->SetRelativeLocation((Start + End) * 0.5f + Offset);
		Section->SetBoxExtent(FVector(Length * 0.5f, Thickness * 0.5f, Height * 0.5f));
		Section->SetRelativeRotation(Rotation);

		Section->SetCollisionProfileName(InvisibleWall_NAME);
		Section->SetHiddenInGame(CVarShowInvisibleWalls.GetValueOnGameThread() == 0);

		Section->RegisterComponent();
		WallSections.Add(Section);
	}
}
