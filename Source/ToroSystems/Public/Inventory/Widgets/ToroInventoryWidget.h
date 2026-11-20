// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserInterface/Bases/ToroMenuWidget.h"
#include "ToroInventoryWidget.generated.h"

class UImage;
class UTextBlock;
class UCommonButtonBase;
class UCommonTextButton;
class UInventoryBehavior;
class UToroInventoryEntryWidget;
class UToroInventoryArchiveEntry;
class UToroInventoryItemEntry;
class UCommonAnimatedSwitcher;
class UItemDefinition;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType)
class UToroInventoryWidget final : public UToroMenuWidget
{
	GENERATED_BODY()

public:

	UToroInventoryWidget();

	UFUNCTION(BlueprintCallable, Category = UserInterface,
		DisplayName = "Push Inventory", meta = (WorldContext = ContextObject))
	static TOROSYSTEMS_API bool Push(const UObject* ContextObject, int32 PlayerIdx = 0);

private:

	UPROPERTY(EditAnywhere, Category = Settings)
	TSubclassOf<UToroInventoryItemEntry> ItemEntryClass;

	UPROPERTY(EditAnywhere, Category = Settings)
	TSubclassOf<UToroInventoryArchiveEntry> ArchiveEntryClass;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> ItemsTab;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> ArchivesTab;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonAnimatedSwitcher> EntrySwitcher;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPanelWidget> ItemContainer;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPanelWidget> ArchiveContainer;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> DescText;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> ContentText;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage> ArchiveImage;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonTextButton> EquipButton;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (BindWidgetAnim, AllowPrivateAccess = true))
	TObjectPtr<UWidgetAnimation> ArchiveAnimation;

	UPROPERTY(Transient)
	TWeakObjectPtr<UInventoryBehavior> Inventory;

	UPROPERTY(Transient)
	TWeakObjectPtr<const UItemDefinition> SelectedItem;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UToroInventoryEntryWidget>> Entries;

	bool bUpdatingSelection = false;

	void BuildEntries();
	void UpdateContent(const UItemDefinition* Item) const;

	void OnItemTab(bool bSelected);
	void OnArchiveTab(bool bSelected);
	void OnEntrySelected(const UToroInventoryEntryWidget* Entry);
	void OnEquipmentClicked() const;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
