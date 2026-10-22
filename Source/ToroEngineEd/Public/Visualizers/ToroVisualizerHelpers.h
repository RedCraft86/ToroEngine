// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SceneView.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "PrimitiveDrawingUtils.h"
#include "PrimitiveDrawInterface.h"
#include "Internationalization/Text.h"
#include "Fonts/SlateFontInfo.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Math/Color.h"
#include "Math/Vector.h"
#include "Math/Vector2D.h"
#include "Math/UnrealMathUtility.h"

/** Drawing utilities for component-visualizer viewport geometry and projected canvas labels. */
namespace ToroVisualizerHelpers
{
	/**
	 * Draws an outlined sector using straight arc segments and two radial edges.
	 * Axes are used without normalization; orthonormal axes produce a circular sector.
	 * No drawing occurs for a null PDI, a zero-length axis, nearly equal angles,
	 * a non-positive radius or thickness, or zero sections.
	 * @param PDI Primitive drawing interface receiving the translucent lines.
	 * @param Location Sector center in world space.
	 * @param AxisX Basis direction corresponding to zero degrees.
	 * @param AxisY Basis direction corresponding to positive ninety degrees.
	 * @param MinAngle Starting angle in degrees.
	 * @param MaxAngle Ending angle in degrees; values below MinAngle reverse the sweep.
	 * @param Radius Radius in world units when the basis axes are unit length.
	 * @param Sections Number of straight segments approximating the arc, from 1 to 255.
	 * @param Color Line color, including opacity.
	 * @param DepthPriority Scene depth-priority group passed to the drawing interface.
	 * @param Thickness Positive line thickness passed to the drawing interface.
	 */
	TOROENGINEED_API inline void DrawDiscSlice(FPrimitiveDrawInterface* PDI,
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

	/**
	 * Draws a canvas label anchored at the projected world position.
	 * Projected coordinates are divided by the canvas DPI scale unless that scale is nearly zero.
	 * Projection success does not guarantee that the point lies within the viewport bounds.
	 * No drawing occurs if the view, canvas, or resolved font is null, or projection fails.
	 * @param View Scene view supplying the viewport rectangle and world-to-clip transform.
	 * @param Canvas Canvas receiving the text item.
	 * @param Location World-space position at which to anchor the label.
	 * @param Text Label content.
	 * @param FontScale Font size supplied to FSlateFontInfo; zero uses the default size of 12.
	 * @param Color Text color, including opacity.
	 * @param bDrawShadow Whether to draw a black text shadow.
	 * @param Font Optional font override; null uses the engine small font when available.
	 */
	TOROENGINEED_API inline void DrawText(const FSceneView* View, FCanvas* Canvas, const FVector& Location,
		const FText& Text, const uint8 FontScale = 12, const FLinearColor& Color = FLinearColor::White,
		const bool bDrawShadow = false, const UFont* Font = nullptr)
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
}
