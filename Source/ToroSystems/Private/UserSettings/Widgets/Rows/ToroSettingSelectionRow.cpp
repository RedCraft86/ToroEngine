// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/Widgets/Rows/ToroSettingSelectionRow.h"
#include "UserSettings/Registration/EntryTypes/UserSettingSelection.h"
#include "Components/ComboBoxString.h"
#include "CommonButtonBase.h"
#include "Types/SlateEnums.h"

void UToroSettingSelectionRow::InitSetting(const FUserSettingEntry* InEntry)
{
	Setting = (InEntry && InEntry->GetControlType() == EUserSettingControl::Selection)
		? InEntry->AsSelection()
		: nullptr;

	Super::InitSetting(InEntry);
}

void UToroSettingSelectionRow::OnItemSelected(FString, ESelectInfo::Type Type)
{
	if (!bRefreshing && SelectionBox && Setting && Type != ESelectInfo::Direct)
	{
		Setting->SelectIndex(SelectionBox->GetSelectedIndex());
		RefreshUI();
	}
}

void UToroSettingSelectionRow::OnRefreshUI()
{
	TGuardValue<bool> RefreshGuard(bRefreshing, true);
	if (!Setting || !SelectionBox)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (ResetButton)
	{
		ResetButton->SetIsEnabled(Setting->CanResetSelection());
		ResetButton->SetVisibility(ResetButton->GetIsEnabled()
			? ESlateVisibility::Visible: ESlateVisibility::Hidden);
	}

	SelectionBox->ClearOptions();
	for (int32 i = 0; i < Setting->GetOptionCount(); i++)
	{
		SelectionBox->AddOption(Setting->GetOptionLabel(i).ToString());
	}
	SelectionBox->SetSelectedIndex(Setting->GetSelectedIndex());
}

void UToroSettingSelectionRow::OnResetClicked()
{
	if (Setting)
	{
		Setting->ResetSelection();
		Super::OnResetClicked();
	}
}

void UToroSettingSelectionRow::NativeConstruct()
{
	Super::NativeConstruct();
	if (!IsValid(SelectionBox))
	{
		RemoveFromParent();
		return;
	}

	SelectionBox->OnSelectionChanged.RemoveAll(this);
	SelectionBox->OnSelectionChanged.AddDynamic(this, &UToroSettingSelectionRow::OnItemSelected);
}
