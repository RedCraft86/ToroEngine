// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserInterface/Bases/ToroAutoWidget.h"
#include "ToroStaminaWidget.generated.h"

class UProgressBar;

/**
 * Gameplay auto-widget that displays the owning player's sprint stamina.
 * Hides while stamina is disabled or the player cannot run.
 */
UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType)
class UToroStaminaWidget final : public UToroAutoWidget
{
	GENERATED_BODY()

private:

	/** Progress bar that displays the current fraction of sprint stamina. */
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;

	/** Fill tint while sprinting is blocked by exhaustion. */
	UPROPERTY(EditDefaultsOnly, Category = Appearance)
	FLinearColor ExhaustedFillColor = FLinearColor::Red;

	FLinearColor NormalFillColor = FLinearColor::White;
	void RefreshStamina() const;

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
