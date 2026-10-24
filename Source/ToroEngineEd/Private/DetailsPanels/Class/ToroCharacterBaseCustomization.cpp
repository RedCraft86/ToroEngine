// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DetailsPanels/Class/ToroCharacterBaseCustomization.h"

TArray<FString> FToroCharacterBaseCustomization::GetDefaultCategories()
{
	return TArray<FString>{
		TEXT("Settings"),
		TEXT("Tools"),
		TEXT("Lighting"),
		TEXT("Rendering"),
		// TEXT("HLOD"),
		// TEXT("Mobile"),
		// TEXT("RayTracing"),
		TEXT("Pawn"),
		// TEXT("Replication"),
		// TEXT("Networking"),
		TEXT("Input"),
		TEXT("Actor"),
		TEXT("Optimization"),
		TEXT("LevelOfDetail"),
		TEXT("MaterialParameters"),
		TEXT("TextureStreaming"),
		TEXT("WorldPartition"),
		TEXT("LevelInstance"),
		TEXT("DataLayers")
	};
}
