// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DetailsPanels/Class/ToroCharacterCustomization.h"
#include "DetailLayoutBuilder.h"
#include "EditorCategoryUtils.h"
#include "Containers/Set.h"
#include "UObject/Class.h"

void FToroCharacterCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TSet<FName> AllowedCategories {
		TEXT("Transform"),
		TEXT("TransformCommon"),
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

	TArray<FString> PrioritizedCategories;
	CustomizingClass->GetPrioritizeCategories(PrioritizedCategories);
	for (const FString& Category : PrioritizedCategories)
	{
		AllowedCategories.Add(*Category);
	}

	TArray<FString> ShownCategories;
	FEditorCategoryUtils::GetClassShowCategories(CustomizingClass, ShownCategories);
	for (const FString& Category : ShownCategories)
	{
		AllowedCategories.Add(*Category);
	}

	TArray<FName> Categories;
	DetailBuilder.GetCategoryNames(Categories);
	for (const FName Category : Categories)
	{
		if (!AllowedCategories.Contains(Category)
			|| FEditorCategoryUtils::IsCategoryHiddenFromClass(CustomizingClass, Category.ToString()))
		{
			DetailBuilder.HideCategory(Category);
		}
	}

	FToroActorCustomization::CustomizeDetails(DetailBuilder);
}
