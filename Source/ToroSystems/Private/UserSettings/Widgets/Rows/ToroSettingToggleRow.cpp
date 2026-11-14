// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/Widgets/Rows/ToroSettingToggleRow.h"
#include "UserSettings/Registration/EntryTypes/UserSettingToggle.h"
#include "Libraries/ToroWidgetLibrary.h"
#include "Animation/WidgetAnimation.h"
#include "CommonButtonBase.h"

void UToroSettingToggleRow::InitSetting(const FUserSettingEntry* InEntry)
{
	Setting = (InEntry && InEntry->GetControlType() == EUserSettingControl::Toggle)
		? static_cast<const FUserSettingToggle*>(InEntry)
		: nullptr;

	Super::InitSetting(InEntry);
}

void UToroSettingToggleRow::OnToggleClicked()
{
	if (!bRefreshing && Setting)
	{
		Setting->Toggle();

		if (Setting->GetValue())
		{
			PlayAnimationForward(ToggleAnimation);
		}
		else
		{
			PlayAnimationReverse(ToggleAnimation);
		}

		OnRefreshUI();
	}
}

void UToroSettingToggleRow::OnRefreshUI()
{
	TGuardValue<bool> RefreshGuard(bRefreshing, true);
	if (!Setting || !ToggleButton || !ToggleAnimation)
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

	UToroWidgetLibrary::InitWidgetAnimation(this, ToggleAnimation);

	const bool bWantsToEnd = Setting->GetValue();
	if (!IsAnimationPlaying(ToggleAnimation) || bWantsToEnd != IsAnimationPlayingForward(ToggleAnimation))
	{
		const float TargetTime = bWantsToEnd ? ToggleAnimation->GetEndTime() : ToggleAnimation->GetStartTime();
		SetAnimationCurrentTime(ToggleAnimation, TargetTime);
	}
}

void UToroSettingToggleRow::OnResetClicked()
{
	if (Setting)
	{
		Setting->ResetValue();
		if (Setting->GetValue())
		{
			PlayAnimationForward(ToggleAnimation);
		}
		else
		{
			PlayAnimationReverse(ToggleAnimation);
		}

		Super::OnResetClicked();
	}
}

void UToroSettingToggleRow::NativeConstruct()
{
	Super::NativeConstruct();
	if (!IsValid(ToggleButton) || !IsValid(ToggleAnimation))
	{
		RemoveFromParent();
		return;
	}

	ToggleButton->OnClicked().RemoveAll(this);
	ToggleButton->OnClicked().AddUObject(this, &UToroSettingToggleRow::OnToggleClicked);
}
