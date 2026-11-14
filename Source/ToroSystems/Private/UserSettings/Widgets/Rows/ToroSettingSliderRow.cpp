// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/Widgets/Rows/ToroSettingSliderRow.h"
#include "UserSettings/Registration/EntryTypes/UserSettingSlider.h"
#include "Components/SpinBox.h"
#include "CommonButtonBase.h"

void UToroSettingSliderRow::InitSetting(const FUserSettingEntry* InEntry)
{
	Setting = (InEntry && InEntry->GetControlType() == EUserSettingControl::Slider)
		? static_cast<const FUserSettingSlider*>(InEntry)
		: nullptr;

	Super::InitSetting(InEntry);
}

void UToroSettingSliderRow::OnSliderFinished(float InValue)
{
	if (!bRefreshing && Setting)
	{
		Setting->SetValue(InValue);
		RefreshUI();
	}
}

void UToroSettingSliderRow::OnValueEntered(float InValue, ETextCommit::Type)
{
	if (!bRefreshing && Setting)
	{
		Setting->SetValue(InValue);
		RefreshUI();
	}
}

void UToroSettingSliderRow::OnSliderChanged(float InValue) const
{
	if (!bRefreshing && Setting && Setting->GetSliderType() == EUserSettingSliderType::Live)
	{
		Setting->SetValue(InValue);
	}
}

void UToroSettingSliderRow::OnRefreshUI()
{
	TGuardValue<bool> RefreshGuard(bRefreshing, true);
	if (!Setting || !SliderBox)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (ResetButton)
	{
		ResetButton->SetIsEnabled(Setting->CanReset() && Setting->DiffersFromDefault());
		ResetButton->SetVisibility(ResetButton->GetIsEnabled()
			? ESlateVisibility::Visible: ESlateVisibility::Hidden);
	}

	SliderBox->SetMinFractionalDigits(Setting->GetNumDecimals());
	SliderBox->SetMaxFractionalDigits(Setting->GetNumDecimals());
	SliderBox->SetMinValue(Setting->GetMin());
	SliderBox->SetMaxValue(Setting->GetMax());
	SliderBox->SetDelta(Setting->GetDelta());
	SliderBox->SetAlwaysUsesDeltaSnap(true);
	SliderBox->SetValue(Setting->GetValue());
}

void UToroSettingSliderRow::OnResetClicked()
{
	if (Setting)
	{
		Setting->ResetValue();
		Super::OnResetClicked();
	}
}

void UToroSettingSliderRow::NativeConstruct()
{
	Super::NativeConstruct();
	if (!IsValid(SliderBox))
	{
		RemoveFromParent();
		return;
	}

	SliderBox->OnEndSliderMovement.RemoveAll(this);
	SliderBox->OnEndSliderMovement.AddDynamic(this, &UToroSettingSliderRow::OnSliderFinished);

	SliderBox->OnValueCommitted.RemoveAll(this);
	SliderBox->OnValueCommitted.AddDynamic(this, &UToroSettingSliderRow::OnValueEntered);

	SliderBox->OnValueChanged.RemoveAll(this);
	SliderBox->OnValueChanged.AddDynamic(this, &UToroSettingSliderRow::OnSliderChanged);
}
