// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CommonUserWidget.h"
#include "UserSettings/Registration/UserSettingMetadata.h"
#include "UserSettings/Registration/EntryTypes/UserSettingEntry.h"
#include "ToroSettingWidgetRow.generated.h"

class UCommonButtonBase;
class UToroSettingsWidget;

UCLASS(Blueprintable, BlueprintType, NotPlaceable)
class TOROSYSTEMS_API UToroSettingWidgetRow : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	UToroSettingWidgetRow();

	virtual void InitSetting(const FUserSettingEntry* InEntry);
	void InitMetadata(UToroSettingsWidget* Owner, const FUserSettingMetadata& InMetadata);

protected:

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> ResetButton;

	bool bRefreshing;

	UFUNCTION()
	virtual void OnRefreshUI() {}
	virtual void OnResetClicked();

	virtual void NativeConstruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

private:

	FUserSettingMetadata Metadata;
	TWeakObjectPtr<UToroSettingsWidget> OwnerWidget;
};
