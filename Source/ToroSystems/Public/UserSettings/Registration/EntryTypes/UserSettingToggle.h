// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserSettingEntry.h"

struct FUserSettingToggle final : TUserValueSetting<FUserSettingToggle, bool>
{
	explicit FUserSettingToggle(FGameplayTag Key)
		: TUserValueSetting(Key)
	{}

	virtual EUserSettingControl GetControlType() const override
	{
		return EUserSettingControl::Toggle;
	}

	void Toggle()
	{
		if (IsAvailable())
		{
			SetValue(!GetValue());
		}
	}
};