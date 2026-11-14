// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/Widgets/ToroSettingsWidget.h"
#include "UserSettings/Widgets/Rows/ToroSettingToggleRow.h"
#include "UserSettings/Widgets/Rows/ToroSettingSliderRow.h"
#include "UserSettings/Widgets/Rows/ToroSettingSelectionRow.h"
#include "UserSettings/Widgets/Rows/ToroSettingActionRow.h"
#include "UserSettings/Registration/UserSettingsBuilder.h"
#include "UserInterface/Elements/CommonTextButton.h"
#include "UserInterface/UserInterfaceConfig.h"
#include "Framework/ToroPlayerHUD.h"
#include "Components/ExpandableArea.h"
#include "Components/PanelWidget.h"
#include "Components/VerticalBox.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetTree.h"
#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
UToroSettingsWidget::UToroSettingsWidget()
	: TabPadding(2.0f, 4.0f, 2.0f, 4.0f)
	, Registry(MakeUnique<FUserSettingsRegistry>())
{
	bPausesGame = true;
	MetadataStack->OnChanged().AddUObject(this, &UToroSettingsWidget::OnMetadataChanged);
}

bool UToroSettingsWidget::Push(const UObject* ContextObject, int32 PlayerIdx)
{
	const AToroPlayerHUD* HUD = AToroPlayerHUD::Get(ContextObject, PlayerIdx);
	if (!IsValid(HUD) || !HUD->HasMasterWidget())
	{
		TORO_REPORT_ERROR(TEXT("Failed to push Settings Widget: HUD or MasterWidget is unavailable."));
		return false;
	}

	const FUserInterfaceConfig* Config = FUserInterfaceConfig::Get();
	const TSubclassOf<UToroSettingsWidget> Class = Config ? Config->SettingsWidgetClass.LoadSynchronous() : nullptr;
	if (!IsValid(Class) || Class->HasAnyClassFlags(CLASS_Abstract))
	{
		TORO_REPORT_ERROR(TEXT("Failed to push Settings Widget: Failed to obtain config or got an invalid class."));
		return false;
	}

	if (HUD->AddWidgetByClass(EToroMasterWidgetType::Menu, Class))
	{
		return true;
	}

	TORO_REPORT_ERROR(TEXT("Failed to push Settings Widget: Could not create or add widget."));
	return false;
}

void UToroSettingsWidget::ClearRows()
{
	for (const TWeakObjectPtr<UToroSettingWidgetRow>& Row : SettingRows)
	{
		if (Row.IsValid())
		{
			Row->ClearSetting();
		}
	}
	SettingRows.Reset();
}

void UToroSettingsWidget::BuildMenu()
{
	if (UserSettings.IsValid() && WidgetTree && TabContainer && PageContainer)
	{
		TabButtons.Empty();
		TabContainer->ClearChildren();
		PageContainer->ClearChildren();

		int32 ButtonIdx = 0;
		for (int32 i = 0; i < Registry->Num(); i++)
		{
			const FUserSettingsPage* Page = (*Registry)[i];
			if (!Page)
			{
				continue;
			}

			UWidget* PageWidget = AddPage(Page);
			if (!PageWidget)
			{
				continue;
			}
			PageContainer->AddChild(PageWidget);

			UCommonTextButton* TabButton = WidgetTree->ConstructWidget<UCommonTextButton>();
			TabButton->OnClicked().AddUObject(this, &UToroSettingsWidget::OnTabClicked, ButtonIdx);
			TabButton->SetButtonText(Page->GetMetadata().DisplayName);
			TabButton->SetIsInteractableWhenSelected(false);
			TabButton->SetIsSelectable(true);
			TabButton->SetStyle(TabStyle);
			TabButton->SetPadding(TabPadding);
			TabContainer->AddChild(TabButton);
			TabButtons.Add(TabButton);

			ButtonIdx++;
		}
	}
}

UWidget* UToroSettingsWidget::AddPage(const FUserSettingsPage* Page)
{
	if (Page && WidgetTree)
	{
		UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>();
		for (int32 i = 0; i < Page->Num(); i++)
		{
			if (UWidget* Section = AddSection((*Page)[i]))
			{
				Scroll->AddChild(Section);
			}
		}

		Scroll->SetAlwaysShowScrollbar(true);
		Scroll->SetAlwaysShowScrollbarTrack(true);
		return Scroll;
	}

	return nullptr;
}

UWidget* UToroSettingsWidget::AddSection(const FUserSettingsSection* Section)
{
	if (Section && WidgetTree)
	{
		FSlateBrush EmptyBrush;
		EmptyBrush.DrawAs = ESlateBrushDrawType::NoDrawType;

		UExpandableArea* Area = WidgetTree->ConstructWidget<UExpandableArea>();
		Area->SetClipping(EWidgetClipping::ClipToBounds);
		Area->SetBorderBrush(EmptyBrush);
		Area->SetStyle(SectionAreaStyle);

		if (UToroSettingWidgetRow* Header = CreateWidget<UToroSettingWidgetRow>(this, SectionHeaderClass))
		{
			Header->InitMetadata(this, Section->GetMetadata());
			Area->SetContentForSlot(TEXT("Header"), Header);
		}

		UVerticalBox* Rows = WidgetTree->ConstructWidget<UVerticalBox>();
		for (int32 i = 0; i < Section->Num(); i++)
		{
			if (UWidget* Row = AddEntry((*Section)[i]))
			{
				Rows->AddChildToVerticalBox(Row);
			}
		}

		Area->SetContentForSlot(TEXT("Body"), Rows);
		Area->SetIsExpanded(true);
		return Area;
	}

	return nullptr;
}

