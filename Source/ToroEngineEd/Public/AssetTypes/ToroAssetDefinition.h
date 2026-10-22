// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "AssetDefinitionDefault.h"
#include "ToroAssetDefinition.generated.h"

UCLASS(Abstract)
class TOROENGINEED_API UToroAssetDefinition : public UAssetDefinitionDefault
{
	GENERATED_BODY()

protected:

	virtual FLinearColor GetAssetColor() const override;
	virtual FText GetAssetDisplayName() const override;
	virtual FText GetAssetDescription(const FAssetData& Asset) const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
};
