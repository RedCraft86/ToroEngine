// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroInventoryEntryWidget.h"
#include "ToroInventoryArchiveEntry.generated.h"

class UTextBlock;

UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API UToroInventoryArchiveEntry final : public UToroInventoryEntryWidget
{
	GENERATED_BODY()

public:

	UToroInventoryArchiveEntry();

private:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

	virtual bool InitEntry() override;
	virtual void NativeConstruct() override;
};
