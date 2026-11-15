// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/Widgets/Rows/ToroSettingWidgetRow.h"
#include "UserSettings/Widgets/ToroSettingsWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Components/TextBlock.h"
#include "CommonButtonBase.h"

UToroSettingWidgetRow::UToroSettingWidgetRow()
	: bRefreshing(false)
	, bShowingMetadata(false)
	, bDanglingMetadata(false)
{
	SetVisibilityInternal(ESlateVisibility::Visible);
}

void UToroSettingWidgetRow::InitSetting(const FUserSettingEntry* InEntry)
{
	Entry = InEntry;
	RefreshUI();
}

void UToroSettingWidgetRow::RefreshUI()
{
	if (Entry)
	{
		SetVisibility(Entry->IsAvailable() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	OnRefreshUI();
}

void UToroSettingWidgetRow::InitMetadata(UToroSettingsWidget* Owner, const FUserSettingMetadata& InMetadata)
{
	if (IsValid(Owner) && Owner->UserSettings.IsValid())
	{
		OwnerWidget = Owner;
		Metadata = InMetadata;
		Owner->SettingRows.Add(this);
		Owner->UserSettings->GetOnUIUpdate().AddUniqueDynamic(this, &UToroSettingWidgetRow::RefreshUI);
		if (NameBlock)
		{
			NameBlock->SetText(Metadata.DisplayName);
		}
	}
}

void UToroSettingWidgetRow::ClearSetting()
{
	if (OwnerWidget.IsValid() && OwnerWidget->UserSettings.IsValid())
	{
		OwnerWidget->UserSettings->GetOnUIUpdate().RemoveDynamic(this, &UToroSettingWidgetRow::RefreshUI);
	}

	bDanglingMetadata = false;
	SetShowMetadata(false);

	OwnerWidget.Reset();
	InitSetting(nullptr);
}

void UToroSettingWidgetRow::OnResetClicked()
{
	RefreshUI();
}

void UToroSettingWidgetRow::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsValid(ResetButton))
	{
		ResetButton->OnClicked().RemoveAll(this);
		ResetButton->OnClicked().AddUObject(this, &UToroSettingWidgetRow::OnResetClicked);
	}

	RefreshUI();
}

void UToroSettingWidgetRow::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (bDanglingMetadata && !MyGeometry.IsUnderLocation(FSlateApplication::Get().GetCursorPos()))
	{
		SetShowMetadata(false);
	}
}

void UToroSettingWidgetRow::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	bDanglingMetadata = false;
	SetShowMetadata(true);
}

void UToroSettingWidgetRow::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if (!GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
	{
		SetShowMetadata(false);
	}
	else
	{
		bDanglingMetadata = true;
	}
}

void UToroSettingWidgetRow::SetShowMetadata(bool bHovered)
{
	if (bShowingMetadata == bHovered)
	{
		return;
	}

	bShowingMetadata = bHovered;
	if (OwnerWidget.IsValid())
	{
		if (bHovered)
		{
			OwnerWidget->MetadataStack->Add(this, Metadata);
		}
		else
		{
			bDanglingMetadata = false;
			OwnerWidget->MetadataStack->Remove(this);
		}
	}
}
