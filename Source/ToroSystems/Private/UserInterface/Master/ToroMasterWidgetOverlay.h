// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

// ReSharper disable CppMemberFunctionMayBeConst
#pragma once

#include "Blueprint/WidgetTree.h"
#include "Components/OverlaySlot.h"
#include "Components/Overlay.h"
#include "CommonInputModeTypes.h"
#include "CommonActivatableWidget.h"
#include "Blueprint/UserWidgetPool.h"
#include "Input/UIActionBindingHandle.h"
#include "UserInterface/Widgets/ToroUserWidget.h"
#include "ToroMasterWidgetOverlay.generated.h"

/**
 * Internal overlay host for UCommonActivatableWidget entries in the master UI.
 * Supports pooled class additions and externally owned instances, activating entries when added.
 * Explicit removal deactivates entries and waits for UToroUserWidget exit transitions when playing;
 * other activatable entries are removed immediately after deactivation.
 * Releasing Slate resources preserves remaining entries and immediately finishes pending removals.
 * The independent overlay host remains inactive so it cannot displace menus as CommonUI's active input root.
 * Child presentation and explicit activation are independent of the host's activation state; modal children
 * can establish their own routing roots. The menu stack activates its configured gameplay root to supply
 * captured gameplay input when no menu is displayed.
 */
UCLASS(MinimalAPI, NotBlueprintable, NotBlueprintType, NotPlaceable)
class UToroMasterWidgetOverlay final : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:

	UToroMasterWidgetOverlay()
		: WidgetPool(*this)
	{
		bAutoActivate = false;
		bSupportsActivationFocus = false;
	}

	void ConfigureAsGameplayRoot()
	{
		bGameplayRoot = true;
		bAutoActivate = false;
		bSupportsActivationFocus = true;
	}

	TOROSYSTEMS_API UCommonActivatableWidget* AddByClass(TSubclassOf<UCommonActivatableWidget> Class)
	{
		if (!IsValid(RootOverlay) || !IsValid(Class) || Class->HasAnyClassFlags(CLASS_Abstract))
		{
			return nullptr;
		}

		if (UCommonActivatableWidget* FoundWidget = GetRegisteredClassLayer(Class))
		{
			return FoundWidget;
		}

		if (UCommonActivatableWidget* NewWidget = WidgetPool.GetOrCreateInstance<UCommonActivatableWidget>(Class))
		{
			HandleAdd(*NewWidget);
			ClassLayers.Add(Class, NewWidget);
			return NewWidget;
		}

		return nullptr;
	}

	TOROSYSTEMS_API void RemoveByClass(TSubclassOf<UCommonActivatableWidget> Class)
	{
		if (!IsValid(RootOverlay) || !IsValid(Class) || Class->HasAnyClassFlags(CLASS_Abstract))
		{
			return;
		}

		if (UCommonActivatableWidget* FoundWidget = GetRegisteredClassLayer(Class))
		{
			HandleRemove(*FoundWidget);
		}

		ClassLayers.Remove(Class);
	}

	TOROSYSTEMS_API void AddByInstance(UCommonActivatableWidget* Instance)
	{
		if (IsValid(Instance)
			&& IsValid(RootOverlay)
			&& !RootOverlay->HasChild(Instance))
		{
			HandleAdd(*Instance);
		}
	}

	TOROSYSTEMS_API void RemoveByInstance(UCommonActivatableWidget* Instance)
	{
		if (IsValid(Instance)
			&& IsValid(RootOverlay)
			&& RootOverlay->HasChild(Instance))
		{
			HandleRemove(*Instance);
		}
	}

private:

	UPROPERTY(Transient)
	TObjectPtr<UOverlay> RootOverlay;

	UPROPERTY(Transient)
	FUserWidgetPool WidgetPool;

	bool bGameplayRoot = false;
	TSet<TWeakObjectPtr<UToroUserWidget>> PendingRemoval;
	TMap<TSubclassOf<UCommonActivatableWidget>, TWeakObjectPtr<UCommonActivatableWidget>> ClassLayers;

	UCommonActivatableWidget* GetRegisteredClassLayer(TSubclassOf<UCommonActivatableWidget> Class)
	{
		const TWeakObjectPtr<UCommonActivatableWidget>* FoundWidget = ClassLayers.Find(Class);
		if (FoundWidget && FoundWidget->IsValid() && RootOverlay->HasChild(FoundWidget->Get()))
		{
			return FoundWidget->Get();
		}

		return nullptr;
	}

	void HandleAdd(UCommonActivatableWidget& Widget)
	{
		UOverlaySlot* OverlaySlot = RootOverlay->AddChildToOverlay(&Widget);
		OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
		OverlaySlot->SetVerticalAlignment(VAlign_Fill);
		OverlaySlot->SetPadding(FMargin(0.0f));
		Widget.ActivateWidget();
	}

	void HandleRemove(UCommonActivatableWidget& Widget)
	{
		if (UToroUserWidget* ToroWidget = Cast<UToroUserWidget>(&Widget))
		{
			PendingRemoval.Add(ToroWidget);
			ToroWidget->OnTransitionFinished().AddUObject(
				this, &UToroMasterWidgetOverlay::OnTransitionFinished
			);
			ToroWidget->DeactivateWidget();
			if (!ToroWidget->IsTransitioning())
			{
				OnTransitionFinished(ToroWidget);
			}
		}
		else
		{
			Widget.DeactivateWidget();
			FinalizeRemoval(Widget);
		}
	}

	void OnTransitionFinished(UToroUserWidget* Widget)
	{
		if (PendingRemoval.Remove(Widget) > 0)
		{
			FinalizeRemoval(*Widget);
		}
	}

	void FinalizeRemoval(UCommonActivatableWidget& Widget)
	{
		Widget.RemoveFromParent();
		WidgetPool.Release(&Widget);
	}

	virtual bool Initialize() override
	{
		const bool bInitializedThisFrame = Super::Initialize();
		if (bInitializedThisFrame && WidgetTree)
		{
			RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
			WidgetTree->RootWidget = RootOverlay;
		}

		return bInitializedThisFrame;
	}

	virtual void ReleaseSlateResources(bool bReleaseChildren) override
	{
		const TArray<TWeakObjectPtr<UToroUserWidget>> Removing = PendingRemoval.Array();
		for (const TWeakObjectPtr<UToroUserWidget>& Entry : Removing)
		{
			if (UToroUserWidget* Widget = Entry.Get())
			{
				OnTransitionFinished(Widget);
			}
		}
		PendingRemoval.Empty();

		Super::ReleaseSlateResources(bReleaseChildren);
		WidgetPool.ReleaseAllSlateResources();
	}

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override
	{
		if (bGameplayRoot)
		{
			return FUIInputConfig(
				ECommonInputMode::Game,
				EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown,
				EMouseLockMode::LockOnCapture, true
			);
		}

		return Super::GetDesiredInputConfig();
	}
};