UWidget* UToroSettingsWidget::AddEntry(const FUserSettingEntry* Entry)
{
	if (Entry)
	{
		switch (Entry->GetControlType())
		{
		case EUserSettingControl::Toggle:
			if (UToroSettingToggleRow* Row = CreateWidget<UToroSettingToggleRow>(this, ToggleRowClass))
			{
				Row->InitMetadata(this, Entry->GetMetadata());
				Row->InitSetting(Entry);
				return Row;
			}
			return nullptr;

		case EUserSettingControl::Slider:
			if (UToroSettingSliderRow* Row = CreateWidget<UToroSettingSliderRow>(this, SliderRowClass))
			{
				Row->InitMetadata(this, Entry->GetMetadata());
				Row->InitSetting(Entry);
				return Row;
			}
			return nullptr;

		case EUserSettingControl::Selection:
			if (UToroSettingSelectionRow* Row = CreateWidget<UToroSettingSelectionRow>(this, SelectionRowClass))
			{
				Row->InitMetadata(this, Entry->GetMetadata());
				Row->InitSetting(Entry);
				return Row;
			}
			return nullptr;

		case EUserSettingControl::Action:
			if (UToroSettingActionRow* Row = CreateWidget<UToroSettingActionRow>(this, ActionRowClass))
			{
				Row->InitMetadata(this, Entry->GetMetadata());
				Row->InitSetting(Entry);
				return Row;
			}
			return nullptr;

		default:
			return nullptr;
		}
	}

	return nullptr;
}

void UToroSettingsWidget::OnCloseClicked()
{
	if (UserSettings.IsValid())
	{
		UserSettings->ApplySettings(false);
	}

	DeactivateWidget();
}

void UToroSettingsWidget::OnTabClicked(int32 Index) const
{
	if (TabButtons.IsValidIndex(Index)
		&& Index < PageContainer->GetChildrenCount())
	{
		PageContainer->SetActiveWidgetIndex(Index);
		for (int32 i = 0; i < TabButtons.Num(); i++)
		{
			if (i == Index)
			{
				TabButtons[i]->SetIsSelected(true, false);
			}
			else
			{
				TabButtons[i]->ClearSelection();
			}
		}
	}
}

void UToroSettingsWidget::OnMetadataChanged() const
{
	FUserSettingMetadata Metadata;
	if (const FUserSettingMetadata* MetadataPtr = MetadataStack->GetLast())
	{
		Metadata = *MetadataPtr;
	}
	else if (const FUserSettingsPage* Page = (*Registry)[PageContainer->GetActiveWidgetIndex()])
	{
		Metadata = Page->GetMetadata();
	}

	MetaLabelBlock->SetText(Metadata.DisplayName);
	MetaDescBlock->SetText(Metadata.Description);

	MetaImpactBlock->SetVisibility(ESlateVisibility::Visible);
	switch (Metadata.Impact)
	{
	case EUserSettingImpact::None:
		MetaImpactBlock->SetText(LOCTEXT("UserSettingsImpactNone", "None"));
		MetaImpactBlock->SetColorAndOpacity(FLinearColor::White);
		break;

	case EUserSettingImpact::Low:
		MetaImpactBlock->SetText(LOCTEXT("UserSettingsImpactLow", "Low"));
		MetaImpactBlock->SetColorAndOpacity(FLinearColor::Green);
		break;

	case EUserSettingImpact::Medium:
		MetaImpactBlock->SetText(LOCTEXT("UserSettingsImpactMedium", "Medium"));
		MetaImpactBlock->SetColorAndOpacity(FLinearColor::Yellow);
		break;

	case EUserSettingImpact::High:
		MetaImpactBlock->SetText(LOCTEXT("UserSettingsImpactHigh", "High"));
		MetaImpactBlock->SetColorAndOpacity(FLinearColor::Red);
		break;

	default:
		MetaImpactBlock->SetVisibility(ESlateVisibility::Collapsed);
		break;
	}
}

void UToroSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!WidgetTree || !IsValid(CloseButton) || !IsValid(TabContainer) || !IsValid(PageContainer))
	{
		DeactivateWidget();
		return;
	}

	CloseButton->OnClicked().RemoveAll(this);
	CloseButton->OnClicked().AddUObject(this, &UToroSettingsWidget::OnCloseClicked);

	ClearRows();
	Registry->Reset();
	UserSettings = UToroGameUserSettings::Get();
	if (UserSettings.IsValid())
	{
		UUserSettingsBuilder::Build(*Registry, *UserSettings.Get());
	}

	BuildMenu();
	if (!TabButtons.IsEmpty())
	{
		PageContainer->SetDisableTransitionAnimation(true);
		OnTabClicked(0);
		PageContainer->SetDisableTransitionAnimation(false);
	}

	OnMetadataChanged();
}

void UToroSettingsWidget::NativeDestruct()
{
	ClearRows();
	Super::NativeDestruct();
}
#undef LOCTEXT_NAMESPACE
