// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "AssetDefinitionDefault.h"
#include "ToroAssetDefinition.generated.h"

/**
 * Shared editor asset-definition base for Toro data assets.
 * Defaults to UToroDataAsset, the ToroUtilities category, a green asset color, and an empty description.
 * Thumbnail and icon overrides defer to the engine's class-brush fallback.
 * Concrete definitions should override the supported asset class and replace the Unknown Asset display name.
 */
UCLASS(Abstract)
class TOROENGINEED_API UToroAssetDefinition : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:

	UToroAssetDefinition() {}

protected:

	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;

	virtual FLinearColor GetAssetColor() const override;
	virtual FText GetAssetDisplayName() const override;
	virtual FText GetAssetDescription(const FAssetData& Asset) const override;
	virtual const FSlateBrush* GetThumbnailBrush(const FAssetData& InAssetData, const FName InClassName) const override;
	virtual const FSlateBrush* GetIconBrush(const FAssetData& InAssetData, const FName InClassName) const override;
};
