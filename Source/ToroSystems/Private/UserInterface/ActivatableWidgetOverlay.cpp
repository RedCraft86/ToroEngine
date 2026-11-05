// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/ActivatableWidgetOverlay.h"
#include "CommonActivatableWidget.h"
#include "Components/OverlaySlot.h"
#include "Slate/SObjectWidget.h"

UActivatableWidgetOverlay::UActivatableWidgetOverlay()
	: WidgetPool(*this)
{
}

UCommonActivatableWidget* UActivatableWidgetOverlay::AddWidget(TSubclassOf<UCommonActivatableWidget> InClass)
{
	if (!IsValid(InClass) || InClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return nullptr;
	}

	if (const FEntry* Entry = Widgets.Find(InClass))
	{
		if (UCommonActivatableWidget* Widget = Entry->Widget.Get())
		{
			if (!Widget->IsActivated())
			{
				Widget->ActivateWidget();
			}
			return Widget;
		}
	}

	TGuardValue<bool> TickGuard(bPauseTicking, true);

	UCommonActivatableWidget* NewWidget = WidgetPool.GetOrCreateInstance<UCommonActivatableWidget>(InClass,
		[this](UUserWidget* Widget, TSharedRef<SWidget> Content)
		{
			RegisterEntry(CastChecked<UCommonActivatableWidget>(Widget));
			return SNew(SObjectWidget, Widget)
			[
				Content
			];
		});

	if (!NewWidget)
	{
		return nullptr;
	}

	if (!FindEntry(NewWidget))
	{
		RegisterEntry(NewWidget);
	}

	UOverlaySlot* NewSlot = AddChildToOverlay(NewWidget);
	NewSlot->SetHorizontalAlignment(HAlign_Fill);
	NewSlot->SetVerticalAlignment(VAlign_Fill);
	NewSlot->SetPadding(FMargin(0.0f));

	const FEntry* AddedEntry = FindEntry(NewWidget);
	if (AddedEntry && AddedEntry->bActive)
	{
		if (!NewWidget->IsActivated())
		{
			NewWidget->ActivateWidget();
		}
		else
		{
			HandleActivated(NewWidget);
		}
	}

	return NewWidget;
}

void UActivatableWidgetOverlay::RemoveWidget(TSubclassOf<UCommonActivatableWidget> InClass)
{
	if (!IsValid(InClass) || InClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return;
	}

	TGuardValue<bool> TickGuard(bPauseTicking, true);
	if (const FEntry* Entry = Widgets.Find(InClass))
	{
		if (UCommonActivatableWidget* Widget = Entry->Widget.Get())
		{
			if (Widget->IsActivated())
			{
				Widget->DeactivateWidget();
			}
			else
			{
				HandleDeactivated(Widget);
			}
		}
	}
}

void UActivatableWidgetOverlay::SetFadeDuration(float InDuration)
{
	if (FMath::IsFinite(InDuration) && InDuration >= 0.1f)
	{
		FadeDuration = InDuration;
	}
}

void UActivatableWidgetOverlay::RegisterEntry(UCommonActivatableWidget* Widget)
{
	Widgets.Add(Widget->GetClass(), FEntry(Widget, Widget->GetVisibility()));

	Widget->SetRenderOpacity(0.0f);
	Widget->OnActivated().AddUObject(this, &ThisClass::HandleActivated, Widget);
	Widget->OnDeactivated().AddUObject(this, &ThisClass::HandleDeactivated, Widget);
}

UActivatableWidgetOverlay::FEntry* UActivatableWidgetOverlay::FindEntry(const UCommonActivatableWidget* Widget)
{
	FEntry* Entry = Widgets.Find(Widget->GetClass());
	return Entry && Entry->Widget.Get() == Widget ? Entry : nullptr;
}

void UActivatableWidgetOverlay::HandleActivated(UCommonActivatableWidget* Widget)
{
	FEntry* Entry = FindEntry(Widget);
	if (!Entry || bUpdatingSlateResources)
	{
		return;
	}

	if (!Widget->IsActivated())
	{
		HandleDeactivated(Widget);
		return;
	}

	if (!Entry->bActive && !Widget->SetsVisibilityOnActivated())
	{
		Widget->SetVisibility(Entry->ActiveVisibility);
	}
	else
	{
		Entry->ActiveVisibility = Widget->GetVisibility();
	}

	Entry->bActive = true;
	Animations.Add(Widget, true);
	StartTickTimer(GetCachedWidget());
}

void UActivatableWidgetOverlay::HandleDeactivated(UCommonActivatableWidget* Widget)
{
	FEntry* Entry = FindEntry(Widget);
	if (!Entry || bUpdatingSlateResources)
	{
		return;
	}

	if (Widget->IsActivated())
	{
		HandleActivated(Widget);
		return;
	}

	Entry->bActive = false;
	Animations.Add(Widget, false);

	Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
	StartTickTimer(GetCachedWidget());
}

void UActivatableWidgetOverlay::UnbindDelegates(const UCommonActivatableWidget* Widget) const
{
	Widget->OnActivated().RemoveAll(this);
	Widget->OnDeactivated().RemoveAll(this);
}

