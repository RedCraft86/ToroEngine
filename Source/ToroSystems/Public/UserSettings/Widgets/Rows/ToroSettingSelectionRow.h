// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroSettingWidgetRow.h"
#include "ToroSettingSelectionRow.generated.h"

class UComboBoxString;
class IUserSettingSelection;

UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API UToroSettingSelectionRow final : public UToroSettingWidgetRow
{
	GENERATED_BODY()

public:

	virtual void InitSetting(const FUserSettingEntry* InEntry) override;

private:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UComboBoxString> SelectionBox;

	const IUserSettingSelection* Setting = nullptr;

	UFUNCTION()
	void OnItemSelected(FString Item, ESelectInfo::Type Type);

	virtual void OnRefreshUI() override;
	virtual void OnResetClicked() override;
	virtual void NativeConstruct() override;
};
