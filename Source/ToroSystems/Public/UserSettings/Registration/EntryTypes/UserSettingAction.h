// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserSettingEntry.h"

class FUserSettingAction final : public TUserSettingEntry<FUserSettingAction>
{
public:

	template <typename ObjectType>
	using FExecuter = TFunction<void(ObjectType&, const FUserSettingEntry&)>;

	explicit FUserSettingAction(FName Key)
		: TUserSettingEntry(Key)
	{}

	template<typename ObjectType>
	FUserSettingAction& BindCallback(ObjectType* Object,
		TIdentity_T<FExecuter<ObjectType>> Executer)
	{
		static_assert(TIsDerivedFrom<ObjectType, UToroGameUserSettings>::Value,
			"ObjectType must derive from UToroGameUserSettings");

		const TWeakObjectPtr<ObjectType> WeakObject(Object);
		BindingIsValid = [WeakObject]() -> bool
		{
			return WeakObject.IsValid();
		};

		Callback = [WeakObject, Executer = MoveTemp(Executer)](const FUserSettingEntry& Entry)
		{
			if (ObjectType* Target = WeakObject.Get())
			{
				Executer(*Target, Entry);
			}
		};

		return *this;
	}

	template<typename ObjectType, typename MethodType>
	FUserSettingAction& Bind(ObjectType* Object, MethodType Method)
	{
		static_assert(TIsDerivedFrom<ObjectType, UToroGameUserSettings>::Value,
			"ObjectType must derive from UToroGameUserSettings");

		return BindCallback<ObjectType>(Object,
			[Method](ObjectType& Target, const FUserSettingEntry&)
			{
				(Target.*Method)();
			}
		);
	}

	virtual EUserSettingControl GetControlType() const override
	{
		return EUserSettingControl::Action;
	}

	virtual bool IsAvailable() const override
	{
		return TUserSettingEntry::IsAvailable()
			&& BindingIsValid && BindingIsValid();
	}

	void Execute() const
	{
		if (IsAvailable() && Callback)
		{
			Callback(*this);
		}
	}

private:

	TFunction<bool()> BindingIsValid;
	TFunction<void(const FUserSettingEntry&)> Callback;
};