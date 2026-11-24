// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Cutscenes/ToroCutsceneWidget.h"
#include "Cutscenes/CutsceneSubsystem.h"
#include "Cutscenes/CutsceneActor.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

UToroCutsceneWidget::UToroCutsceneWidget()
{
	bIsBackHandler = false;
	bPausesGame = false;
	SetIsFocusable(true);
}

void UToroCutsceneWidget::CompleteSkip()
{
	ResetSkipHold();
	if (Subsystem.IsValid())
	{
		Subsystem->SkipActiveCutscene();
	}
}

void UToroCutsceneWidget::ResetSkipHold()
{
	bSkipHeld = false;
	SkipHoldCounter = 0.0f;
	SkipProgress->SetPercent(0.0f);
}

void UToroCutsceneWidget::BeginSkipHold()
{
	if (bSkipHeld || !Subsystem.IsValid() || !Subsystem->CanSkipCutscene(SkipHoldTime))
	{
		return;
	}

	if (SkipHoldTime <= 0.0f)
	{
		CompleteSkip();
		return;
	}

	SkipHoldCounter = SkipHoldTime;
	bSkipHeld = true;
}

void UToroCutsceneWidget::OnUpdateSubtitle(const FText& NewText) const
{
	SubtitleText->SetText(NewText);
	SubtitleContainer->SetVisibility(
		NewText.IsEmptyOrWhitespace()
		? ESlateVisibility::Collapsed
		: ESlateVisibility::HitTestInvisible
	);
}

void UToroCutsceneWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	OnUpdateSubtitle(FText::GetEmpty());
	ResetSkipHold();

	Subsystem = GetWorld()->GetSubsystem<UCutsceneSubsystem>();
	ACutsceneActor* Cutscene = Subsystem.IsValid() ? Subsystem->GetCutscene() : nullptr;
	if (Cutscene)
	{
		Cutscene->OnSubtitle().BindUObject(this, &UToroCutsceneWidget::OnUpdateSubtitle);
		OnUpdateSubtitle(Cutscene->GetActiveSubtitle());
	}

	const bool bSkipBound = SkipKey.IsValid() && Subsystem.IsValid() && Subsystem->CanSkipCutscene(SkipHoldTime);
	SkipProgress->SetVisibility(bSkipBound && SkipHoldTime > 0.0f
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed);
	SkipKeyText->SetVisibility(bSkipBound
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed);
	SkipKeyText->SetText(FText::Format(SkipHoldTime > 0.0f
		? INVTEXT("Hold {0} to Skip")
		: INVTEXT("Press {0} to Skip"), SkipKey.GetDisplayName()));
}

void UToroCutsceneWidget::NativeOnDeactivated()
{
	ResetSkipHold();
	Super::NativeOnDeactivated();
}

UWidget* UToroCutsceneWidget::NativeGetDesiredFocusTarget() const
{
	return const_cast<UToroCutsceneWidget*>(this);
}

FReply UToroCutsceneWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (IsActivated() && InKeyEvent.GetKey() == SkipKey)
	{
		if (!InKeyEvent.IsRepeat())
		{
			BeginSkipHold();
		}
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

FReply UToroCutsceneWidget::NativeOnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (IsActivated() && InKeyEvent.GetKey() == SkipKey)
	{
		ResetSkipHold();
		return FReply::Handled();
	}
	return Super::NativeOnKeyUp(MyGeometry, InKeyEvent);
}

void UToroCutsceneWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	ResetSkipHold();
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
}

void UToroCutsceneWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (bSkipHeld)
	{
		SkipHoldCounter = FMath::Max(0.0f, SkipHoldCounter - InDeltaTime);
	}
	const float RemainingHoldTime = bSkipHeld ? SkipHoldCounter : SkipHoldTime;
	const bool bCanSkip = SkipKey.IsValid() && Subsystem.IsValid() && Subsystem->CanSkipCutscene(RemainingHoldTime);
	SkipProgress->SetVisibility(bCanSkip && SkipHoldTime > 0.0f
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed);
	SkipKeyText->SetVisibility(bCanSkip
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed);
	if (!bCanSkip)
	{
		if (bSkipHeld)
		{
			ResetSkipHold();
		}
		return;
	}

	if (bSkipHeld)
	{
		if (SkipHoldCounter > 0.0f && SkipHoldTime > 0.0f)
		{
			SkipProgress->SetPercent(1.0f - SkipHoldCounter / SkipHoldTime);
			return;
		}

		SkipProgress->SetPercent(1.0f);
		CompleteSkip();
	}
}
