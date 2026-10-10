// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "DetailsPanel/ToroActorCustomizations.h"

void FToroActorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
}

void FToroVolumeCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
}

void FToroCharacterCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
}

TArray<FString> FToroCharacterCustomization::GetDefaultCategories()
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
