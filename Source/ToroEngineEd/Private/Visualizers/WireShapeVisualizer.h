// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SceneView.h"
#include "Math/Quat.h"
#include "GameFramework/Actor.h"
#include "PrimitiveDrawingUtils.h"
#include "PrimitiveDrawInterface.h"
#include "Visualizers/ToroComponentVisualizer.h"
#include "Visualizers/ToroVisualizerHelpers.h"
#include "Components/WireShapeComponent.h"

class FWireShapeVisualizer final : public FToroComponentVisualizer
{
	virtual void DrawVisualization(const UActorComponent* Component,
		const FSceneView* View, FPrimitiveDrawInterface* PDI) override
	{
		const UWireShapeComponent* ShapeComponent = Cast<UWireShapeComponent>(Component);
		if (!IsValid(ShapeComponent) || !PDI)
		{
			return;
		}

		const AActor* Owner = ShapeComponent->GetOwner();
		const FTransform OwnerTransform = IsValid(Owner) ? Owner->GetActorTransform() : FTransform::Identity;
		const auto TransformLocation = [&OwnerTransform](const FVector& Location)
		{
			return OwnerTransform.TransformPositionNoScale(Location);
		};

		for (const auto& [Name, PointData] : ShapeComponent->WirePoints)
		{
			PDI->DrawPoint(
				TransformLocation(PointData.Location),
				PointData.Color, PointData.Size,
				PointData.bRenderOnTop ? SDPG_Foreground : SDPG_World
			);
		}

		for (const auto& [Name, LineData] : ShapeComponent->WireLines)
		{
			PDI->DrawLine(
				TransformLocation(LineData.Start),
				TransformLocation(LineData.End), LineData.Color,
				LineData.bRenderOnTop ? SDPG_Foreground : SDPG_World,
				LineData.Thickness
			);
		}

		for (const auto& [Name, CircleData] : ShapeComponent->WireCircles)
		{
			const FQuat WorldRotation = OwnerTransform.TransformRotation(CircleData.Rotation.Quaternion());
			DrawCircle(PDI,
				TransformLocation(CircleData.Location),
				WorldRotation.GetAxisX(),
				WorldRotation.GetAxisY(),
				CircleData.Color, CircleData.Radius, FMath::Max<int32>(CircleData.NumOfSides, 3),
				CircleData.bRenderOnTop ? SDPG_Foreground : SDPG_World,
				CircleData.Thickness
			);
		}

		for (const auto& [Name, ArcData] : ShapeComponent->WireArcs)
		{
			const FQuat WorldRotation = OwnerTransform.TransformRotation(ArcData.Rotation.Quaternion());
			ToroVisualizerHelpers::DrawDiscSlice(PDI,
				TransformLocation(ArcData.Location),
				WorldRotation.GetAxisX(),
				WorldRotation.GetAxisY(),
				ArcData.MinAngle, ArcData.MaxAngle,
				ArcData.Radius, ArcData.Sections, ArcData.Color,
				ArcData.bRenderOnTop ? SDPG_Foreground : SDPG_World,
				ArcData.Thickness
			);
		}

		for (const auto& [Name, BoxData] : ShapeComponent->WireBoxes)
		{
			const FQuat WorldRotation = OwnerTransform.TransformRotation(BoxData.Rotation.Quaternion());
			DrawOrientedWireBox(PDI,
				TransformLocation(BoxData.Location),
				WorldRotation.GetAxisX(),
				WorldRotation.GetAxisY(),
				WorldRotation.GetAxisZ(),
				BoxData.Extents, BoxData.Color,
				BoxData.bRenderOnTop ? SDPG_Foreground : SDPG_World,
				BoxData.Thickness
			);
		}

		for (const auto& [Name, SphereData] : ShapeComponent->WireSpheres)
		{
			DrawWireSphere(PDI,
				TransformLocation(SphereData.Location),
				SphereData.Color, SphereData.Radius, FMath::Max<int32>(SphereData.NumOfSides, 3),
				SphereData.bRenderOnTop ? SDPG_Foreground : SDPG_World,
				SphereData.Thickness
			);
		}

		for (const auto& [Name, CylinderData] : ShapeComponent->WireCylinders)
		{
			const FQuat WorldRotation = OwnerTransform.TransformRotation(CylinderData.Rotation.Quaternion());
			DrawWireCylinder(PDI,
				TransformLocation(CylinderData.Location),
				WorldRotation.GetAxisX(),
				WorldRotation.GetAxisY(),
				WorldRotation.GetAxisZ(),
				CylinderData.Color, CylinderData.Radius,
				CylinderData.HalfHeight, FMath::Max<int32>(CylinderData.NumOfSides, 3),
				CylinderData.bRenderOnTop ? SDPG_Foreground : SDPG_World,
				CylinderData.Thickness
			);
		}

		for (const auto& [Name, CapsuleData] : ShapeComponent->WireCapsules)
		{
			const FQuat WorldRotation = OwnerTransform.TransformRotation(CapsuleData.Rotation.Quaternion());
			DrawWireCapsule(PDI,
				TransformLocation(CapsuleData.Location),
				WorldRotation.GetAxisX(),
				WorldRotation.GetAxisY(),
				WorldRotation.GetAxisZ(),
				CapsuleData.Color, CapsuleData.Radius,
				CapsuleData.HalfHeight, FMath::Max<int32>(CapsuleData.NumOfSides, 4),
				CapsuleData.bRenderOnTop ? SDPG_Foreground : SDPG_World,
				CapsuleData.Thickness
			);
		}

		for (const auto& [Name, NavPathData] : ShapeComponent->WireNavPaths)
		{
			const int32 NumPoints = NavPathData.PathPoints.Num();
			for (int32 i = 0; i < NumPoints; i++)
			{
				PDI->DrawPoint(
					NavPathData.PathPoints[i],
					NavPathData.PointColor, NavPathData.PointSize,
					NavPathData.bRenderOnTop ? SDPG_Foreground : SDPG_World
				);

			}

			for (int32 i = 0; i < NumPoints - 1; i++)
			{
				const FVector Current = NavPathData.PathPoints[i];
				const FVector Next = NavPathData.PathPoints[i + 1];
				if (!Current.Equals(Next, 1.0f))
				{
					PDI->DrawLine(
						Current, Next, NavPathData.Color,
						NavPathData.bRenderOnTop ? SDPG_Foreground : SDPG_World,
						NavPathData.LineThickness
					);
				}
			}
		}
	}

