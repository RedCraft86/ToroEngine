// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/Widgets/ToroStaminaWidget.h"
#include "Components/ProgressBar.h"
#include "Framework/ToroPlayerCharacter.h"

void UToroStaminaWidget::RefreshStamina() const
{
	UWidget* Content = GetRootWidget();
	if (!Content || !StaminaBar)
	{
		return;
	}

	const auto* Player = GetOwningPlayerPawn<AToroPlayerCharacter>();
	const bool bShowStamina = Player && Player->IsStaminaEnabled() && Player->HasCapability(EPlayerCapability::Run);
	// Keep the user widget ticking while its Designer content is hidden.
	Content->SetVisibility(bShowStamina ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bShowStamina)
	{
		StaminaBar->SetPercent(Player->GetStamina());
		const FLinearColor& FillColor = Player->IsStaminaExhausted() ? ExhaustedFillColor : NormalFillColor;
		if (StaminaBar->GetFillColorAndOpacity() != FillColor)
		{
			StaminaBar->SetFillColorAndOpacity(FillColor);
		}

		StaminaBar->SetRenderOpacity(FMath::GetMappedRangeValueClamped(
			FVector2D(0.9f, 1.0f), FVector2D(1.0f, 0.0f), StaminaBar->GetPercent()
		));
	}
}

void UToroStaminaWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (StaminaBar)
	{
		NormalFillColor = StaminaBar->GetFillColorAndOpacity();
	}
}

void UToroStaminaWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::HitTestInvisible);
	RefreshStamina();
}

void UToroStaminaWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshStamina();
}
