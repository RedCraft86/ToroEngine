// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "AssetDefinitionDefault.h"
#include "ToroAssetDefinition.generated.h"

/**
 * Base asset definition for UToroDataAsset in the ToroUtilities category.
 * Defaults to a green color, an Unknown Asset label, and an empty description;
 * derived definitions can override these values for their asset type.
 */
UCLASS(Abstract)
class TOROCOREED_API UToroAssetDefinition : public UAssetDefinitionDefault
{
	GENERATED_BODY()

protected:

	virtual FLinearColor GetAssetColor() const override;
	virtual FText GetAssetDisplayName() const override;
	virtual FText GetAssetDescription(const FAssetData& Asset) const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
};
