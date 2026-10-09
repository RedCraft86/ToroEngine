// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToroCoreEd.h"
#include "UnrealEdGlobals.h"
#include "ComponentVisualizer.h"
#include "Editor/UnrealEdEngine.h"
#include "ToroVisualizerHelpers.h"

/**
 * Component-visualizer base with tracked registration and optional empty drawing callbacks.
 */
class TOROCOREED_API FToroComponentVisualizer : public FComponentVisualizer
{
	static inline TSet<FName> ComponentNames = {};

public:

	/**
	 * Registers a visualizer with GUnrealEd and calls its OnRegister callback.
	 * Does nothing when GUnrealEd is unavailable; duplicate tracked registrations are ignored.
	 * @tparam Component Reflected component class derived from UActorComponent.
	 * @tparam Visualizer Default-constructible type derived from FToroComponentVisualizer.
	 */
	template<typename Component, typename Visualizer>
	static void Register()
	{
		static_assert(TIsDerivedFrom<Component, UActorComponent>::Value,
			"Component must be a UClass derived from UActorComponent");

		static_assert(TIsDerivedFrom<Visualizer, FToroComponentVisualizer>::Value,
			"Visualizer must derive from FToroComponentVisualizer");

		const FName CompName = Component::StaticClass()->GetFName();
		if (GUnrealEd && !CompName.IsNone())
		{
			if (ComponentNames.Contains(CompName))
			{
				UE_LOG(LogToroCoreEd, Warning,
					TEXT("FToroComponentVisualizer: Attempting to register multiple Component Visualizers for %s"), *CompName.ToString()
				);
				return;
			}

			ComponentNames.Add(CompName);
			const TSharedPtr<FComponentVisualizer> VisInstance = MakeShared<Visualizer>();
			GUnrealEd->RegisterComponentVisualizer(CompName, VisInstance);
			VisInstance->OnRegister();

			UE_LOG(LogToroCoreEd, Display, TEXT("FToroComponentVisualizer: Registering Component Visualizer for %s"), *CompName.ToString());
		}
	}

	/**
	 * Removes tracked visualizers when GUnrealEd is available and always clears local tracking.
	 */
	static void UnregisterAll()
	{
		if (GUnrealEd && !ComponentNames.IsEmpty())
		{
			for (const FName& Vis : ComponentNames)
			{
				GUnrealEd->UnregisterComponentVisualizer(Vis);
			}

			UE_LOG(LogToroCoreEd, Display, TEXT("FToroComponentVisualizer: Unregistered Component Visualizers"));
		}

		ComponentNames.Empty();
	}

protected:

	virtual void DrawVisualization(const UActorComponent* Comp, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawVisualizationHUD(const UActorComponent* Comp, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
};
