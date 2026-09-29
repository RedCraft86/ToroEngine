// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Engine/DataAsset.h"
#include "ToroDataAsset.generated.h"

/**
 * Base for native data assets with editor refresh and asset-local validation actions.
 * Native subclasses override RefreshData to rebuild derived data and IsDataValid to add checks.
 * The Validate action displays this asset's validation issues and result in the Asset Check log.
 */
UCLASS(Abstract, NotBlueprintable, BlueprintType, PrioritizeCategories = (Asset))
class TOROCORE_API UToroDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UToroDataAsset();

#if WITH_EDITOR
protected:

	/**
	 * Rebuilds derived asset data when Refresh is clicked in the editor.
	 * The base implementation does nothing; native subclasses provide the behavior.
	 */
	UFUNCTION(CallInEditor, Category = Asset, DisplayName = "Refresh")
	virtual void RefreshData() {}

	/**
	 * Runs this asset's IsDataValid implementation and displays its issues and result in the Asset Check log.
	 * Additional validators registered with the editor validator subsystem are not invoked.
	 */
	UFUNCTION(CallInEditor, Category = Asset, DisplayName = "Validate")
	virtual void ValidateData() const;

	/**
	 * Extension point for native subclasses to add validation checks and report issues through Context.
	 * The base implementation forwards to Super and returns its validation result.
	 */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
