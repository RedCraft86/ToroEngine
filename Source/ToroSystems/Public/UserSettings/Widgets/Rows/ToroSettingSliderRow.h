// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroSettingWidgetRow.h"
#include "ToroSettingSliderRow.generated.h"

class USpinBox;
class FUserSettingSlider;

UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API UToroSettingSliderRow final : public UToroSettingWidgetRow
{
	GENERATED_BODY()

public:

	virtual void InitSetting(const FUserSettingEntry* InEntry) override;

private:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<USpinBox> SliderBox;

	const FUserSettingSlider* Setting = nullptr;

	UFUNCTION() void OnSliderFinished(float InValue);
	UFUNCTION() void OnValueEntered(float InValue, ETextCommit::Type Method);
	UFUNCTION() void OnSliderChanged(float InValue) const;

	virtual void OnRefreshUI() override;
	virtual void OnResetClicked() override;
	virtual void NativeConstruct() override;
};
