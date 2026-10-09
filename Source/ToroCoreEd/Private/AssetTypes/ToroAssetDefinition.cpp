// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "AssetTypes/ToroAssetDefinition.h"
#include "Objects/ToroDataAsset.h"

FLinearColor UToroAssetDefinition::GetAssetColor() const
{
	return FLinearColor::Green;
}

FText UToroAssetDefinition::GetAssetDisplayName() const
{
	return INVTEXT("Unknown Asset");
}

FText UToroAssetDefinition::GetAssetDescription(const FAssetData& Asset) const
{
	return FText::GetEmpty();
}

TConstArrayView<FAssetCategoryPath> UToroAssetDefinition::GetAssetCategories() const
{
	static const TArray Categories { FAssetCategoryPath(INVTEXT("ToroUtilities")) };
	return Categories;
}

TSoftClassPtr<UObject> UToroAssetDefinition::GetAssetClass() const
{
	return UToroDataAsset::StaticClass();
}
