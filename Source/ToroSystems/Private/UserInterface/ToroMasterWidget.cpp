// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/ToroMasterWidget.h"
#include "UserInterface/ToroMenuWidgetBase.h"
#include "UserInterface/ActivatableWidgetOverlay.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "CommonActivatableWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/OverlaySlot.h"
#include "Components/Overlay.h"

UCommonActivatableWidget* UToroMasterWidget::AddGameplayWidget(TSubclassOf<UCommonActivatableWidget> InClass) const
{
	if (GameplayLayer && IsValid(InClass) && !InClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return GameplayLayer->AddWidget(InClass);
	}

	return nullptr;
}

void UToroMasterWidget::RemoveGameplayWidget(TSubclassOf<UCommonActivatableWidget> InClass) const
{
	if (GameplayLayer && IsValid(InClass) && !InClass->HasAnyClassFlags(CLASS_Abstract))
	{
		GameplayLayer->RemoveWidget(InClass);
	}
}

UCommonActivatableWidget* UToroMasterWidget::AddMenuWidget(TSubclassOf<UToroMenuWidgetBase> InClass) const
{
	if (MenuLayer && IsValid(InClass) && !InClass->HasAnyClassFlags(CLASS_Abstract))
	{
		for (UCommonActivatableWidget* Widget : MenuLayer->GetWidgetList())
		{
			if (IsValid(Widget) && Widget->GetClass() == InClass.Get())
			{
				return Widget;
			}
		}

		return MenuLayer->AddWidget(InClass);
	}

	return nullptr;
}

void UToroMasterWidget::RemoveMenuWidget(TSubclassOf<UToroMenuWidgetBase> InClass) const
{
	if (MenuLayer && IsValid(InClass) && !InClass->HasAnyClassFlags(CLASS_Abstract))
	{
		for (UCommonActivatableWidget* Widget : MenuLayer->GetWidgetList())
		{
			if (IsValid(Widget) && Widget->GetClass() == InClass.Get())
			{
				MenuLayer->RemoveWidget(*Widget);
				return;
			}
		}
	}
}

void UToroMasterWidget::AddMenuWidgetInstance(UToroMenuWidgetBase* Widget) const
{
	if (MenuLayer && IsValid(Widget) && !MenuLayer->GetWidgetList().Contains(Widget))
	{
		MenuLayer->AddWidgetInstance(*Widget);
	}
}

void UToroMasterWidget::RemoveMenuWidgetInstance(UToroMenuWidgetBase* Widget) const
{
	if (MenuLayer && IsValid(Widget) && MenuLayer->GetWidgetList().Contains(Widget))
	{
		MenuLayer->RemoveWidget(*Widget);
	}
}

UCommonActivatableWidget* UToroMasterWidget::AddOverlayWidget(TSubclassOf<UCommonActivatableWidget> InClass) const
{
	if (OverlayLayer && IsValid(InClass) && !InClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return OverlayLayer->AddWidget(InClass);
	}

	return nullptr;
}

void UToroMasterWidget::RemoveOverlayWidget(TSubclassOf<UCommonActivatableWidget> InClass) const
{
	if (OverlayLayer && IsValid(InClass) && !InClass->HasAnyClassFlags(CLASS_Abstract))
	{
		OverlayLayer->RemoveWidget(InClass);
	}
}

void UToroMasterWidget::SetFadeDuration(float InDuration)
{
	if (FMath::IsFinite(InDuration) && InDuration >= 0.1f)
	{
		FadeDuration = InDuration;
		ApplyFadeDuration();
	}
}

void UToroMasterWidget::SetScreenFade(float InAmount)
{
	ScreenFade = FMath::Clamp(InAmount, 0.0f, 1.0f);
	if (ScreenFadeLayer)
	{
		ScreenFadeLayer->SetRenderOpacity(1.0f - ScreenFade);
	}
}

void UToroMasterWidget::ApplyFadeDuration() const
{
	if (GameplayLayer)
	{
		GameplayLayer->SetFadeDuration(FadeDuration);
	}
	if (MenuLayer)
	{
		MenuLayer->SetTransitionDuration(FadeDuration);
	}
	if (OverlayLayer)
	{
		OverlayLayer->SetFadeDuration(FadeDuration);
	}
}

void UToroMasterWidget::BindMenuLayer()
{
	UnbindMenuLayer();
	if (MenuLayer)
	{
		MenuLayer->OnDisplayedWidgetChanged().AddUObject(
			this, &UToroMasterWidget::OnMenuDisplayChanged
		);

		OnMenuDisplayChanged(MenuLayer->GetActiveWidget());
	}
}

void UToroMasterWidget::UnbindMenuLayer() const
{
	if (MenuLayer)
	{
		MenuLayer->OnDisplayedWidgetChanged().RemoveAll(this);
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UToroMasterWidget::OnMenuDisplayChanged(UCommonActivatableWidget* ActiveWidget)
{
	bShowGameplay = ActiveWidget == nullptr;
}

void UToroMasterWidget::NativeDestruct()
{
	UnbindMenuLayer();
	Super::NativeDestruct();
}

void UToroMasterWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetScreenFade(ScreenFade);
	ApplyFadeDuration();
	BindMenuLayer();
}

void UToroMasterWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

TSharedRef<SWidget> UToroMasterWidget::RebuildWidget()
{
	if (!WidgetTree->RootWidget)
	{
		auto AddChild = [](UOverlay* Parent, UWidget* Child)
		{
			UOverlaySlot* ChildSlot = Parent->AddChildToOverlay(Child);
			ChildSlot->SetHorizontalAlignment(HAlign_Fill);
			ChildSlot->SetVerticalAlignment(VAlign_Fill);
			ChildSlot->SetPadding(FMargin(0.0f));
		};

		UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
		Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		WidgetTree->RootWidget = Root;

		ScreenFadeLayer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("ScreenFadeLayer"));
		ScreenFadeLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		AddChild(Root, ScreenFadeLayer);

		GameplayLayer = WidgetTree->ConstructWidget<UActivatableWidgetOverlay>(
			UActivatableWidgetOverlay::StaticClass(), TEXT("GameplayLayer"));
		GameplayLayer->SetVisibility(ESlateVisibility::HitTestInvisible);
		AddChild(ScreenFadeLayer, GameplayLayer);

		MenuLayer = WidgetTree->ConstructWidget<UCommonActivatableWidgetStack>(
			UCommonActivatableWidgetStack::StaticClass(), TEXT("MenuLayer"));
		AddChild(ScreenFadeLayer, MenuLayer);

		OverlayLayer = WidgetTree->ConstructWidget<UActivatableWidgetOverlay>(
			UActivatableWidgetOverlay::StaticClass(), TEXT("OverlayLayer"));
		OverlayLayer->SetVisibility(ESlateVisibility::HitTestInvisible);
		AddChild(Root, OverlayLayer);

		ApplyFadeDuration();
		SetScreenFade(ScreenFade);
	}

	return Super::RebuildWidget();
}

void UToroMasterWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);

	// Manage GameplayLayer
	{
		const float Current = GameplayLayer->GetRenderOpacity();
		if (!bShowGameplay && FMath::IsNearlyZero(Current))
		{
			GameplayLayer->SetVisibility(ESlateVisibility::Collapsed);
			return;
		}

		GameplayLayer->SetVisibility(ESlateVisibility::HitTestInvisible);
		GameplayLayer->SetRenderOpacity(FMath::FInterpConstantTo(
			Current, bShowGameplay ? 1.0f : 0.0f, DeltaTime, 1.0f / FadeDuration
		));
	}
}
