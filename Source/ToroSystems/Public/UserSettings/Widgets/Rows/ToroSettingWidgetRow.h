// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CommonUserWidget.h"
#include "UserSettings/Registration/UserSettingMetadata.h"
#include "UserSettings/Registration/EntryTypes/UserSettingEntry.h"
#include "ToroSettingWidgetRow.generated.h"

class UTextBlock;
class UCommonButtonBase;
class UToroSettingsWidget;

UCLASS(Abstract, Blueprintable, BlueprintType, NotPlaceable)
class TOROSYSTEMS_API UToroSettingWidgetRow : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	UToroSettingWidgetRow();

	virtual void InitSetting(const FUserSettingEntry* InEntry);
	void InitMetadata(UToroSettingsWidget* Owner, const FUserSettingMetadata& InMetadata);
	void ClearSetting();

protected:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> NameBlock;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> ResetButton;

	bool bRefreshing;

	virtual void OnRefreshUI() {}

	UFUNCTION()
	void RefreshUI();
	virtual void OnResetClicked();

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

private:

	bool bShowingMetadata;
	bool bDanglingMetadata;
	FUserSettingMetadata Metadata;
	TWeakObjectPtr<UToroSettingsWidget> OwnerWidget;
	const FUserSettingEntry* Entry = nullptr;

	void SetShowMetadata(bool bHovered);
};
