// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroEngineEd.h"

class TOROENGINEED_API FToroToolbarButton : public TSharedFromThis<FToroToolbarButton>
{
	static inline TMap<FName, TSharedPtr<FToroToolbarButton>> Entries = {};

public:

	virtual ~FToroToolbarButton() = default;

	template<typename ButtonType>
	static FName Register()
	{
		static_assert(TIsDerivedFrom<ButtonType, FToroToolbarButton>::Value,
			"ButtonType must derive from FToroToolbarButton");

		TSharedPtr<FToroToolbarButton> Button = MakeShareable(new ButtonType());
		if (Button->GetFName().IsNone())
		{
			UE_LOG(LogToroEngineEd, Warning, TEXT("FToroToolbarButton: Attempting to register a button with an invalid name"));
			Button.Reset();
			return NAME_None;
		}

		if (Entries.Contains(Button->GetFName()))
		{
			UE_LOG(LogToroEngineEd, Warning,
				TEXT("FToroToolbarButton: Attempting to register multiple buttons with name %s"), *Button->GetName()
			);

			Button.Reset();
			return NAME_None;
		}

		Entries.Add(Button->GetFName(), Button);
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateSP(
			Button.ToSharedRef(), &FToroToolbarButton::Register
		));

		UE_LOG(LogToroEngineEd, Log, TEXT("FToroToolbarButton: Registered button with name %s"), *Button->GetName());
		return Button->GetFName();
	}

	static void UnregisterAll()
	{
		if (!Entries.IsEmpty())
		{
			for (TPair<FName, TSharedPtr<FToroToolbarButton>>& Entry : Entries)
			{
				if (Entry.Value.IsValid())
				{
					Entry.Value->Unregister();
					Entry.Value.Reset();
				}
			}
		}

		Entries.Empty();
		UE_LOG(LogToroEngineEd, Log, TEXT("FToroToolbarButton: Unregistered all buttons"));
	}

	FORCEINLINE FName GetFName() const
	{
		return Name;
	}

	FORCEINLINE FString GetName() const
	{
		return Name.ToString();
	}

	virtual FSlateIcon GetSlateIcon()
	{
		return FSlateIcon(FCoreStyle::Get().GetStyleSetName(), TEXT("Launcher.Instance_Unknown"));
	}

protected:

	FName Name;
	FText Label = FText::GetEmpty();
	FText Tooltip = FText::GetEmpty();
	FName MenuHook = NAME_None;
	FName SectionName = NAME_None;
	bool bConditional = false;
	bool bRegistered = false;

	FToroToolbarButton()
		: Name(NAME_None)
	{}

	FToroToolbarButton(const FName& InName)
		: Name(FString::Printf(TEXT("%s.%s"), TEXT(UE_MODULE_NAME), *InName.ToString()))
	{}

	virtual void Register();
	virtual void Unregister();

	virtual void BuildButtonEntry(FToolMenuSection& Section);
	virtual bool ShouldShowEntry(const FToolMenuContext& Context) const;

	virtual void Execute(const FToolMenuContext& Context) = 0;
};
