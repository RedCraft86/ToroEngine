// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CommonButtonBase.h"
#include "CommonButton.generated.h"

class UNamedSlot;

/**
 * CommonUI button with a single named content slot for arbitrary child widgets.
 * The assigned button style controls padding, dimensions, and state brushes.
 */
UCLASS(NotBlueprintable, BlueprintType, PrioritizeCategories = (Style))
class TOROSYSTEMS_API UCommonButton final : public UCommonButtonBase
{
	GENERATED_BODY()

	static const FName ContentSlotName;

public:

	/** Returns the widget assigned to the content slot, or null when empty. */
	UFUNCTION(BlueprintPure, Category = Content)
	UWidget* GetContent() const;

	/** Replaces the content widget; passing null clears the slot. */
	UFUNCTION(BlueprintCallable, Category = Content)
	void SetContent(UWidget* InContent);

	virtual void GetSlotNames(TArray<FName>& SlotNames) const override;

protected:

	UPROPERTY(Transient)
	TObjectPtr<UNamedSlot> Content;

	virtual UCommonButtonInternalBase* ConstructInternalButton() override;
};
