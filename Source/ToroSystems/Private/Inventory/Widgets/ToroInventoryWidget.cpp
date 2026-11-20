// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Inventory/Widgets/ToroInventoryWidget.h"
#include "Inventory/Widgets/ToroInventoryItemEntry.h"
#include "Inventory/Widgets/ToroInventoryArchiveEntry.h"
#include "Inventory/Widgets/ToroInventoryEntryWidget.h"
#include "UserInterface/Elements/CommonTextButton.h"
#include "UserInterface/UserInterfaceConfig.h"
#include "Libraries/ToroWidgetLibrary.h"
#include "Inventory/InventoryBehavior.h"
#include "Inventory/ItemDefinition.h"
#include "Utilities/ToroLogging.h"
#include "Framework/ToroPlayerHUD.h"
#include "Animation/WidgetAnimation.h"
#include "CommonAnimatedSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "GameFramework/Pawn.h"
#include "CommonButtonBase.h"

UToroInventoryWidget::UToroInventoryWidget()
{
	bPausesGame = true;
}

bool UToroInventoryWidget::Push(const UObject* ContextObject, int32 PlayerIdx)
{
	const AToroPlayerHUD* HUD = AToroPlayerHUD::Get(ContextObject, PlayerIdx);
	if (!IsValid(HUD) || !HUD->HasMasterWidget())
	{
		TORO_REPORT_ERROR(TEXT("Failed to push Inventory Widget: HUD or MasterWidget is unavailable."));
		return false;
	}

	const FUserInterfaceConfig* Config = FUserInterfaceConfig::Get();
	const TSubclassOf<UToroInventoryWidget> Class = Config ? Config->InventoryWidgetclass.LoadSynchronous() : nullptr;
	if (!IsValid(Class) || Class->HasAnyClassFlags(CLASS_Abstract))
	{
		TORO_REPORT_ERROR(TEXT("Failed to push Inventory Widget: Failed to obtain config or got an invalid class."));
		return false;
	}

	if (HUD->AddWidgetByClass(EToroMasterWidgetType::Menu, Class))
	{
		return true;
	}

	TORO_REPORT_ERROR(TEXT("Failed to push Inventory Widget: Could not create or add widget."));
	return false;
}

void UToroInventoryWidget::BuildEntries()
{
	if (!Inventory.IsValid())
	{
		DeactivateWidget();
		return;
	}

	for (const auto& Entry : Entries)
	{
		if (Entry.IsValid())
		{
			Entry->GetOnSelected().RemoveAll(this);
		}
	}
	Entries.Empty();
	ItemContainer->ClearChildren();
	ArchiveContainer->ClearChildren();

	TArray<const UItemDefinition*> Items;
	Inventory->GetItems(Items);
	for (const UItemDefinition* Item : Items)
	{
		if (!IsValid(Item) || !Item->IsDefinitionValid())
		{
			continue;
		}

		TSubclassOf<UToroInventoryEntryWidget> EntryClass = ItemEntryClass;
		if (Item->Fragment.GetScriptStruct() == FItemTextFragment::StaticStruct())
		{
			EntryClass = ArchiveEntryClass;
		}

		if (UToroInventoryEntryWidget* Entry = CreateWidget<UToroInventoryEntryWidget>(this, EntryClass))
		{
			if (EntryClass == ItemEntryClass)
			{
				ItemContainer->AddChild(Entry);
			}
			else
			{
				ArchiveContainer->AddChild(Entry);
			}

			Entry->Init(Inventory.Get(), Item);
			if (!IsValid(Entry->GetItem()))
			{
				Entry->RemoveFromParent();
				continue;
			}

			Entry->GetOnSelected().AddUObject(this, &UToroInventoryWidget::OnEntrySelected);
			Entries.Add(Entry);
		}
	}
}

