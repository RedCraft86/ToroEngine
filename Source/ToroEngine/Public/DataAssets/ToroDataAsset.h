// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Engine/DataAsset.h"
#include "ToroDataAsset.generated.h"

/**
 * Abstract data asset base for native subclasses with editor-only refresh and validation hooks.
 * Asset references are available to Blueprint, but Blueprint subclasses are not supported.
 */
UCLASS(Abstract, NotBlueprintable, BlueprintType)
class TOROENGINE_API UToroDataAsset : public UDataAsset
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class FAssetRefreshButton;
	friend class FAssetValidateButton;
#endif

public:

	UToroDataAsset() {}

#if WITH_EDITOR
protected:

	/**
	 * Refreshes asset data through a native override; the base implementation does nothing.
	 * Called by the editor toolbar refresh and validation buttons. Other validation paths do not automatically call this hook.
	 * No transaction handling, package dirty marking, or saving is performed by the base implementation.
	 */
	virtual void RefreshData();

	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
