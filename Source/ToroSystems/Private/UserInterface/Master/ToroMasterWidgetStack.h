// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroMasterWidgetOverlay.h"
#include "UserInterface/Widgets/ToroUserWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "ToroMasterWidgetStack.generated.h"

/**
 * Internal menu stack for the master UI, accepting any UCommonActivatableWidget entry.
 * Hosts the gameplay overlay at slot zero and owns transitions that hide gameplay content while
 * menus are displayed. Class additions return an existing registered menu instead of pushing a duplicate.
 * UToroUserWidget entries have their own transitions disabled so the stack alone supplies animation.
 * Releasing Slate resources clears menu membership and class lookup while retaining the gameplay root.
 */
UCLASS(MinimalAPI, NotBlueprintable, NotBlueprintType, NotPlaceable)
class UToroMasterMenuStack final : public UCommonActivatableWidgetContainerBase
{
	GENERATED_BODY()

public:

	TOROSYSTEMS_API UCommonActivatableWidget* AddByClass(TSubclassOf<UCommonActivatableWidget> Class)
	{
		if (!IsValid(Class) || Class->HasAnyClassFlags(CLASS_Abstract))
		{
			return nullptr;
		}

		if (UCommonActivatableWidget* FoundWidget = GetRegisteredClassLayer(Class))
		{
			return FoundWidget;
		}

		if (UCommonActivatableWidget* NewWidget = AddWidget<UCommonActivatableWidget>(Class))
		{
			ClassLayers.Add(Class, NewWidget);
			return NewWidget;
		}

		return nullptr;
	}

	TOROSYSTEMS_API void RemoveByClass(TSubclassOf<UCommonActivatableWidget> Class)
	{
		if (!IsValid(Class) || Class->HasAnyClassFlags(CLASS_Abstract))
		{
			return;
		}

		if (UCommonActivatableWidget* FoundWidget = GetRegisteredClassLayer(Class))
		{
			RemoveWidget(*FoundWidget);
		}

		ClassLayers.Remove(Class);
	}

	TOROSYSTEMS_API void AddByInstance(UCommonActivatableWidget* Instance)
	{
		if (IsValid(Instance) && !WidgetList.Contains(Instance))
		{
			AddWidgetInstance(*Instance);
		}
	}

	TOROSYSTEMS_API void RemoveByInstance(UCommonActivatableWidget* Instance)
	{
		if (IsValid(Instance) && WidgetList.Contains(Instance))
		{
			RemoveWidget(*Instance);
		}
	}

	TOROSYSTEMS_API UToroMasterWidgetOverlay* GetRootContent()
	{
		if (!RootContent)
		{
			RootContent = CreateWidget<UToroMasterWidgetOverlay>(this);
			if (RootContent)
			{
				RootContent->ConfigureAsGameplayRoot();
				RootContent->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
		}

		return RootContent;
	}

private:

	UPROPERTY(Transient)
	TObjectPtr<UToroMasterWidgetOverlay> RootContent;

	TMap<TSubclassOf<UCommonActivatableWidget>, TWeakObjectPtr<UCommonActivatableWidget>> ClassLayers;

	UCommonActivatableWidget* GetRegisteredClassLayer(TSubclassOf<UCommonActivatableWidget> Class)
	{
		const TWeakObjectPtr<UCommonActivatableWidget>* FoundWidget = ClassLayers.Find(Class);
		if (FoundWidget && FoundWidget->IsValid() && WidgetList.Contains(FoundWidget->Get()))
		{
			return FoundWidget->Get();
		}

		return nullptr;
	}

	virtual void SynchronizeProperties() override
	{
		Super::SynchronizeProperties();
		UToroMasterWidgetOverlay* RootWidget = GetRootContent();
		if (RootWidget && MySwitcher)
		{
			MySwitcher->GetChildSlot(0)->AttachWidget(RootWidget->TakeWidget());
		}
	}

	virtual void OnWidgetAddedToList(UCommonActivatableWidget& AddedWidget) override
	{
		if (UToroUserWidget* ToroWidget = Cast<UToroUserWidget>(&AddedWidget))
		{
			ToroWidget->SetEnableTransitions(false);
		}

		if (MySwitcher)
		{
			MySwitcher->AddSlot() [ AddedWidget.TakeWidget() ];
			SetSwitcherIndex(MySwitcher->GetNumWidgets() - 1);
		}
	}
	
	virtual void ReleaseSlateResources(bool bReleaseChildren) override
	{
		Super::ReleaseSlateResources(bReleaseChildren);
		ClassLayers.Empty();
	}

	using Super::AddWidget;
	using Super::AddWidgetInstance;
	// ReSharper disable once CppHidingFunction
	void RemoveWidget(UCommonActivatableWidget& WidgetToRemove)
	{
		Super::RemoveWidget(WidgetToRemove);
	}
};