void UToroInventoryWidget::UpdateContent(const UItemDefinition* Item) const
{
	if (!IsValid(Item))
	{
		NameText->SetText(FText::GetEmpty());
		DescText->SetText(FText::GetEmpty());
		IconImage->SetBrushFromTexture(nullptr);
		IconImage->SetVisibility(ESlateVisibility::Collapsed);
		EquipButton->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	NameText->SetText(Item->DisplayName);
	DescText->SetText(Item->Description);
	if (UTexture2D* Image = Item->Icon.LoadSynchronous())
	{
		IconImage->SetBrushFromTexture(Image);
		IconImage->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		IconImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (const FItemTextFragment* Fragment = Item->Fragment.GetPtr<FItemTextFragment>())
	{
		TitleText->SetText(Fragment->Title);
		ContentText->SetText(Fragment->BodyText);
		if (UTexture2D* Image = Fragment->ContentImage.LoadSynchronous())
		{
			ArchiveImage->SetBrushFromTexture(Image);
			ArchiveImage->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ArchiveImage->SetBrushFromTexture(nullptr);
			ArchiveImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (Item->Fragment.GetPtr<FItemEquipmentFragment>())
	{
		EquipButton->SetVisibility(ESlateVisibility::Visible);
		EquipButton->SetButtonText(Inventory->GetEquipment() == Item ? INVTEXT("Unequip") : INVTEXT("Equip"));
	}
	else
	{
		EquipButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UToroInventoryWidget::OnCloseClicked()
{
	DeactivateWidget();
}

void UToroInventoryWidget::SetArchivePanelOpen(bool bOpen)
{
	if (bArchivePanelOpen == bOpen)
	{
		return;
	}
	bArchivePanelOpen = bOpen;
	if (bOpen)
	{
		PlayAnimationForward(ArchiveAnimation);
	}
	else
	{
		PlayAnimationReverse(ArchiveAnimation);
	}
}

void UToroInventoryWidget::OnItemTab(bool bSelected)
{
	if (bUpdatingSelection)
	{
		return;
	}

	if (bSelected)
	{
		{
			TGuardValue<bool> SelectionGuard(bUpdatingSelection, true);
			ItemsTab->SetIsSelected(true, false);
			ArchivesTab->ClearSelection();
		}
		EntrySwitcher->SetActiveWidgetIndex(0);
		SetArchivePanelOpen(false);

		for (const auto& Entry : Entries)
		{
			if (Entry.IsValid() && Entry->IsA<UToroInventoryArchiveEntry>() == false)
			{
				OnEntrySelected(Entry.Get());
				return;
			}
		}

		for (const auto& Entry : Entries)
		{
			if (Entry.IsValid())
			{
				Entry->SetIsSelected(false);
			}
		}

		SelectedItem.Reset();
		UpdateContent(nullptr);
	}
}

void UToroInventoryWidget::OnArchiveTab(bool bSelected)
{
	if (bUpdatingSelection)
	{
		return;
	}

	if (bSelected)
	{
		{
			TGuardValue<bool> SelectionGuard(bUpdatingSelection, true);
			ItemsTab->ClearSelection();
			ArchivesTab->SetIsSelected(true, false);
		}

		EntrySwitcher->SetActiveWidgetIndex(1);
		SetArchivePanelOpen(true);

		for (const auto& Entry : Entries)
		{
			if (Entry.IsValid() && Entry->IsA<UToroInventoryArchiveEntry>() == true)
			{
				OnEntrySelected(Entry.Get());
				return;
			}
		}

		for (const auto& Entry : Entries)
		{
			if (Entry.IsValid())
			{
				Entry->SetIsSelected(false);
			}
		}

		SelectedItem.Reset();
		UpdateContent(nullptr);
	}
}

void UToroInventoryWidget::OnEntrySelected(const UToroInventoryEntryWidget* Entry)
{
	if (bUpdatingSelection || !IsValid(Entry) || !IsValid(Entry->GetItem()) || !Inventory.IsValid())
	{
		return;
	}

	TGuardValue<bool> SelectionGuard(bUpdatingSelection, true);
	const bool bArchive = Entry->IsA<UToroInventoryArchiveEntry>();
	if (bArchive)
	{
		ItemsTab->ClearSelection();
		ArchivesTab->SetIsSelected(true, false);
	}
	else
	{
		ArchivesTab->ClearSelection();
		ItemsTab->SetIsSelected(true, false);
	}
	EntrySwitcher->SetActiveWidgetIndex(bArchive ? 1 : 0);
	SetArchivePanelOpen(bArchive);

	for (const TWeakObjectPtr<UToroInventoryEntryWidget>& Inst : Entries)
	{
		if (Inst.IsValid())
		{
			Inst->SetIsSelected(Inst.Get() == Entry);
		}
	}

	SelectedItem = Entry->GetItem();
	UpdateContent(SelectedItem.Get());
}

void UToroInventoryWidget::OnEquipmentClicked() const
{
	if (!SelectedItem.IsValid() || !Inventory.IsValid())
	{
		return;
	}

	if (Inventory->GetEquipment() == SelectedItem)
	{
		Inventory->UnequipItem();
	}
	else
	{
		Inventory->EquipItem(SelectedItem.Get());
	}

	for (const auto& Entry : Entries)
	{
		if (UToroInventoryItemEntry* ItemEntry = Cast<UToroInventoryItemEntry>(Entry.Get()))
		{
			ItemEntry->RefreshEquipmentMarker();
		}
	}
	UpdateContent(SelectedItem.Get());
}

void UToroInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const APawn* Pawn = GetOwningPlayerPawn();
	Inventory = IsValid(Pawn) ? Pawn->GetComponentByClass<UInventoryBehavior>() : nullptr;
	if (!Inventory.IsValid()
		|| !IsValid(CloseButton)
		|| !IsValid(ItemsTab)
		|| !IsValid(ArchivesTab)
		|| !IsValid(EntrySwitcher)
		|| !IsValid(ItemContainer)
		|| !IsValid(ArchiveContainer)
		|| !IsValid(NameText)
		|| !IsValid(DescText)
		|| !IsValid(IconImage)
		|| !IsValid(TitleText)
		|| !IsValid(ContentText)
		|| !IsValid(ArchiveImage)
		|| !IsValid(EquipButton)
		|| !IsValid(ArchiveAnimation))
	{
		Inventory.Reset();
		DeactivateWidget();
		return;
	}

	ItemsTab->OnIsSelectedChanged().RemoveAll(this);
	ArchivesTab->OnIsSelectedChanged().RemoveAll(this);
	EquipButton->OnClicked().RemoveAll(this);
	SelectedItem.Reset();
	UpdateContent(nullptr);

	TitleText->SetText(FText::GetEmpty());
	ContentText->SetText(FText::GetEmpty());
	ArchiveImage->SetBrushFromTexture(nullptr);
	ArchiveImage->SetVisibility(ESlateVisibility::Collapsed);

	UToroWidgetLibrary::InitWidgetAnimation(this, ArchiveAnimation);

	bArchivePanelOpen = false;
	SetAnimationCurrentTime(ArchiveAnimation, 0.0f);
	EntrySwitcher->SetActiveWidgetIndex(0);

	CloseButton->SetIsSelectable(false);
	CloseButton->OnClicked().AddUObject(this, &UToroInventoryWidget::OnCloseClicked);

	ItemsTab->SetIsSelectable(true);
	ItemsTab->SetIsInteractableWhenSelected(false);
	ItemsTab->OnIsSelectedChanged().AddUObject(this, &UToroInventoryWidget::OnItemTab);

	ArchivesTab->SetIsSelectable(true);
	ArchivesTab->SetIsInteractableWhenSelected(false);
	ArchivesTab->OnIsSelectedChanged().AddUObject(this, &UToroInventoryWidget::OnArchiveTab);

	EquipButton->SetIsSelectable(false);
	EquipButton->OnClicked().AddUObject(this, &UToroInventoryWidget::OnEquipmentClicked);

	BuildEntries();

	if (Entries.IsEmpty())
	{
		Inventory->OpenArchive.Reset();
		OnItemTab(true);
		return;
	}

	if (Inventory->OpenArchive.IsNull())
	{
		OnEntrySelected(Entries[0].Get());
		return;
	}

	const TWeakObjectPtr<UToroInventoryEntryWidget>* Entry = Entries.FindByPredicate(
		[this](const TWeakObjectPtr<UToroInventoryEntryWidget>& Element) -> bool
		{
			return Element.IsValid() && Element->GetItem() == Inventory->OpenArchive.LoadSynchronous();
		});

	if (Entry && Entry->IsValid())
	{
		OnEntrySelected(Entry->Get());
	}
	else
	{
		OnEntrySelected(Entries[0].Get());
	}

	Inventory->OpenArchive.Reset();
}

void UToroInventoryWidget::NativeDestruct()
{
	if (IsValid(ItemsTab))
	{
		ItemsTab->OnIsSelectedChanged().RemoveAll(this);
	}
	if (IsValid(ArchivesTab))
	{
		ArchivesTab->OnIsSelectedChanged().RemoveAll(this);
	}
	if (IsValid(EquipButton))
	{
		EquipButton->OnClicked().RemoveAll(this);
	}
	for (const auto& Entry : Entries)
	{
		if (Entry.IsValid())
		{
			Entry->GetOnSelected().RemoveAll(this);
		}
	}
	Entries.Empty();
	if (IsValid(ItemContainer))
	{
		ItemContainer->ClearChildren();
	}
	if (IsValid(ArchiveContainer))
	{
		ArchiveContainer->ClearChildren();
	}
	SelectedItem.Reset();
	Inventory.Reset();
	Super::NativeDestruct();
}
