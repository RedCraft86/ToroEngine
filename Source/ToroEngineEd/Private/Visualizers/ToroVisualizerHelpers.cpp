// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Visualizers/ToroVisualizerHelpers.h"
#include "PrimitiveDrawInterface.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector2D.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Fonts/SlateFontInfo.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "SceneView.h"

void ToroVisualizerHelpers::DrawDiscSlice(FPrimitiveDrawInterface* PDI,
	const FVector& Location, const FVector& AxisX, const FVector& AxisY,
	const float MinAngle, const float MaxAngle, const float Radius, const uint8 Sections,
	const FLinearColor& Color, const uint8 DepthPriority, const float Thickness)
{
	if (!PDI
		|| AxisX.SizeSquared() <= 0.0f
		|| AxisY.SizeSquared() <= 0.0f
		|| FMath::IsNearlyEqual(MinAngle, MaxAngle)
		|| Radius <= 0.0f
		|| Sections <= 0
		|| Thickness <= 0.0f)
	{
		return;
	}

	constexpr float DegToRad = UE_PI / 180.0f;
	const float AngleStep = (MaxAngle - MinAngle) / static_cast<float>(Sections);
	float CurrentAngle = MinAngle;

	FVector LastVertex = Location + Radius * (FMath::Cos(CurrentAngle * DegToRad) * AxisX + FMath::Sin(CurrentAngle * DegToRad) * AxisY);
	CurrentAngle += AngleStep;

	PDI->DrawTranslucentLine(Location, LastVertex, Color, DepthPriority, Thickness);
	for (uint8 i = 0; i < Sections; i++)
	{
		FVector ThisVertex = Location + Radius * (FMath::Cos(CurrentAngle * DegToRad) * AxisX + FMath::Sin(CurrentAngle * DegToRad) * AxisY);
		PDI->DrawTranslucentLine(LastVertex, ThisVertex, Color, DepthPriority, Thickness);
		CurrentAngle += AngleStep;
		LastVertex = ThisVertex;
	}
	PDI->DrawTranslucentLine(Location, LastVertex, Color, DepthPriority, Thickness);
}

void ToroVisualizerHelpers::DrawText(const FSceneView* View, FCanvas* Canvas, const FVector& Location,
	const FText& Text, const uint8 FontScale, const FLinearColor& Color,
	const bool bDrawShadow, const UFont* Font)
{
	if (!View || !Canvas)
	{
		return;
	}

	const UFont* DrawFont = Font ? Font : (GEngine ? GEngine->GetSmallFont() : nullptr);
	if (!DrawFont)
	{
		return;
	}

	FVector2D ScreenPos;
	if (FSceneView::ProjectWorldToScreen(Location, View->UnscaledViewRect, View->ViewMatrices.GetWorldToClip(), ScreenPos))
	{
		const float DPIScale = Canvas->GetDPIScale();
		if (!FMath::IsNearlyZero(DPIScale))
		{
			ScreenPos /= DPIScale;
		}

		FCanvasTextItem TextItem = FCanvasTextItem(ScreenPos, Text, FSlateFontInfo(
			DrawFont, FontScale > 0 ? static_cast<float>(FontScale) : 12.0f, NAME_None
		), Color);

		if (bDrawShadow)
		{
			TextItem.EnableShadow(FColor::Black);
		}
		else
		{
			TextItem.DisableShadow();
		}

		Canvas->DrawItem(TextItem);
	}
}
