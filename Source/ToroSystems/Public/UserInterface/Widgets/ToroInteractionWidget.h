// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserInterface/Bases/ToroAutoWidget.h"
#include "ToroInteractionWidget.generated.h"

class UImage;
class UTextBlock;

/**
 * Gameplay auto-widget that displays an idle image or the current interaction prompt.
 * Hides while the owning player cannot interact.
 */
UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType)
class UToroInteractionWidget final : public UToroAutoWidget
{
	GENERATED_BODY()

private:

	/** Image shown when interaction is available and no target is selected. */
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage> IdleImage;

	/** Image shown when the player has an interaction target. */
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage> TargetImage;

	/** Displays the selected target's label when it has one. */
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> TargetLabel;

	void RefreshPrompt() const;

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
