// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroSettingWidgetRow.h"
#include "ToroSettingActionRow.generated.h"

class UCommonButtonBase;
class FUserSettingAction;

UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API UToroSettingActionRow final : public UToroSettingWidgetRow
{
	GENERATED_BODY()

public:

	virtual void InitSetting(const FUserSettingEntry* InEntry) override;

private:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> ExecuteButton;

	const FUserSettingAction* Setting = nullptr;

	void OnExecuteClicked() const;
	void OnRefreshUI() override;
	virtual void NativeConstruct() override;
};
