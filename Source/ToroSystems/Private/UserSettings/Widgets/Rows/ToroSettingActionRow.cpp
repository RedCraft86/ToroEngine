// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/Widgets/Rows/ToroSettingActionRow.h"
#include "UserSettings/Registration/EntryTypes/UserSettingAction.h"
#include "CommonButtonBase.h"

void UToroSettingActionRow::InitSetting(const FUserSettingEntry* InEntry)
{
	Setting = (InEntry && InEntry->GetControlType() == EUserSettingControl::Action)
		? static_cast<const FUserSettingAction*>(InEntry)
		: nullptr;

	Super::InitSetting(InEntry);
}

void UToroSettingActionRow::OnExecuteClicked() const
{
	if (Setting)
	{
		Setting->Execute();
	}
}

void UToroSettingActionRow::OnRefreshUI()
{
	TGuardValue<bool> RefreshGuard(bRefreshing, true);
	if (!Setting || !ExecuteButton)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UToroSettingActionRow::NativeConstruct()
{
	Super::NativeConstruct();
	if (!IsValid(ExecuteButton))
	{
		RemoveFromParent();
		return;
	}

	ExecuteButton->OnClicked().RemoveAll(this);
	ExecuteButton->OnClicked().AddUObject(this, &UToroSettingActionRow::OnExecuteClicked);
}
