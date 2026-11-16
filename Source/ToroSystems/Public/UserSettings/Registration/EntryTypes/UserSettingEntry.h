// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Misc/Optional.h"
#include "Templates/Identity.h"
#include "UserSettings/Registration/UserSettingMetadata.h"

class UToroGameUserSettings;

#define LOCTEXT_NAMESPACE "ToroEngine"

enum class EUserSettingControl : uint8
{
	Toggle,
	Slider,
	Selection,
	Action
};

class IUserSettingSelection;

class FUserSettingEntry
{
public:

	FUserSettingEntry() = delete;
	FUserSettingEntry(const FUserSettingEntry&) = delete;
	FUserSettingEntry& operator=(const FUserSettingEntry&) = delete;

	explicit FUserSettingEntry(const FName& InKey)
		: Key(InKey)
	{}

	virtual ~FUserSettingEntry() = default;

	virtual EUserSettingControl GetControlType() const = 0;

	virtual const IUserSettingSelection* AsSelection() const
	{
		return nullptr;
	}

	virtual bool IsAvailable() const
	{
		return !EnabledPredicate || EnabledPredicate();
	}

	FName GetKey() const { return Key; }
	const FUserSettingMetadata& GetMetadata() const { return Metadata; }

protected:

	FName Key;
	FUserSettingMetadata Metadata;
	TFunction<bool()> EnabledPredicate;
};

template <typename Derived>
class TUserSettingEntry : public FUserSettingEntry
{
public:

	explicit TUserSettingEntry(const FName& InKey)
		: FUserSettingEntry(InKey)
	{}

	Derived& DisplayName(FText Text)
	{
		Metadata.DisplayName = MoveTemp(Text);
		return Self();
	}

	Derived& Description(FText Text)
	{
		Metadata.Description = MoveTemp(Text);
		return Self();
	}

	Derived& Impact(EUserSettingImpact Value)
	{
		Metadata.Impact = Value;
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
class TUserValueSetting : public TUserSettingEntry<Derived>
{
public:

	template<typename ObjectType>
	using FHandler = TFunction<void(ObjectType&, const FUserSettingEntry&)>;

	template <typename ObjectType>
	using FGetter = TFunction<T(const ObjectType&, const FUserSettingEntry&)>;

	template <typename ObjectType>
	using FSetter = TFunction<void(ObjectType&, const FUserSettingEntry&, const T&)>;

	using FComparator = TFunction<bool(const T&, const T&)>;

	explicit TUserValueSetting(FName Key)
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

	Derived& IsDefault(TFunction<bool()> Check)
	{
		DefaultCheck = MoveTemp(Check);
		return this->Self();
	}

	template<typename ObjectType>
	Derived& BindReset(ObjectType* Object, TIdentity_T<FHandler<ObjectType>> Handler)
	{
		ResetHandler = MakeHandler(Object, MoveTemp(Handler));
		return this->Self();
	}

	template<typename ObjectType>
	Derived& BindPostApply(ObjectType* Object, TIdentity_T<FHandler<ObjectType>> Handler)
	{
		PostApplyHandler = MakeHandler(Object, MoveTemp(Handler));
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

	void SetValue(const T& NewValue) const
	{
		if (IsAvailable() && WriteValue)
		{
			WriteValue(*this, NewValue);
			this->RunPostApply();
		}
	}

	bool CanReset() const
	{
		return IsAvailable() && (ResetHandler || (DefaultValue.IsSet() && WriteValue));
	}

	void ResetValue() const
	{
		if (CanReset())
		{
			if (this->ResetHandler)
			{
				this->ResetHandler(*this);
				this->RunPostApply();
			}
			else
			{
				SetValue(DefaultValue.GetValue());
			}
		}
	}

	bool DiffersFromDefault() const
	{
		if (!IsAvailable())
		{
			return false;
		}

		if (DefaultCheck)
		{
			return !DefaultCheck();
		}

		return DefaultValue.IsSet() && ReadValue
			&& !AreValuesEqual(ReadValue(*this), DefaultValue.GetValue());
	}

private:

	template<typename ObjectType>
	static TFunction<void(const FUserSettingEntry&)> MakeHandler(
		ObjectType* Object, TIdentity_T<FHandler<ObjectType>> Handler)
	{
		static_assert(TIsDerivedFrom<ObjectType, UToroGameUserSettings>::Value,
			"ObjectType must derive from UToroGameUserSettings");

		const TWeakObjectPtr<ObjectType> WeakObject(Object);
		return [WeakObject, Handler = MoveTemp(Handler)](const FUserSettingEntry& Entry)
		{
			ObjectType* Target = WeakObject.Get();
			if (Target && Handler)
			{
				Handler(*Target, Entry);
			}
		};
	}

	void RunPostApply() const
	{
		if (PostApplyHandler)
		{
			PostApplyHandler(*this);
		}
	}

	TOptional<T> DefaultValue;
	FComparator CompareValues;
	TFunction<bool()> DefaultCheck;
	TFunction<bool()> BindingIsValid;
	TFunction<T(const FUserSettingEntry&)> ReadValue;
	TFunction<void(const FUserSettingEntry&, const T&)> WriteValue;
	TFunction<void(const FUserSettingEntry&)> PostApplyHandler;
	TFunction<void(const FUserSettingEntry&)> ResetHandler;
};

#undef LOCTEXT_NAMESPACE
