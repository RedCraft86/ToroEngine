// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/Widgets/ToroDialogWidget.h"
#include "UserInterface/Elements/CommonTextButton.h"
#include "UserInterface/ToroUserInterfaceConfig.h"
#include "UserInterface/ToroMasterWidget.h"
#include "Framework/ToroPlayerHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/ToroLogging.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"

UToroDialogWidget::UToroDialogWidget()
{
	SetVisibilityInternal(ESlateVisibility::Visible);
	ChoiceButtonPadding = FMargin(10.0f);
}

void UToroDialogWidget::Create(
	const UObject* ContextObject,
	const FText TitleText,
	const FText BodyText,
	const TArray<FText>& Choices,
	bool bShouldPauseGame,
	FToroDialogReturnDelegate OnClosed,
	int32 AutoSelectIdx,
	float AutoSelectTime,
	int32 PlayerIdx)
{
	if (!OnClosed.IsBound())
	{
		TORO_LOG_WARNING(TEXT("User Dialog has no influence. Make sure to bind OnClosed."));
	}

	if (UToroDialogWidget* NewDialog = CreateInternal(
		ContextObject,
		TitleText,
		BodyText,
		Choices,
		bShouldPauseGame,
		AutoSelectIdx,
		AutoSelectTime,
		PlayerIdx))
	{
		NewDialog->BPDelegate = OnClosed;
	}
}

FToroDialogReturnDelegateNative::RegistrationType& UToroDialogWidget::Create(
	const UObject* ContextObject,
	const FText& TitleText,
	const FText& BodyText,
	const TArray<FText>& Choices,
	bool bShouldPauseGame,
	int32 AutoSelectIdx,
	float AutoSelectTime,
	int32 PlayerIdx)
{
	if (UToroDialogWidget* NewDialog = CreateInternal(
		ContextObject,
		TitleText,
		BodyText,
		Choices,
		bShouldPauseGame,
		AutoSelectIdx,
		AutoSelectTime,
		PlayerIdx))
	{
		return NewDialog->NativeDelegate;
	}

	static FToroDialogReturnDelegateNative Dummy;
	Dummy.Unbind();
	return Dummy;
}

UToroDialogWidget* UToroDialogWidget::CreateInternal(
	const UObject* ContextObject,
	const FText& TitleText,
	const FText& BodyText,
	const TArray<FText>& Choices,
	bool bShouldPauseGame,
	int32 AutoSelectIdx,
	float AutoSelectTime,
	int32 PlayerIdx)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(ContextObject, PlayerIdx);
	if (!IsValid(PC))
	{
		TORO_REPORT_ERROR(TEXT("Failed to create User Dialog: The Player Controller could not be found."));
		return nullptr;
	}

	const FToroUserInterfaceConfig* Config = FToroUserInterfaceConfig::Get();
	const TSubclassOf<UToroDialogWidget> Class = Config ? Config->DialogWidgetClass.LoadSynchronous() : nullptr;
	if (!IsValid(Class) || Class->HasAnyClassFlags(CLASS_Abstract))
	{
		TORO_REPORT_ERROR(TEXT("Failed to create User Dialog: Failed to obtain config or got an invalid class."));
		return nullptr;
	}

	UToroDialogWidget* Dialog = CreateWidget<UToroDialogWidget>(PC, Class);
	if (!IsValid(Dialog))
	{
		TORO_REPORT_ERROR(TEXT("Failed to create User Dialog: Widget could not be created."));
		return nullptr;
	}

	Dialog->bPausesGame = bShouldPauseGame;
	Dialog->InitializeDialog(TitleText, BodyText, Choices);
	if (AutoSelectIdx >= 0 && AutoSelectTime >= 3.0f
		&& Dialog->Buttons.IsValidIndex(AutoSelectIdx)
		&& IsValid(Dialog->Buttons[AutoSelectIdx]))
	{
		Dialog->SetupAutoSelection(AutoSelectIdx, AutoSelectTime, Choices);
	}

	const AToroPlayerHUD* HUD = AToroPlayerHUD::Get(ContextObject, PlayerIdx);
	if (!IsValid(HUD) || !HUD->HasMasterWidget())
	{
		TORO_REPORT_ERROR(TEXT("Failed to create User Dialog: Widget could not be created."));
		return nullptr;
	}

	HUD->AddWidgetByInstance(EToroMasterWidgetType::Menu, Dialog);

	return Dialog;
}

void UToroDialogWidget::InitializeDialog(const FText& TitleText, const FText& BodyText, const TArray<FText>& Choices)
{
	checkf(IsValid(TitleBlock)
	       && IsValid(BodyBlock)
	       && IsValid(ButtonContainer)
	       && IsValid(WidgetTree),
	       TEXT("Dialog elements are invalid and cannot be initialized."));

	TitleBlock->SetText(TitleText);
	BodyBlock->SetText(BodyText);

	// The buttons array will still map to the correct indices even if entries are skipped.
	// Skipped entries will just be null, which will get handled later.
	Buttons.SetNum(Choices.Num());
	for (int32 i = 0; i < Choices.Num(); i++)
	{
		if (Choices[i].IsEmptyOrWhitespace())
		{
			continue;
		}

		if (UCommonTextButton* NewButton = WidgetTree->ConstructWidget<UCommonTextButton>())
		{
			ButtonContainer->AddChild(NewButton);
			NewButton->SetButtonText(Choices[i]);
			NewButton->SetStyle(ChoiceButtonStyle);
			NewButton->SetPadding(ChoiceButtonPadding);
			NewButton->OnClicked().AddUObject(this, &UToroDialogWidget::OnOptionClicked, i);
			Buttons[i] = NewButton;
		}
		else
		{
			TORO_LOG_WARNING(TEXT("Failed to create dialog button for choice '%s'"), *Choices[i].ToString());
		}
	}
}

void UToroDialogWidget::SetupAutoSelection(int32 Index, float Time, const TArray<FText>& Choices)
{
	AutoSelectIndex = Index;
	AutoSelectTimer = FMath::Max(Time, 3.0f);
	AutoSelectChoice = Choices[Index];
}

void UToroDialogWidget::OnOptionClicked(int32 Source)
{
	if (IsActivated())
	{
		SelectedIndex = Source;

		DeactivateWidget();
		NativeDelegate.ExecuteIfBound(SelectedIndex);
		BPDelegate.ExecuteIfBound(SelectedIndex);
	}
}

bool UToroDialogWidget::NativeOnHandleBackAction()
{
	if (!bIsBackHandler || !IsActivated())
	{
		return false;
	}

	OnOptionClicked(INDEX_NONE);
	return true;
}

void UToroDialogWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!IsActivated() || AutoSelectIndex < 0 || AutoSelectTimer == 0.0f)
	{
		return;
	}

	if ((AutoSelectTimer -= InDeltaTime) <= 0.0f)
	{
		OnOptionClicked(AutoSelectIndex);
	}
	else if (UCommonTextButton* AutoSelectButton = Buttons[AutoSelectIndex])
	{
		AutoSelectButton->SetButtonText(FText::Format(
			INVTEXT("{0} ({1}s)"),
			AutoSelectChoice,
			FText::AsNumber(FMath::CeilToInt(AutoSelectTimer))
		));
	}
}
