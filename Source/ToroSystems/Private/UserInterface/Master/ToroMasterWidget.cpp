// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/ToroMasterWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "ToroMasterWidgetOverlay.h"
#include "ToroMasterWidgetStack.h"
#include "Components/OverlaySlot.h"
#include "Components/Overlay.h"
#include "Blueprint/WidgetTree.h"

UCommonActivatableWidget* UToroMasterWidget::AddByClass(EToroMasterWidgetType Layer,
	TSubclassOf<UCommonActivatableWidget> Class) const
{
	switch (Layer)
	{
	case EToroMasterWidgetType::Gameplay:
		if (GameplayLayer)
		{
			return GameplayLayer->AddByClass(Class);
		}
		break;
	case EToroMasterWidgetType::Menu:
		if (MenuLayer)
		{
			return MenuLayer->AddByClass(Class);
		}
		break;
	case EToroMasterWidgetType::Overlay:
		if (OverlayLayer)
		{
			return OverlayLayer->AddByClass(Class);
		}
		break;
	}

	return nullptr;
}

void UToroMasterWidget::RemoveByClass(EToroMasterWidgetType Layer,
	TSubclassOf<UCommonActivatableWidget> Class) const
{
	switch (Layer)
	{
	case EToroMasterWidgetType::Gameplay:
		if (GameplayLayer)
		{
			GameplayLayer->RemoveByClass(Class);
		}
		break;
	case EToroMasterWidgetType::Menu:
		if (MenuLayer)
		{
			MenuLayer->RemoveByClass(Class);
		}
		break;
	case EToroMasterWidgetType::Overlay:
		if (OverlayLayer)
		{
			OverlayLayer->RemoveByClass(Class);
		}
		break;
	}
}

void UToroMasterWidget::AddByInstance(EToroMasterWidgetType Layer,
	UCommonActivatableWidget* Instance) const
{
	switch (Layer)
	{
	case EToroMasterWidgetType::Gameplay:
		if (GameplayLayer)
		{
			GameplayLayer->AddByInstance(Instance);
		}
		break;
	case EToroMasterWidgetType::Menu:
		if (MenuLayer)
		{
			MenuLayer->AddByInstance(Instance);
		}
		break;
	case EToroMasterWidgetType::Overlay:
		if (OverlayLayer)
		{
			OverlayLayer->AddByInstance(Instance);
		}
		break;
	}
}

void UToroMasterWidget::RemoveByInstance(EToroMasterWidgetType Layer,
	UCommonActivatableWidget* Instance) const
{
	switch (Layer)
	{
	case EToroMasterWidgetType::Gameplay:
		if (GameplayLayer)
		{
			GameplayLayer->RemoveByInstance(Instance);
		}
		break;
	case EToroMasterWidgetType::Menu:
		if (MenuLayer)
		{
			MenuLayer->RemoveByInstance(Instance);
		}
		break;
	case EToroMasterWidgetType::Overlay:
		if (OverlayLayer)
		{
			OverlayLayer->RemoveByInstance(Instance);
		}
		break;
	}
}

bool UToroMasterWidget::Initialize()
{
	const bool bInitializedThisFrame = Super::Initialize();
	if (bInitializedThisFrame && WidgetTree)
	{
		UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(), TEXT("RootOverlay"));
		RootOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		WidgetTree->RootWidget = RootOverlay;

		auto AttachWidget = [RootOverlay](UWidget* Attachment)
		{
			UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(Attachment);
			OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			OverlaySlot->SetVerticalAlignment(VAlign_Fill);
			OverlaySlot->SetPadding(FMargin(1.0f));
		};

		MenuLayer = WidgetTree->ConstructWidget<UToroMasterMenuStack>(
			UToroMasterMenuStack::StaticClass(), TEXT("MenuLayer"));
		MenuLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		AttachWidget(MenuLayer);

		GameplayLayer = MenuLayer->GetRootContent();
		if (GameplayLayer)
		{
			GameplayLayer->SetVisibility(ESlateVisibility::HitTestInvisible);
		}

		OverlayLayer = WidgetTree->ConstructWidget<UToroMasterWidgetOverlay>(
			UToroMasterWidgetOverlay::StaticClass(), TEXT("OverlayLayer"));
		OverlayLayer->SetVisibility(ESlateVisibility::HitTestInvisible);
		AttachWidget(OverlayLayer);
	}

	return bInitializedThisFrame;
}

void UToroMasterWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (MenuLayer)
	{
		const APlayerCameraManager* PCM = GetOwningPlayerCameraManager();
		if (IsValid(PCM) && PCM->bEnableFading && PCM->FadeAmount > 0.0f)
		{
			bUpdateMenuOpacity = true;
			MenuLayer->SetRenderOpacity(1.0f - PCM->FadeAmount);
			MenuLayer->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else if (bUpdateMenuOpacity)
		{
			bUpdateMenuOpacity = false;
			MenuLayer->SetRenderOpacity(1.0f);
			MenuLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
	else
	{
		bUpdateMenuOpacity = true;
	}
}
