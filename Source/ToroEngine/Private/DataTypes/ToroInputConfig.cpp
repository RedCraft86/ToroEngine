// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DataTypes/ToroInputConfig.h"
#include "Blueprint/UserWidget.h"

FToroInputConfig::FToroInputConfig(
	const EToroInputMode InInputMode,
	const bool bInShowMouseCursor,
	const EMouseLockMode InMouseLock,
	const bool bInHideCursorOnCapture,
	const bool bInConsumeCaptureMouseDown,
	UUserWidget* InFocusWidget
)
	: InputMode(InInputMode)
	, bShowMouseCursor(bInShowMouseCursor)
	, MouseLock(InMouseLock)
	, bHideCursorOnCapture(bInHideCursorOnCapture)
	, bConsumeCaptureMouseDown(bInConsumeCaptureMouseDown)
	, FocusWidget(InFocusWidget)
{}

const FInputModeDataBase& FToroInputConfig::GetInputMode()
{
	switch (InputMode)
	{
	case EToroInputMode::GameAndUI:
		GameAndUI.SetLockMouseToViewportBehavior(MouseLock);
		GameAndUI.SetHideCursorDuringCapture(bHideCursorOnCapture);
		if (FocusWidget.IsValid())
		{
			GameAndUI.SetWidgetToFocus(FocusWidget->TakeWidget());
		}
		else
		{
			GameAndUI.SetWidgetToFocus(nullptr);
		}
		return GameAndUI;

	case EToroInputMode::GameOnly:
		GameOnly.SetConsumeCaptureMouseDown(bConsumeCaptureMouseDown);
		return GameOnly;

	case EToroInputMode::UI_Only:
		UI_Only.SetLockMouseToViewportBehavior(MouseLock);
		if (FocusWidget.IsValid())
		{
			UI_Only.SetWidgetToFocus(FocusWidget->TakeWidget());
		}
		else
		{
			UI_Only.SetWidgetToFocus(nullptr);
		}
		return UI_Only;
	}

	return DefaultMode;
}

void FToroInputConfig::Reset()
{
	UI_Only = FInputModeUIOnly();
	GameOnly = FInputModeGameOnly();
	GameAndUI = FInputModeGameAndUI();
	FocusWidget.Reset();
}
