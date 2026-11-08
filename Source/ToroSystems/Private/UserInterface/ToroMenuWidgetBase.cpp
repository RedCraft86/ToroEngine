// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/ToroMenuWidgetBase.h"
#include "Framework/ToroPlayerController.h"
#include "DataTypes/ToroInputConfig.h"

UToroMenuWidgetBase::UToroMenuWidgetBase(): bPausesGame(false)
{
	bSupportsActivationFocus = true;
	bAutoRestoreFocus = true;
	bIsBackHandler = true;
	bIsModal = true;
}

void UToroMenuWidgetBase::NativeOnActivated()
{
	Super::NativeOnActivated();
	if (AToroPlayerController* PC = GetOwningPlayer<AToroPlayerController>())
	{
		const FToroInputConfig Config(EToroInputMode::GameAndUI, true, EMouseLockMode::LockAlways);
		PC->RequestInputConfig(this, Config);
		if (bPausesGame)
		{
			PC->RequestPause(this);
		}
	}
}

void UToroMenuWidgetBase::NativeOnDeactivated()
{
	if (AToroPlayerController* PC = GetOwningPlayer<AToroPlayerController>())
	{
		PC->ReleaseInputConfig(this);
		if (bPausesGame)
		{
			PC->ReleasePause(this);
		}
	}
	Super::NativeOnDeactivated();
}
