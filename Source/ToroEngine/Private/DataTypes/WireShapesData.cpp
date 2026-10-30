// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DataTypes/WireShapesData.h"
#include "Libraries/ToroCameraLibrary.h"
#include "Utilities/ToroUtilities.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

void FWireNavPathData::FindNearestToCamera()
{
	LabelPoint.Reset();
	if (PathPoints.IsEmpty())
	{
		return;
	}

	constexpr float DotWeight = 0.5f;
	constexpr float DistWeight = 1.0f - DotWeight;

	const TOptional<FTransform> Camera = UToroCameraLibrary::TryGetViewTransform(ToroEngine::GetWorld(nullptr, true));
	if (!Camera.IsSet())
	{
		LabelPoint = PathPoints[0];
		return;
	}

	float BestWeight = 0.0f;
	const FVector CameraFwd = Camera.GetValue().GetRotation().GetForwardVector();
	const FVector CameraLoc = Camera.GetValue().GetTranslation();
	for (const FVector& Point : PathPoints)
	{
		float Weight = 1.0f;

		const FVector ToPoint = Point - CameraLoc;
		Weight += FMath::Max(0.0f, FVector::DotProduct(ToPoint.GetSafeNormal(), CameraFwd)) * DotWeight;

		const double DistSq = ToPoint.SizeSquared();
		Weight += (1.0f - FMath::Clamp(DistSq / FMath::Square(1000.0f), 0.0f, 1.0f)) * DistWeight;
		if (Weight > BestWeight)
		{
			BestWeight = Weight;
			LabelPoint = Point;
		}
	}
}

void FWireNavPathData::GeneratePathPoints(AActor* Owner)
{
	PathPoints.Reset();
	LabelPoint.Reset();
	const int32 NumTargets = Targets.Num();
	if (NumTargets < 2 || !IsValid(Owner) || !Owner->GetWorld())
	{
		return;
	}

	PathPoints.Reserve(NumTargets);
	const FTransform OwnerTransform = Owner->GetActorTransform();
	for (int32 i = 0; i < NumTargets - 1; i++)
	{
		const FVector Current = OwnerTransform.TransformPositionNoScale(Targets[i]);
		const FVector Next = OwnerTransform.TransformPositionNoScale(Targets[i + 1]);
		const UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(Owner, Current, Next);
		if (!IsValid(NavPath) || !NavPath->IsValid() || NavPath->PathPoints.IsEmpty())
		{
			if (!bAllowPartialPath)
			{
				PathPoints.Reset();
			}
			return;
		}

		if (NavPath->IsPartial() && !bAllowPartialPath)
		{
			PathPoints.Reset();
			return;
		}

		for (const FVector& Point : NavPath->PathPoints)
		{
			if (PathPoints.IsEmpty() || !PathPoints.Last().Equals(Point))
			{
				PathPoints.Add(Point);
			}
		}

		if (NavPath->IsPartial())
		{
			return;
		}
	}
}
