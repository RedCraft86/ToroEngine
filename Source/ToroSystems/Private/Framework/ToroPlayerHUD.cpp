// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroPlayerHUD.h"
#include "CommonActivatableWidget.h"
#include "GameFramework/PlayerController.h"
#include "UserInterface/ToroMasterWidget.h"
#include "Kismet/GameplayStatics.h"

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

bool AToroPlayerHUD::HasMasterWidget() const
{
	return IsValid(MasterWidget) && MasterWidget->IsInViewport();
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
	}
	Super::BeginPlay();
}

void AToroPlayerHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MasterWidget)
	{
		MasterWidget->RemoveFromParent();
		MasterWidget = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}
