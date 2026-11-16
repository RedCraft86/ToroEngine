// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/Widgets/ToroNotifyWidget.h"
#include "UserInterface/UserInterfaceConfig.h"
#include "Framework/ToroPlayerHUD.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"
#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"

namespace
{
	float NotifyProgress(const float Elapsed, const float Duration)
	{
		return Duration <= 0.0f
			? 1.0f
			: FMath::Clamp(Elapsed / Duration, 0.0f, 1.0f);
	}

	float NotifyEase(const float Alpha)
	{
		return Alpha * Alpha * (3.0f - 2.0f * Alpha);
	}
}

UToroNotifyWidget::UToroNotifyWidget()
{
	AutoLayer = EToroAutoWidgetType::Overlay;
}

bool UToroNotifyWidget::AddNotification(const UObject* ContextObject, FText Text, float ExtraTime)
{
	const AToroPlayerHUD* HUD = AToroPlayerHUD::Get(ContextObject);
	if (UToroNotifyWidget* Widget = HUD ? HUD->GetAutoWidget<UToroNotifyWidget>() : nullptr)
	{
		return Widget->AddNotification(Text, ExtraTime);
	}

	return false;
}

bool UToroNotifyWidget::ClearNotifications(const UObject* ContextObject)
{
	const AToroPlayerHUD* HUD = AToroPlayerHUD::Get(ContextObject);
	if (UToroNotifyWidget* Widget = HUD ? HUD->GetAutoWidget<UToroNotifyWidget>() : nullptr)
	{
		Widget->ClearNotifications();
		return true;
	}

	return false;
}

void UToroNotifyWidget::RecalculateTargets()
{
	float TargetY = 0.0f;
	const float Spacing = FMath::Max(0.0f, EntrySpacing);
	for (FToroNotifyEntry& Entry : Entries)
	{
		// Leaving entries stay frozen, but continue reserving their row.
		if (Entry.Phase != EToroNotifyPhase::Leaving &&
			!FMath::IsNearlyEqual(Entry.TargetY, TargetY))
		{
			Entry.StartY = Entry.CurrentY;
			Entry.TargetY = TargetY;
			Entry.MoveElapsed = 0.0f;
		}

		TargetY += Entry.Height + Spacing;
	}
}

void UToroNotifyWidget::ClearNotifications()
{
	for (const FToroNotifyEntry& Entry : Entries)
	{
		Entry.Widget->RemoveFromParent();
	}

	Entries.Reset();
}

bool UToroNotifyWidget::AddNotification(const FText& Text, float ExtraTime)
{
	const FUserInterfaceConfig* Config = FUserInterfaceConfig::Get();
	if (Text.IsEmptyOrWhitespace() || !TextStyle || !EntryCanvas || !WidgetTree || !Config)
	{
		return false;
	}

	// Make sure the last entry isn't the same text
	if (!Entries.IsEmpty() && Entries[Entries.Num() - 1].Text.ToString() == Text.ToString())
	{
		return true;
	}

	UCommonTextBlock* TextBlock = WidgetTree->ConstructWidget<UCommonTextBlock>();
	TextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
	TextBlock->SetJustification(ETextJustify::Center);
	TextBlock->SetRenderOpacity(0.0f);
	TextBlock->SetWrapTextAt(EntryWidth);
	TextBlock->SetStyle(TextStyle);
	TextBlock->SetText(Text);

	UCanvasPanelSlot* CanvasSlot = EntryCanvas->AddChildToCanvas(TextBlock);
	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.0f));
	CanvasSlot->SetAnchors(FAnchors(0.5f, 0.0f));
	CanvasSlot->SetAutoSize(false);

	TextBlock->ForceLayoutPrepass();
	const float Height = TextBlock->GetDesiredSize().Y;
	CanvasSlot->SetSize(FVector2D(EntryWidth, Height));

	float TargetY = 0.0f;
	const float Spacing = FMath::Max(0.0f, EntrySpacing);
	for (const FToroNotifyEntry& Existing : Entries)
	{
		TargetY += Existing.Height + Spacing;
	}

	FToroNotifyEntry& Entry = Entries.Emplace_GetRef(Text);
	Entry.HoldDuration = Config->CalculateTimeForText(Text, ExtraTime + PerItemExtraTime * (Entries.Num() - 1));
	Entry.Widget = TextBlock;
	Entry.Height = Height;
	Entry.TargetY = TargetY;
	Entry.CurrentY = TargetY + FMath::Max(0.0f, EntranceOffset);
	Entry.StartY = Entry.CurrentY;
	CanvasSlot->SetPosition(FVector2D(0.0f, Entry.CurrentY));

	if (FadeDuration <= 0.0f)
	{
		TextBlock->SetRenderOpacity(1.0f);
	}

	return true;
}

void UToroNotifyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

    bool bRemovedEntry = false;
    const float DeltaTime = FMath::Max(0.0f, InDeltaTime);
    for (int32 i = Entries.Num() - 1; i >= 0; i--)
    {
        FToroNotifyEntry& Entry = Entries[i];
        Entry.PhaseElapsed += DeltaTime;

        if (Entry.Phase != EToroNotifyPhase::Leaving)
        {
            Entry.MoveElapsed += DeltaTime;
            const float MoveAlpha = NotifyProgress(Entry.MoveElapsed, MoveDuration);
            Entry.CurrentY = FMath::Lerp(Entry.StartY, Entry.TargetY, NotifyEase(MoveAlpha));
            if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Entry.Widget->Slot))
            {
                CanvasSlot->SetPosition(FVector2D(0.0f, Entry.CurrentY));
            }
        }

        switch (Entry.Phase)
        {
        case EToroNotifyPhase::Entering:
	        {
		        const float Alpha = NotifyProgress(Entry.PhaseElapsed, FadeDuration);
	        	Entry.Widget->SetRenderOpacity(NotifyEase(Alpha));

	        	// Hold begins after both entrance movement and fading finish.
	        	if (Alpha >= 1.0f && NotifyProgress(Entry.MoveElapsed, MoveDuration) >= 1.0f)
	        	{
	        		Entry.Phase = EToroNotifyPhase::Visible;
	        		Entry.PhaseElapsed = 0.0f;
	        	}
	        	break;
	        }

        case EToroNotifyPhase::Visible:
	        {
		        if (Entry.PhaseElapsed >= Entry.HoldDuration)
		        {
		        	// Freeze wherever it currently is, even during a gap-closing slide.
		        	Entry.Phase = EToroNotifyPhase::Leaving;
		        	Entry.PhaseElapsed = 0.0f;
		        }
	        	break;
	        }

        case EToroNotifyPhase::Leaving:
	        {
		        const float Alpha = NotifyProgress(Entry.PhaseElapsed, FadeDuration);
	        	Entry.Widget->SetRenderOpacity(1.0f - NotifyEase(Alpha));

	        	if (Alpha >= 1.0f)
	        	{
	        		Entry.Widget->RemoveFromParent();
	        		Entries.RemoveAt(i);
	        		bRemovedEntry = true;
	        	}
	        	break;
	        }
        }
    }

    // Batch simultaneous removals before retargeting the remaining entries.
    if (bRemovedEntry)
    {
        RecalculateTargets();
    }
}

void UToroNotifyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!EntryCanvas || !TextStyle)
	{
		RemoveFromParent();
	}
}

void UToroNotifyWidget::NativeDestruct()
{
    ClearNotifications();
	Super::NativeDestruct();
}
