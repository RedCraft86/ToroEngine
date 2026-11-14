// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserSettingEntry.h"

enum class EUserSettingSliderType : uint8
{
	Live,
	Commit
};

class FUserSettingSlider final : public TUserValueSetting<FUserSettingSlider, float>
{
public:

	explicit FUserSettingSlider(FGameplayTag Key)
		: TUserValueSetting(Key)
	{}

	FUserSettingSlider& Delta(float Value)
	{
		check(FMath::IsFinite(Value) && Value >= 0.0f);
		Step = Value;
		return *this;
	}

	FUserSettingSlider& NumDecimal(int32 Value)
	{
		check(Value >= 0);
		DecimalCount = Value;
		return *this;
	}

	FUserSettingSlider& Range(float Min, float Max)
	{
		check(FMath::IsFinite(Min) && FMath::IsFinite(Max) && Min <= Max);
		MinValue = Min; MaxValue = Max;
		return *this;
	}

	FUserSettingSlider& Type(EUserSettingSliderType Value)
	{
		SliderType = Value;
		return *this;
	}

	virtual EUserSettingControl GetControlType() const override
	{
		return EUserSettingControl::Slider;
	}

	float GetDelta() const { return Step; }
	float GetMin() const { return MinValue; }
	float GetMax() const { return MaxValue; }
	int32 GetNumDecimals() const { return DecimalCount; }
	EUserSettingSliderType GetSliderType() const
	{
		return SliderType;
	}

private:

	float Step = 0.1f;
	float MinValue = 0.0f;
	float MaxValue = 1.0f;
	int32 DecimalCount = 1;
	EUserSettingSliderType SliderType = EUserSettingSliderType::Live;
};