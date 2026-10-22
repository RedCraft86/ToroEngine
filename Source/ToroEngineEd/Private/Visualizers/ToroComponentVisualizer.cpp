// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Visualizers/ToroComponentVisualizer.h"

void FToroComponentVisualizer::DrawVisualization(const UActorComponent* Component,
	const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	// Not pure-virtual since this might not always be needed.
}

void FToroComponentVisualizer::DrawVisualizationHUD(const UActorComponent* Component,
	const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	// Not pure-virtual since this might not always be needed.
}
