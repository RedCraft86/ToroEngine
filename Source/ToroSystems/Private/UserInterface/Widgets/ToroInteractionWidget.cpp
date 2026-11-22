// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/Widgets/ToroInteractionWidget.h"
#include "ActorBehaviors/Trigger/InteractTriggerBehavior.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Framework/ToroPlayerCharacter.h"

void UToroInteractionWidget::RefreshPrompt() const
{
	UWidget* Content = GetRootWidget();
	if (!Content || !IdleImage || !TargetImage || !TargetLabel)
	{
		return;
	}

	const auto* Player = GetOwningPlayerPawn<AToroPlayerCharacter>();
	const bool bCanInteract = Player && Player->HasCapability(EPlayerCapability::Interact);
	// Keep the user widget ticking while its Designer content is hidden.
	Content->SetVisibility(bCanInteract ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (!bCanInteract)
	{
		return;
	}

	const UInteractTriggerBehavior* Target = Player->GetInteractionTarget();
	IdleImage->SetVisibility(Target ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	TargetImage->SetVisibility(Target ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	const FText Label = Target ? Target->DisplayLabel : FText::GetEmpty();
	if (!TargetLabel->GetText().IdenticalTo(Label))
	{
		TargetLabel->SetText(Label);
	}
	TargetLabel->SetVisibility(Label.IsEmptyOrWhitespace() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

void UToroInteractionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::HitTestInvisible);
	RefreshPrompt();
}

void UToroInteractionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshPrompt();
}
