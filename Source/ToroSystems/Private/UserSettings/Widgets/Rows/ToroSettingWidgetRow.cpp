// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/Widgets/Rows/ToroSettingWidgetRow.h"
#include "UserSettings/Widgets/ToroSettingsWidget.h"
#include "CommonButtonBase.h"

UToroSettingWidgetRow::UToroSettingWidgetRow(): bRefreshing(false)
{
	SetVisibilityInternal(ESlateVisibility::Visible);
}

void UToroSettingWidgetRow::InitSetting(const FUserSettingEntry* InEntry)
{
	if (InEntry)
	{
		SetVisibility(InEntry->IsAvailable() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	OnRefreshUI();
}

void UToroSettingWidgetRow::InitMetadata(UToroSettingsWidget* Owner, const FUserSettingMetadata& InMetadata)
{
	if (IsValid(Owner) && Owner->UserSettings.IsValid())
	{
		OwnerWidget = Owner;
		Metadata = InMetadata;
		Owner->UserSettings->GetOnUIUpdate().AddDynamic(this, &UToroSettingWidgetRow::OnRefreshUI);
	}
}

void UToroSettingWidgetRow::OnResetClicked()
{
	OnRefreshUI();
}

void UToroSettingWidgetRow::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsValid(ResetButton))
	{
		ResetButton->OnClicked().RemoveAll(this);
		ResetButton->OnClicked().AddUObject(this, &UToroSettingWidgetRow::OnResetClicked);
	}
}

void UToroSettingWidgetRow::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	if (OwnerWidget.IsValid())
	{
		OwnerWidget->MetadataStack->Add(this, Metadata);
	}
}

void UToroSettingWidgetRow::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (OwnerWidget.IsValid())
	{
		OwnerWidget->MetadataStack->Remove(this);
	}
}
