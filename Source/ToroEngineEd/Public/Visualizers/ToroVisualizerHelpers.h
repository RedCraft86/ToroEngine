// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Internationalization/Text.h"
#include "Math/Color.h"
#include "Math/Vector.h"

class UFont;
class FCanvas;
class FSceneView;
class FPrimitiveDrawInterface;

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
	TOROENGINEED_API void DrawDiscSlice(FPrimitiveDrawInterface* PDI,
		const FVector& Location, const FVector& AxisX, const FVector& AxisY,
		const float MinAngle, const float MaxAngle, const float Radius, const uint8 Sections,
		const FLinearColor& Color, const uint8 DepthPriority, const float Thickness);

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
	TOROENGINEED_API void DrawText(const FSceneView* View, FCanvas* Canvas, const FVector& Location,
		const FText& Text, const uint8 FontScale = 12, const FLinearColor& Color = FLinearColor::White,
		const bool bDrawShadow = false, const UFont* Font = nullptr);
}
