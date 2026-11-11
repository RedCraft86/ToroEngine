// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/Widgets/ToroMenuWidget.h"
#include "Framework/ToroPlayerController.h"
#include "CommonInputModeTypes.h"

UToroMenuWidget::UToroMenuWidget()
	: InputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture, EMouseLockMode::DoNotLock)
	, bPausesGame(false)
{
	bAutoActivate = false;
	bSetVisibilityOnActivated = false;
	bSetVisibilityOnDeactivated = false;
	bSupportsActivationFocus = true;
	bAutoRestoreFocus = true;
	bIsBackHandler = true;
	bIsModal = true;
}

void UToroMenuWidget::UpdatePauseRequest() const
{
	if (AToroPlayerController* PC = GetOwningPlayer<AToroPlayerController>())
	{
		if (!IsActivated())
		{
			PC->ReleasePause(this);
			return;
		}

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
	UpdatePauseRequest();
}

void UToroMenuWidget::NativeOnDeactivated()
{
	UpdatePauseRequest();
	Super::NativeOnDeactivated();
}

TOptional<FUIInputConfig> UToroMenuWidget::GetDesiredInputConfig() const
{
	const TOptional<FUIInputConfig> BlueprintConfig = Super::GetDesiredInputConfig();
	if (BlueprintConfig.IsSet())
	{
		return BlueprintConfig;
	}

	return InputConfig;
}