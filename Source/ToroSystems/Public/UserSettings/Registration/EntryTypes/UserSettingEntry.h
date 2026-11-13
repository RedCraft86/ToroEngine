// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameplayTagContainer.h"

class UToroGameUserSettings;

#define LOCTEXT_NAMESPACE "ToroEngine"

enum class EUserSettingControl : uint8
{
	Toggle,
	Slider,
	Selection,
	Action
};

namespace EUserSettingImpact
{
	enum Type : uint8
	{
		None = 0,
		Low = 1,
		Medium = 2,
		High = 3
	};

	inline FText ToText(Type InImpact)
	{
		switch (InImpact)
		{
		case EUserSettingImpact::None: return LOCTEXT("PerformanceImpactNone", "None");
		case EUserSettingImpact::Low: return LOCTEXT("PerformanceImpactLow", "Low");
		case EUserSettingImpact::Medium: return LOCTEXT("PerformanceImpactMedium", "Medium");
		case EUserSettingImpact::High: return LOCTEXT("PerformanceImpactHigh", "High");
		default: return LOCTEXT("PerformanceImpactUnknown", "Unknown");
		}
	}
}

struct IUserSettingSelection;

struct FUserSettingEntry
{
	FUserSettingEntry() = delete;
	FUserSettingEntry(const FUserSettingEntry&) = delete;
	FUserSettingEntry& operator=(const FUserSettingEntry&) = delete;

	explicit FUserSettingEntry(const FGameplayTag& InKey)
		: Key(InKey)
	{}

	virtual ~FUserSettingEntry() = default;

	virtual EUserSettingControl GetControlType() const = 0;

	virtual IUserSettingSelection* AsSelection()
	{
		return nullptr;
	}

	virtual bool IsAvailable() const
	{
		return !EnabledPredicate || EnabledPredicate();
	}

	FGameplayTag GetKey() const { return Key; }
	const FText& GetDisplayName() const { return DisplayNameValue; }
	const FText& GetDescription() const { return DescriptionValue; }
	EUserSettingImpact::Type GetImpact() const { return PerformanceImpact; }

protected:

	FGameplayTag Key;
	FText DisplayNameValue;
	FText DescriptionValue;
	EUserSettingImpact::Type PerformanceImpact = EUserSettingImpact::None;
	TFunction<bool()> EnabledPredicate;
};

template <typename Derived>
struct TUserSettingEntry : FUserSettingEntry
{
	explicit TUserSettingEntry(const FGameplayTag& InKey)
		: FUserSettingEntry(InKey)
	{}

	Derived& DisplayName(FText Text)
	{
		DisplayNameValue = MoveTemp(Text);
		return Self();
	}

	Derived& Description(FText Text)
	{
		DescriptionValue = MoveTemp(Text);
		return Self();
	}

	Derived& Impact(EUserSettingImpact::Type Value)
	{
		PerformanceImpact = Value;
		return Self();
	}

	Derived& IsEnabled(TFunction<bool()> Predicate)
	{
		EnabledPredicate = MoveTemp(Predicate);
		return Self();
	}

protected:

	Derived& Self()
	{
		return static_cast<Derived&>(*this);
	}
};

template <typename Derived, typename T>
struct TUserValueSetting : TUserSettingEntry<Derived>
{
	template <typename ObjectType>
	using FGetter = TFunction<T(const ObjectType&, const FUserSettingEntry&)>;

	template <typename ObjectType>
	using FSetter = TFunction<void(ObjectType&, const FUserSettingEntry&, const T&)>;

	using FComparator = TFunction<bool(const T&, const T&)>;

	explicit TUserValueSetting(FGameplayTag Key)
		: TUserSettingEntry<Derived>(Key)
	{}

	Derived& Default(T Value)
	{
		DefaultValue = MoveTemp(Value);
		return this->Self();
	}

	Derived& Comparator(FComparator Callback)
	{
		CompareValues = MoveTemp(Callback);
		return this->Self();
	}

	template <typename ObjectType>
	Derived& BindCallbacks(ObjectType* Object,
		TIdentity_T<FGetter<ObjectType>> Getter,
		TIdentity_T<FSetter<ObjectType>> Setter)
	{
		static_assert(TIsDerivedFrom<ObjectType, UToroGameUserSettings>::Value,
			"ObjectType must derive from UToroGameUserSettings");

		const TWeakObjectPtr<ObjectType> WeakObject(Object);
		BindingIsValid = [WeakObject]() -> bool
		{
			return WeakObject.IsValid();
		};

		ReadValue = [WeakObject, Getter = MoveTemp(Getter)](const FUserSettingEntry& Entry) -> T
		{
			if (ObjectType* Target = WeakObject.Get())
			{
				return Getter(*Target, Entry);
			}

			return T{};
		};

		WriteValue = [WeakObject, Setter = MoveTemp(Setter)](const FUserSettingEntry& Entry, const T& Value)
		{
			if (ObjectType* Target = WeakObject.Get())
			{
				Setter(*Target, Entry, Value);
			}
		};

		return this->Self();
	}

	template <typename ObjectType, typename GetterType, typename SetterType>
	Derived& Bind(ObjectType* Object, GetterType Getter, SetterType Setter)
	{
		static_assert(TIsDerivedFrom<ObjectType, UToroGameUserSettings>::Value,
			"ObjectType must derive from UToroGameUserSettings");

		return BindCallbacks<ObjectType>(Object,
			[Getter](const ObjectType& Target, const FUserSettingEntry&) -> T
			{
				return (Target.*Getter)();
			},
			[Setter](ObjectType& Target, const FUserSettingEntry&, const T& Value)
			{
				(Target.*Setter)(Value);
			}
		);
	}

	virtual bool IsAvailable() const override
	{
		return TUserSettingEntry<Derived>::IsAvailable()
			&& BindingIsValid && BindingIsValid();
	}

	bool AreValuesEqual(const T& A, const T& B) const
	{
		return CompareValues ? CompareValues(A, B) : A == B;
	}

	T GetValue() const
	{
		return (IsAvailable() && ReadValue) ? ReadValue(*this) : T{};
	}

	void SetValue(const T& NewValue)
	{
		if (IsAvailable() && WriteValue)
		{
			WriteValue(*this, NewValue);
		}
	}

	void ResetValue()
	{
		SetValue(DefaultValue);
	}

	bool DiffersFromDefault()
	{
		return IsAvailable() && ReadValue && !AreValuesEqual(ReadValue(*this), DefaultValue);
	}

private:

	T DefaultValue;
	FComparator CompareValues;
	TFunction<bool()> BindingIsValid;
	TFunction<T(const FUserSettingEntry&)> ReadValue;
	TFunction<void(const FUserSettingEntry&, const T&)> WriteValue;
};

#undef LOCTEXT_NAMESPACE
