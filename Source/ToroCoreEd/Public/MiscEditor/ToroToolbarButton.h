// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToolMenus.h"
#include "ToroCoreEd.h"
#include "Templates/SharedPointer.h"
#include "Styling/CoreStyle.h"

/**
 * Base for owned ToolMenus toolbar entries with deferred startup registration.
 * Derived buttons implement Execute and configure their menu location. Tracked shared ownership
 * keeps raw menu callbacks alive until UnregisterAll removes the entries.
 */
class TOROCOREED_API FToroToolbarButton : public TSharedFromThis<FToroToolbarButton>
{
	static inline TMap<FName, TSharedPtr<FToroToolbarButton>> ButtonEntries = {};

public:

	FToroToolbarButton()
		: Name(TEXT("ToroCoreEd.Unknown"))
		, bRegistered(false)
	{}

	virtual ~FToroToolbarButton() = default;

	/**
	 * Creates and retains a button, registering its menu entry when ToolMenus is ready.
	 * Duplicate tracked names are logged and ignored.
	 * @tparam ButtonType Default-constructible type derived from FToroToolbarButton.
	 */
	template <typename ButtonType>
	static void Register()
	{
		static_assert(TIsDerivedFrom<ButtonType, FToroToolbarButton>::Value,
			"ButtonType must derive from FToroToolbarButton");

		TSharedPtr<FToroToolbarButton> Button = MakeShared<ButtonType>();
		if (ButtonEntries.Contains(Button->GetFName()))
		{
			UE_LOG(LogToroCoreEd, Warning,
				TEXT("FToroToolbarButton: Attempting to register multiple Buttons with name %s"), *Button->GetName()
			);

			Button.Reset();
			return;
		}

		ButtonEntries.Add(Button->GetFName(), Button);
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateSP(
			Button.ToSharedRef(), &FToroToolbarButton::Register
		));

		UE_LOG(LogToroCoreEd, Display, TEXT("FToroToolbarButton: Registered Toolbar Button with name %s"), *Button->GetName());
	}

	/**
	 * Cancels startup callbacks, unregisters tracked entries, and releases this helper's ownership.
	 */
	static void UnregisterAll()
	{
		if (!ButtonEntries.IsEmpty())
		{
			for (TPair<FName, TSharedPtr<FToroToolbarButton>>& Button : ButtonEntries)
			{
				if (Button.Value.IsValid())
				{
					// Fix: Remove deferred callbacks before releasing the button, including derived cleanup paths.
					UToolMenus::UnRegisterStartupCallback(Button.Value.Get());
					Button.Value->Unregister();
					Button.Value.Reset();
				}
			}

			ButtonEntries.Empty();
			UE_LOG(LogToroCoreEd, Display, TEXT("FToroToolbarButton: Unregistered Toolbar Buttons"));
		}
	}

	/** Gets the module-prefixed toolbar entry name. */
	FORCEINLINE FName GetFName() const
	{
		return Name;
	}

	/** Gets the module-prefixed entry name as a string. */
	FORCEINLINE FString GetName() const
	{
		return Name.ToString();
	}

	/** Provides the toolbar icon, defaulting to the core style's unknown launcher icon. */
	virtual FSlateIcon GetSlateIcon()
	{
		return FSlateIcon(FCoreStyle::Get().GetStyleSetName(), TEXT("Launcher.Instance_Unknown"));
	}

protected:

	FName Name;
	FText Label;
	FText Tooltip;
	FName MenuHook;
	FName SectionName;
	bool bRegistered;

	/**
	 * Initializes a button in the Level Editor play toolbar's ToroUtilities section.
	 * @param InName Local name prefixed with the compiling module name.
	 * @param InLabel Visible button label.
	 * @param InTooltip Tooltip shown for the button.
	 */
	FToroToolbarButton(const FName& InName, const FText& InLabel, const FText& InTooltip)
		: Name(FString::Printf(TEXT("%s.%s"), TEXT(UE_MODULE_NAME), *InName.ToString()))
		, Label(InLabel)
		, Tooltip(InTooltip)
		, MenuHook(TEXT("LevelEditor.LevelEditorToolBar.PlayToolBar"))
		, SectionName(TEXT("ToroUtilities"))
		, bRegistered(false)
	{}

	/** Adds the owned menu entry once; derived implementations should preserve ownership and cleanup. */
	virtual void Register();

	/** Cancels deferred registration and removes owned menu entries; derived implementations should call the base. */
	virtual void Unregister();

	/** Runs the action when the toolbar button is activated. */
	virtual void Execute() = 0;
};
