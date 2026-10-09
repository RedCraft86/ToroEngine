// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "ComponentVis/ToroComponentVisualizer.h"

void FToroComponentVisualizer::DrawVisualization(const UActorComponent* Comp, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	// Not pure-virtual since this might not always be needed.
}

void FToroComponentVisualizer::DrawVisualizationHUD(const UActorComponent* Comp, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	// Not pure-virtual since this might not always be needed.
}
