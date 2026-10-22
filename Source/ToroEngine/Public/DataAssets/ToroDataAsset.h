// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Engine/DataAsset.h"
#include "ToroDataAsset.generated.h"

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

	virtual void RefreshData();
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
