// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ItemFragment.h"
#include "DataAssets/ToroDataAsset.h"
#include "StructUtils/InstancedStruct.h"
#include "ItemDefinition.generated.h"

UCLASS(NotBlueprintable, BlueprintType)
class TOROSYSTEMS_API UItemDefinition final : public UToroDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	float Priority = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (ClampMin = 1, UIMin = 1))
	uint8 ItemLimit = 1;

	UPROPERTY(EditAnywhere, Category = Item, meta = (ExcludeBaseStruct))
	TInstancedStruct<FItemFragment> Fragment;

	uint8 GetItemLimit() const;
	float GetPriority() const;
	bool IsDefinitionValid() const;

#if WITH_EDITOR
private:
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
