// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "SceneView.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "PrimitiveDrawInterface.h"
#include "PrimitiveDrawingUtils.h"
#include "Engine/Engine.h"

namespace ToroVisualizerHelpers
{
	/**
	 * Draws a segmented sector outline with an arc and radial lines from its center to both endpoints.
	 * Radial lines are also drawn for full-circle angle ranges. Orthonormal X and Y axes produce
	 * a circular arc; the axes are used as supplied without normalization.
	 * @param PDI Primitive drawing interface; null draws nothing.
	 * @param Base World-space center of the sector.
	 * @param X Basis vector pointing toward the zero-degree angle.
	 * @param Y Basis vector pointing toward the 90-degree angle.
	 * @param MinAngle Starting angle in degrees.
	 * @param MaxAngle Ending angle in degrees; values below MinAngle draw in the reverse direction.
	 * @param Radius Radius in world units when the basis vectors are normalized.
	 * @param Sections Number of straight segments approximating the arc; nonpositive values draw nothing.
	 * @param Color Color passed to each translucent line.
	 * @param DepthPriority Scene depth-priority group used for the lines.
	 * @param Thickness Line thickness passed to the primitive drawing interface.
	 */
	inline void DrawArcWithThickness(FPrimitiveDrawInterface* PDI, const FVector& Base, const FVector& X, const FVector& Y,
		const float MinAngle, const float MaxAngle, const float Radius, const int Sections,
		const FLinearColor& Color, const uint8 DepthPriority, const float Thickness)
	{
		if (!PDI || Sections <= 0)
		{
			return;
		}

		constexpr float ToRadians = PI / 180.0f;

		const float AngleStep = (MaxAngle - MinAngle) / Sections;
		float Angle = MinAngle;

		FVector LastVert = Base + Radius * (FMath::Cos(Angle * ToRadians) * X + FMath::Sin(Angle * ToRadians) * Y);
		Angle += AngleStep;

		for (int32 i = 0; i < Sections; i++)
		{
			FVector ThisVert = Base + Radius * (FMath::Cos(Angle * ToRadians) * X + FMath::Sin(Angle * ToRadians) * Y);
			if (i == 0)
			{
				PDI->DrawTranslucentLine(Base, LastVert, Color, DepthPriority, Thickness);
			}
			if (i == Sections - 1)
			{
				PDI->DrawTranslucentLine(Base, ThisVert, Color, DepthPriority, Thickness);
			}

			PDI->DrawTranslucentLine(LastVert, ThisVert, Color, DepthPriority, Thickness);
			LastVert = ThisVert;
			Angle += AngleStep;
		}
	}

	/**
	 * Draws text anchored at a projected world location using the view's constrained, unscaled rectangle.
	 * Converts projected pixels to canvas coordinates using its nonzero DPI scale. Points behind
	 * the camera are skipped; projection does not test scene occlusion or viewport bounds.
	 * Missing view, canvas, or resolved font inputs draw nothing.
	 * @param View Scene view supplying the projection matrix and view rectangle.
	 * @param Canvas Canvas receiving the text item.
	 * @param Location World-space anchor for the text's top-left position.
	 * @param Text Text to draw.
	 * @param FontScale Slate font size, defaulting to 12; zero also uses 12.
	 * @param Color Text color.
	 * @param bDrawShadow Enables a black text shadow when true.
	 * @param Font Explicit font; null uses GEngine's small font when the engine is available.
	 */
	inline void DrawText(const FSceneView* View, FCanvas* Canvas, const FVector& Location,
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
				DrawFont,
				FontScale > 0 ? static_cast<float>(FontScale) : 12.0f,
				NAME_None
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
