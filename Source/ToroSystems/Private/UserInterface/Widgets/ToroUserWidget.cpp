// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/Widgets/ToroUserWidget.h"
#include "Slate/SCommonAnimatedSwitcher.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Containers/Ticker.h"

UToroUserWidget::UToroUserWidget()
	: bEnableTransitions(true)
	, TransitionType(ECommonSwitcherTransition::FadeOnly)
	, TransitionCurveType(ETransitionCurve::Linear)
	, TransitionDuration(0.4f)
	, bTransitionCompletionPending(false)
{
}

void UToroUserWidget::SetEnableTransitions(bool bEnable)
{
	if (bEnableTransitions != bEnable)
	{
		bEnableTransitions = bEnable;
		if (!bEnableTransitions)
		{
			UpdateTransitionState();
		}
	}
}

void UToroUserWidget::SetTransitionType(ECommonSwitcherTransition Transition)
{
	TransitionType = Transition;
	UpdateTransitionParams();
}

void UToroUserWidget::SetTransitionCurveType(ETransitionCurve CurveType)
{
	TransitionCurveType = CurveType;
	UpdateTransitionParams();
}

void UToroUserWidget::SetTransitionDuration(float Duration)
{
	TransitionDuration = FMath::Max(Duration, 0.0f);
	UpdateTransitionParams();
}

void UToroUserWidget::SetTransitionParams(float Duration,
	ETransitionCurve CurveType, ECommonSwitcherTransition Transition)
{
	TransitionDuration = FMath::Max(Duration, 0.0f);
	TransitionCurveType = CurveType;
	TransitionType = Transition;
	UpdateTransitionParams();
}

bool UToroUserWidget::IsTransitioning() const
{
	return AnimSwitcher.IsValid() && AnimSwitcher->IsTransitionPlaying();
}

void UToroUserWidget::RemoveFromParent()
{
	OnTransitionFinishedEvent.Clear();
	Super::RemoveFromParent();
}

void UToroUserWidget::UpdateTransitionParams()
{
	TransitionDuration = FMath::Max(TransitionDuration, 0.0f);
	if (AnimSwitcher.IsValid() && !AnimSwitcher->IsTransitionPlaying())
	{
		AnimSwitcher->SetTransition(
			TransitionDuration,
			TransitionCurveType,
			TransitionType
		);
	}
}

void UToroUserWidget::UpdateTransitionState()
{
	if (AnimSwitcher.IsValid())
	{
		const int32 TargetIndex = IsActivated() ? 1 : 0;
		if (AnimSwitcher->GetActiveWidgetIndex() == TargetIndex && !AnimSwitcher->IsTransitionPlaying())
		{
			OnTransitionStateChanged(false);
			return;
		}

		bTransitionCompletionPending = true;

		if (!bEnableTransitions)
		{
			AnimSwitcher->TransitionToIndex(TargetIndex, true);
			OnTransitionStateChanged(false);
			return;
		}

		if (!RestoreVisibility.IsSet())
		{
			RestoreVisibility = GetVisibility();
		}

		SetVisibility(ESlateVisibility::HitTestInvisible);
		AnimSwitcher->TransitionToIndex(TargetIndex);
		if (!AnimSwitcher->IsTransitionPlaying())
		{
			OnTransitionStateChanged(false);
		}
	}
}

void UToroUserWidget::RestorePendingVisibility()
{
	if (RestoreVisibility.IsSet())
	{
		const ESlateVisibility Vis = RestoreVisibility.GetValue();
		RestoreVisibility.Reset();
		SetVisibility(Vis);
	}
}

void UToroUserWidget::OnTransitionStateChanged(bool bTransitioning)
{
	if (!InputGuard.IsValid() || !AnimSwitcher.IsValid())
	{
		return;
	}

	InputGuard->SetVisibility(bTransitioning
		? EVisibility::Visible
		: EVisibility::Collapsed);

	if (!bTransitioning)
	{
		const bool bNotify = bTransitionCompletionPending;
		bTransitionCompletionPending = false;

		AnimSwitcher->SetRenderTransform(FSlateRenderTransform());
		AnimSwitcher->SetVisibility(EVisibility::SelfHitTestInvisible);
		RestorePendingVisibility();
		UpdateTransitionParams();

		if (bNotify && !AnimSwitcher->IsTransitionPlaying())
		{
			ExecuteOnGameThread(TEXT("TransitionFinished"), [WeakThis = TWeakObjectPtr(this)]
			{
				if (WeakThis.IsValid()
					&& WeakThis->AnimSwitcher.IsValid()
					&& !WeakThis->AnimSwitcher->IsTransitionPlaying())
				{
					WeakThis->OnTransitionFinishedEvent.Broadcast(WeakThis.Get());
				}
			});
		}
	}
}

void UToroUserWidget::NativeOnActivated()
{
	RestorePendingVisibility();
	Super::NativeOnActivated();
	UpdateTransitionState();
}

void UToroUserWidget::NativeOnDeactivated()
{
	RestorePendingVisibility();
	Super::NativeOnDeactivated();
	UpdateTransitionState();
}

void UToroUserWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	if (AnimSwitcher.IsValid() && IsDesignTime())
	{
		AnimSwitcher->TransitionToIndex(1, true);
	}
}

TSharedRef<SWidget> UToroUserWidget::RebuildWidget()
{
	const TSharedPtr<SOverlay> HostOverlay = SNew(SOverlay)
	+SOverlay::Slot(0)
	[
		SAssignNew(AnimSwitcher, SCommonAnimatedSwitcher)
		.InitialIndex(!bEnableTransitions && IsActivated() ? 1 : 0)
		.TransitionType(TransitionType)
		.TransitionCurveType(TransitionCurveType)
		.TransitionDuration(FMath::Max(TransitionDuration, 0.0f))
		.TransitionFallbackStrategy(ECommonSwitcherTransitionFallbackStrategy::First)
		.OnIsTransitioningChanged_UObject(this, &UToroUserWidget::OnTransitionStateChanged)
	]
	+SOverlay::Slot(1)
	[
		SAssignNew(InputGuard, SSpacer)
		.Visibility(EVisibility::Collapsed)
	];

	AnimSwitcher->AddSlot(0)
	[
		SNullWidget::NullWidget
	];

	AnimSwitcher->AddSlot(1)
	[
		Super::RebuildWidget()
	];

	return HostOverlay.ToSharedRef();
}

void UToroUserWidget::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	if (!IsDesignTime() && IsActivated() && !IsTransitioning())
	{
		UpdateTransitionState();
	}
}

void UToroUserWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	InputGuard.Reset();
	AnimSwitcher.Reset();
	RestorePendingVisibility();
	bTransitionCompletionPending = false;
	Super::ReleaseSlateResources(bReleaseChildren);
}
