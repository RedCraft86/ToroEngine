// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Components/TextBlock.h"
#include "DataTypes/Accumulators.h"
#include "UserInterface/Bases/ToroMenuWidget.h"
#include "UserSettings/Registration/UserSettingMetadata.h"
#include "UserSettings/Registration/UserSettingsRegistry.h"
#include "UserSettings/ToroGameUserSettings.h"
#include "ToroSettingsWidget.generated.h"

class PanelWidget;
class UCommonButtonBase;
class UCommonTextButton;
class UCommonAnimatedSwitcher;
class UToroSettingWidgetRow;
class UToroSettingToggleRow;
class UToroSettingSliderRow;
class UToroSettingSelectionRow;
class UToroSettingActionRow;
class UCommonButtonStyle;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType)
class UToroSettingsWidget final : public UToroMenuWidget
{
	GENERATED_BODY()

	friend class UToroSettingWidgetRow;

public:

	UToroSettingsWidget();

	UFUNCTION(BlueprintCallable, Category = UserInterface,
		DisplayName = "Push Settings", meta = (WorldContext = ContextObject))
	static bool Push(const UObject* ContextObject, int32 PlayerIdx = 0);

private:

	UPROPERTY(EditAnywhere, Category = Appearance)
	FExpandableAreaStyle SectionAreaStyle;

	UPROPERTY(EditAnywhere, Category = "Appearance|TabButton")
	TSubclassOf<UCommonButtonStyle> TabStyle;

	UPROPERTY(EditAnywhere, Category = "Appearance|TabButton")
	FMargin TabPadding;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (AllowAbstract = false,
		DisallowedClasses = "/Script/ToroSystems.ToroSettingToggleRow,/Script/ToroSystems.ToroSettingSliderRow,/Script/ToroSystems.ToroSettingSelectionRow,/Script/ToroSystems.ToroSettingActionRow"
	))
	TSubclassOf<UToroSettingWidgetRow> SectionHeaderClass;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (AllowAbstract = false))
	TSubclassOf<UToroSettingToggleRow> ToggleRowClass;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (AllowAbstract = false))
	TSubclassOf<UToroSettingSliderRow> SliderRowClass;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (AllowAbstract = false))
	TSubclassOf<UToroSettingSelectionRow> SelectionRowClass;

	UPROPERTY(EditAnywhere, Category = Widget, meta = (AllowAbstract = false))
	TSubclassOf<UToroSettingActionRow> ActionRowClass;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CloseButton;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPanelWidget> TabContainer;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonAnimatedSwitcher> PageContainer;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> MetaLabelBlock;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> MetaDescBlock;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> MetaImpactBlock;

	TUniquePtr<FUserSettingsRegistry> Registry;
	TWeakObjectPtr<UToroGameUserSettings> UserSettings;
	TArray<TWeakObjectPtr<UCommonTextButton>> TabButtons;
	TArray<TWeakObjectPtr<UToroSettingWidgetRow>> SettingRows;
	TSharedRef<TOrderedAccumulator<FUserSettingMetadata>> MetadataStack
		= TOrderedAccumulator<FUserSettingMetadata>::Make();

	void ClearRows();
	void BuildMenu();
	UWidget* AddPage(const FUserSettingsPage* Page);
	UWidget* AddSection(const FUserSettingsSection* Section);
	UWidget* AddEntry(const FUserSettingEntry* Entry);

	void OnCloseClicked();
	void OnTabClicked(int32 Index) const;
	void OnPageChanged(UWidget* Page, int32 Index) const;
	void OnMetadataChanged() const;

	virtual bool NativeOnHandleBackAction() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
