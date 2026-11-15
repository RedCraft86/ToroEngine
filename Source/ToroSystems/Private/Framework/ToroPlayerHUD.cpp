// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroPlayerHUD.h"
#include "CommonActivatableWidget.h"
#include "GameFramework/PlayerController.h"
#include "UserInterface/UserInterfaceConfig.h"
#include "UserInterface/Bases/ToroAutoWidget.h"
#include "UserInterface/ToroMasterWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/ToroLogging.h"

AToroPlayerHUD::AToroPlayerHUD()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickGroup = TG_DuringPhysics;

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false; // Stop World Partition from unloading this
#endif

	bEnableAutoLODGeneration = false; // Exclude from HLOD
	SetCanBeDamaged(false);
}

AHUD* AToroPlayerHUD::Get(const UObject* ContextObject, TSubclassOf<AHUD> Class, int32 PlayerIdx)
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(ContextObject, PlayerIdx);
	AHUD* HUD = IsValid(PC) ? PC->GetHUD() : nullptr;
	return IsValid(HUD) && (!IsValid(Class) || HUD->IsA(Class)) ? HUD : nullptr;
}

bool AToroPlayerHUD::HasMasterWidget() const
{
	return IsValid(MasterWidget) && MasterWidget->IsInViewport();
}

UCommonActivatableWidget* AToroPlayerHUD::AddWidgetByClass(EToroMasterWidgetType Layer,
	TSubclassOf<UCommonActivatableWidget> Class) const
{
	return IsValid(MasterWidget) ? MasterWidget->AddByClass(Layer, Class) : nullptr;
}

void AToroPlayerHUD::RemoveWidgetByClass(EToroMasterWidgetType Layer,
	TSubclassOf<UCommonActivatableWidget> Class) const
{
	if (IsValid(MasterWidget))
	{
		MasterWidget->RemoveByClass(Layer, Class);
	}
}

void AToroPlayerHUD::AddWidgetByInstance(EToroMasterWidgetType Layer, UCommonActivatableWidget* Instance) const
{
	if (IsValid(MasterWidget))
	{
		MasterWidget->AddByInstance(Layer, Instance);
	}
}

void AToroPlayerHUD::RemoveWidgetByInstance(EToroMasterWidgetType Layer, UCommonActivatableWidget* Instance) const
{
	if (IsValid(MasterWidget))
	{
		MasterWidget->RemoveByInstance(Layer, Instance);
	}
}

UToroAutoWidget* AToroPlayerHUD::GetAutoWidget(TSubclassOf<UToroAutoWidget> Class) const
{
	if (!IsValid(Class))
	{
		return nullptr;
	}

	for (UToroAutoWidget* Widget : AutoWidgets)
	{
		if (IsValid(Widget) && Widget->IsA(Class))
		{
			return Widget;
		}
	}

	return nullptr;
}

void AToroPlayerHUD::BeginPlay()
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		MasterWidget = CreateWidget<UToroMasterWidget>(PC);
		if (MasterWidget)
		{
			MasterWidget->AddToPlayerScreen();
		}

		const FUserInterfaceConfig* UIConfig = FUserInterfaceConfig::Get();
		if (UIConfig && HasMasterWidget())
		{
			for (const TSoftClassPtr<UToroAutoWidget>& SoftClass : UIConfig->AutoWidgetClasses)
			{
				const TSubclassOf<UToroAutoWidget>& Class = SoftClass.LoadSynchronous();
				if (!IsValid(Class) || Class->HasAnyClassFlags(CLASS_Abstract))
				{
					continue;
				}

				UToroAutoWidget* Widget = CreateWidget<UToroAutoWidget>(PC, Class);
				if (!IsValid(Widget))
				{
					TORO_LOG_ERROR(TEXT("Failed to create widget for %s"), *Class->GetPathName());
					continue;
				}

				MasterWidget->AddByInstance(Widget->GetMasterWidgetLayer(), Widget);
				AutoWidgets.Add(Widget);
			}
		}
	}

	Super::BeginPlay();
}

void AToroPlayerHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (auto It = AutoWidgets.CreateIterator(); It; ++It)
	{
		if (IsValid(*It))
		{
			(*It)->RemoveFromParent();
		}

		It.RemoveCurrent();
	}

	if (MasterWidget)
	{
		MasterWidget->RemoveFromParent();
		MasterWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}
