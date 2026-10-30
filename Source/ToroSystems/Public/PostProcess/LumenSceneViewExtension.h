// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SceneViewExtension.h"

/**
 * Tracks whether the most recently processed view family for a world has an eligible view configured for Lumen GI.
 * Checks platform support, the Lumen GI console variable, and view-family show flags;
 * this cached result does not confirm that Lumen actually rendered.
 */
class FLumenSceneViewExtension final : public FWorldSceneViewExtension
{
public:

	FLumenSceneViewExtension(const FAutoRegister& AutoRegister, UWorld* World);

	/**
	 * Reports whether the most recently processed view family contains an eligible view configured for Lumen GI.
	 * False before the first family is processed. Scene captures, cube captures, reflection captures,
	 * planar reflections, and virtual-texture views are excluded; a family containing only excluded
	 * views resets the result to false. The result persists until another family is processed.
	 */
	[[nodiscard]] FORCEINLINE bool IsUsingLumen() const { return bUsingLumen; }

private:

	/** Cached result for the latest processed view family; initially false and reset before each family is checked. */
	bool bUsingLumen;

	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override;
};