	virtual void DrawVisualizationHUD(const UActorComponent* Component,
		const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override
	{
		const UWireShapeComponent* ShapeComponent = Cast<UWireShapeComponent>(Component);
		if (!IsValid(ShapeComponent) || !View || !Canvas)
		{
			return;
		}

		const AActor* Owner = ShapeComponent->GetOwner();
		const FTransform OwnerTransform = IsValid(Owner) ? Owner->GetActorTransform() : FTransform::Identity;
		const auto TransformLocation = [&OwnerTransform](const FVector& Location)
		{
			return OwnerTransform.TransformPositionNoScale(Location);
		};

		for (const auto &[Name, StringData] : ShapeComponent->WireStrings)
		{
			ToroVisualizerHelpers::DrawText(
				View, Canvas, TransformLocation(StringData.Location),
				StringData.Text.IsEmpty() ? FText::FromName(Name) : FText::FromString(StringData.Text),
				StringData.FontScale, StringData.Color, StringData.bDrawShadow
			);
		}

		const FVector CameraLocation = View->ViewMatrices.GetViewOrigin();
		const FVector CameraForward = View->GetViewDirection();
		for (const auto &[Name, NavPathData] : ShapeComponent->WireNavPaths)
		{
			const FVector* LabelPoint = nullptr;
			double BestWeight = 0.0;
			for (const FVector& Point : NavPathData.PathPoints)
			{
				const FVector ToPoint = Point - CameraLocation;
				const double Weight = 1.0
					+ FMath::Max(0.0, FVector::DotProduct(ToPoint.GetSafeNormal(), CameraForward)) * 0.5
					+ (1.0 - FMath::Clamp(ToPoint.SizeSquared() / 1000000.0, 0.0, 1.0)) * 0.5;

				if (Weight > BestWeight)
				{
					BestWeight = Weight;
					LabelPoint = &Point;
				}
			}

			if (LabelPoint)
			{
				ToroVisualizerHelpers::DrawText(View, Canvas,
					*LabelPoint - FVector(0.0f, 0.0f, 15.0f),
					FText::FromName(Name), 14, NavPathData.Color, true
				);
			}
		}
	}
};
