// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserSettingEntry.h"
#include "Utilities/ToroLogging.h"

struct IUserSettingSelection
{
	virtual ~IUserSettingSelection() = default;
	virtual int32 GetOptionCount() const = 0;
	virtual FText GetOptionLabel(int32 Index) const = 0;
	virtual int32 GetSelectedIndex() const = 0;
	virtual void SelectIndex(int32 Index) = 0;
};

template<typename T>
struct TUserSettingSelection final : TUserValueSetting<TUserSettingSelection<T>, T>, IUserSettingSelection
{
	using Super = TUserValueSetting<TUserSettingSelection<T>, T>;

	struct FOption
	{
		T Value;
		FText Label;
	};

	explicit TUserSettingSelection(FGameplayTag Key)
		: Super(Key)
	{}

	TUserSettingSelection& Option(T Value, FText Label)
	{
		if (Options.ContainsByPredicate([this, &Value](const FOption& Existing)
		{
			return this->AreValuesEqual(Existing.Value, Value);
		}))
		{
			TORO_LOG_ERROR(TEXT("Option with label %s already exist. New entry ignored."), *Label.ToString());
		}
		else
		{
			Options.Add(FOption{MoveTemp(Value), MoveTemp(Label)});
		}

		return *this;
	}

	TUserSettingSelection& FromArray(const TArray<T>& Values, TFunctionRef<FText(const T&)> MakeLabel)
	{
		Options.Empty();
		Options.Reserve(Values.Num());
		for (const T& Value : Values)
		{
			Option(Value, MakeLabel(Value));
		}

		return *this;
	}

	virtual int32 GetOptionCount() const override
	{
		return Options.Num();
	}

	virtual FText GetOptionLabel(int32 Index) const override
	{
		return Options.IsValidIndex(Index) ? Options[Index].Label : FText::GetEmpty();
	}

	virtual int32 GetSelectedIndex() const override
	{
		const T Current = this->GetValue();
		return Options.IndexOfByPredicate([this, &Current](const FOption& Option)
		{
			return this->AreValuesEqual(Option.Value, Current);
		});
	}

	virtual void SelectIndex(int32 Index) override
	{
		if (Options.IsValidIndex(Index) && this->IsAvailable())
		{
			const T Value = Options[Index].Value;
			this->SetValue(Value);
		}
	}

	virtual EUserSettingControl GetControlType() const override
	{
		return EUserSettingControl::Selection;
	}

	virtual IUserSettingSelection* AsSelection() override
	{
		return this;
	}

private:

	TArray<FOption> Options;
};