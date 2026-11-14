// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroSettingWidgetRow.h"
#include "ToroSettingToggleRow.generated.h"

class UCommonButtonBase;
class FUserSettingToggle;

UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API UToroSettingToggleRow final : public UToroSettingWidgetRow
{
	GENERATED_BODY()

public:

	virtual void InitSetting(const FUserSettingEntry* InEntry) override;

private:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> ToggleButton;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> ToggleAnimation;

	const FUserSettingToggle* Setting = nullptr;

	void OnToggleClicked();
	virtual void OnRefreshUI() override;
	virtual void OnResetClicked() override;
	virtual void NativeConstruct() override;
};
