// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/Widgets/ToroMenuWidget.h"
#include "Framework/ToroPlayerController.h"
#include "DataTypes/ToroInputConfig.h"

UToroMenuWidget::UToroMenuWidget(): bPausesGame(false)
{
	bSupportsActivationFocus = true;
	bAutoRestoreFocus = true;
	bIsBackHandler = true;
	bIsModal = true;
}

void UToroMenuWidget::UpdateControllerRequests()
{
	if (AToroPlayerController* PC = GetOwningPlayer<AToroPlayerController>())
	{
		if (!IsActivated())
		{
			PC->ReleaseInputConfig(this);
			PC->ReleasePause(this);
			return;
		}

		const FToroInputConfig Config(
			EToroInputMode::GameAndUI,
			true,
			EMouseLockMode::LockAlways,
			false,
			true,
			this
		);

		PC->RequestInputConfig(this, Config);
		if (bPausesGame)
		{
			PC->RequestPause(this);
		}
		else
		{
			PC->ReleasePause(this);
		}
	}
}

void UToroMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	UpdateControllerRequests();
}

void UToroMenuWidget::NativeOnDeactivated()
{
	UpdateControllerRequests();
	Super::NativeOnDeactivated();
}
