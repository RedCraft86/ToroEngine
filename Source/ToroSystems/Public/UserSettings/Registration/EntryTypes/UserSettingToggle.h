// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserSettingEntry.h"

class FUserSettingToggle final : public TUserValueSetting<FUserSettingToggle, bool>
{
public:

	explicit FUserSettingToggle(FGameplayTag Key)
		: TUserValueSetting(Key)
	{}

	virtual EUserSettingControl GetControlType() const override
	{
		return EUserSettingControl::Toggle;
	}

	void Toggle() const
	{
		if (IsAvailable())
		{
			SetValue(!GetValue());
		}
	}
};
