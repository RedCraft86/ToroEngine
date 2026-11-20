// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroInventoryEntryWidget.h"
#include "ToroInventoryItemEntry.generated.h"

class UImage;
class UTextBlock;

UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API UToroInventoryItemEntry final : public UToroInventoryEntryWidget
{
	GENERATED_BODY()

public:

	UToroInventoryItemEntry();

	void RefreshEquipmentMarker() const;

private:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidget> EquipMarker;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> AmountText;

	virtual bool InitEntry() override;
	virtual void NativeConstruct() override;
};
