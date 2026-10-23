// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroEngineEd.h"
#include "UnrealEdGlobals.h"
#include "Containers/Set.h"
#include "UObject/Class.h"
#include "UObject/NameTypes.h"
#include "ComponentVisualizer.h"
#include "Editor/UnrealEdEngine.h"
#include "Templates/UnrealTypeTraits.h"
#include "Templates/SharedPointer.h"
#include "ToroVisualizerHelpers.h"

/**
 * Base for editor component visualizers with shared registration and shutdown cleanup.
 * Derived visualizers can implement viewport drawing, HUD drawing, or both; the base drawing hooks do nothing.
 * Register on the editor game thread after GUnrealEd is available and call UnregisterAll() during module shutdown.
 */
class TOROENGINEED_API FToroComponentVisualizer : public FComponentVisualizer
{
	/** Component-class names registered through this helper and retained for shutdown cleanup. */
	static inline TSet<FName> RegisteredEntries = {};

public:

	/**
	 * Immediately registers a visualizer for the exact component-class name and invokes its OnRegister() hook.
	 * Existing registrations for that name are rejected; a visualizer inherited from a parent class does not conflict.
	 * @tparam Visualizer Concrete FToroComponentVisualizer subclass constructible without arguments by MakeShared().
	 * @tparam Component Reflected UActorComponent subclass exposing StaticClass().
	 * @return Registered component-class name, or NAME_None if the editor is unavailable,
	 *         the name is invalid, or the exact class already has a registered visualizer.
	 *         Failure does not schedule a retry; the caller must register again when the editor is ready.
	 */
	template<typename Visualizer, typename Component>
	static FName Register()
	{
		static_assert(TIsDerivedFrom<Visualizer, FToroComponentVisualizer>::Value,
			"Visualizer must derive from FToroComponentVisualizer");

		static_assert(TIsDerivedFrom<Component, UActorComponent>::Value,
			"Component must be a UClass derived from UActorComponent");

		if (!GUnrealEd)
		{
			UE_LOG(LogToroEngineEd, Warning, TEXT("FToroComponentVisualizer: Attempting to register while GUnrealEd pointer is null"))
			return NAME_None;
		}

		const FName EntryName = Component::StaticClass()->GetFName();
		if (EntryName.IsNone())
		{
			UE_LOG(LogToroEngineEd, Warning, TEXT("FToroComponentVisualizer: Attempting to register with an invalid component"))
			return NAME_None;
		}

		if (RegisteredEntries.Contains(EntryName) || GUnrealEd->FindComponentVisualizer(EntryName).IsValid())
		{
			UE_LOG(LogToroEngineEd, Warning,
				TEXT("FToroComponentVisualizer: Attempting to register multiple visualizers for %s"), *EntryName.ToString()
			);
			return NAME_None;
		}

		RegisteredEntries.Add(EntryName);
		const TSharedPtr<FComponentVisualizer> VisInstance = MakeShared<Visualizer>();
		GUnrealEd->RegisterComponentVisualizer(EntryName, VisInstance);
		VisInstance->OnRegister();

		UE_LOG(LogToroEngineEd, Display, TEXT("FToroComponentVisualizer: Registered visualizer for %s"), *EntryName.ToString());
		return EntryName;
	}

	/**
	 * Unregisters tracked component-class names while GUnrealEd is available, then clears local tracking.
	 * @note Automatically called by ToroEngineEd -> Module Shutdown.
	 */
	static void UnregisterAll()
	{
		if (GUnrealEd && !RegisteredEntries.IsEmpty())
		{
			for (const FName& Entry : RegisteredEntries)
			{
				GUnrealEd->UnregisterComponentVisualizer(Entry);
			}

			UE_LOG(LogToroEngineEd, Log, TEXT("FToroComponentVisualizer: Unregistered all visualizers"));
		}

		RegisteredEntries.Empty();
	}

protected:

	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
};