void UActivatableWidgetOverlay::ClearTickTimer()
{
	const TSharedPtr<SWidget> Widget = GetCachedWidget();
	const TSharedPtr<FActiveTimerHandle> Timer = TickHandle.Pin();
	if (Widget.IsValid() && Timer.IsValid())
	{
		Widget->UnRegisterActiveTimer(Timer.ToSharedRef());
	}

	TickHandle.Reset();
}

void UActivatableWidgetOverlay::StartTickTimer(const TSharedPtr<SWidget>& InWidget)
{
	if (InWidget.IsValid() && !Animations.IsEmpty() && !TickHandle.IsValid() && !bUpdatingSlateResources)
	{
		TickHandle = InWidget->RegisterActiveTimer(0.0f,
			FWidgetActiveTimerDelegate::CreateUObject(this, &ThisClass::Tick)
		);
	}
}

EActiveTimerReturnType UActivatableWidgetOverlay::Tick(double, float DeltaTime)
{
	if (bPauseTicking)
	{
		return EActiveTimerReturnType::Continue;
	}

	TArray<TWeakObjectPtr<UCommonActivatableWidget>> Snapshot;
	Animations.GenerateKeyArray(Snapshot);
	for (const TWeakObjectPtr<UCommonActivatableWidget>& WeakWidget : Snapshot)
	{
		UCommonActivatableWidget* Widget = WeakWidget.Get();
		const bool* Animation = Animations.Find(WeakWidget);
		if (!Widget || !Animation)
		{
			Animations.Remove(WeakWidget);
			continue;
		}

		const bool bActive = *Animation;
		const float Target = bActive ? 1.0f : 0.0f;
		const float Current = FMath::FInterpConstantTo(
			Widget->GetRenderOpacity(), Target, DeltaTime, 1.0f / FadeDuration
		);

		Widget->SetRenderOpacity(Current);
		if (!FMath::IsNearlyEqual(Current, Target))
		{
			continue;
		}

		Widget->SetRenderOpacity(Target);
		Animations.Remove(WeakWidget);
		if (!bActive)
		{
			RemoveChild(Widget);
		}
	}

	if (Animations.IsEmpty())
	{
		TickHandle.Reset();
		return EActiveTimerReturnType::Stop;
	}

	return EActiveTimerReturnType::Continue;
}

TSharedRef<SWidget> UActivatableWidgetOverlay::RebuildWidget()
{
	ClearTickTimer();
	TGuardValue<bool> TickGuard(bPauseTicking, true);
	TSharedPtr<SWidget> SlateWidget;
	{
		TGuardValue<bool> SlateGuard(bUpdatingSlateResources, true);
		SlateWidget = Super::RebuildWidget();
		WidgetPool.RebuildWidgets();
	}

	TArray<TSubclassOf<UCommonActivatableWidget>> Snapshot;
	Widgets.GenerateKeyArray(Snapshot);
	for (const TSubclassOf<UCommonActivatableWidget>& Class : Snapshot)
	{
		const FEntry* Entry = Widgets.Find(Class);
		UCommonActivatableWidget* Widget = Entry ? Entry->Widget.Get() : nullptr;
		if (!Widget)
		{
			continue;
		}

		if (Entry->bActive)
		{
			Widget->SetVisibility(Entry->ActiveVisibility);
			if (!Widget->IsActivated())
			{
				Widget->ActivateWidget();
			}
		}
		else if (Widget->IsActivated())
		{
			Widget->DeactivateWidget();
		}

		if (const FEntry* CurrentEntry = FindEntry(Widget); CurrentEntry && !CurrentEntry->bActive)
		{
			Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}

	StartTickTimer(SlateWidget);
	return SlateWidget.ToSharedRef();
}

void UActivatableWidgetOverlay::OnSlotRemoved(UPanelSlot* InSlot)
{
	TGuardValue<bool> TickGuard(bPauseTicking, true);
	UCommonActivatableWidget* Widget = Cast<UCommonActivatableWidget>(InSlot->Content);
	const FEntry* Entry = Widget ? FindEntry(Widget) : nullptr;
	if (!Entry)
	{
		Super::OnSlotRemoved(InSlot);
		return;
	}

	const ESlateVisibility ActiveVisibility = Entry->ActiveVisibility;
	UnbindDelegates(Widget);
	Animations.Remove(Widget);
	Widgets.Remove(Widget->GetClass());
	Super::OnSlotRemoved(InSlot);

	Widget->DeactivateWidget();
	Widget->SetVisibility(ActiveVisibility);
	Widget->SetRenderOpacity(0.0f);
	WidgetPool.Release(Widget, true);
}

void UActivatableWidgetOverlay::ReleaseSlateResources(bool bReleaseChildren)
{
	ClearTickTimer();
	TGuardValue<bool> SlateGuard(bUpdatingSlateResources, true);
	Super::ReleaseSlateResources(bReleaseChildren);
	WidgetPool.ReleaseAllSlateResources();
}

void UActivatableWidgetOverlay::BeginDestroy()
{
	ClearTickTimer();
	for (const auto& Pair : Widgets)
	{
		if (const UCommonActivatableWidget* Widget = Pair.Value.Widget.Get())
		{
			UnbindDelegates(Widget);
		}
	}

	Animations.Empty();
	Widgets.Empty();
	WidgetPool.ResetPool();

	Super::BeginDestroy();
}
