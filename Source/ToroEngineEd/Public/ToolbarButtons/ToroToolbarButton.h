// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Utilities/ToroLogging.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Internationalization/Text.h"
#include "Templates/SharedPointer.h"
#include "Templates/UnrealTypeTraits.h"
#include "Textures/SlateIcon.h"
#include "UObject/NameTypes.h"
#include "ToolMenus.h"

/**
 * Shared-owned base for registering toolbar buttons with context-aware actions.
 * Derived constructors configure a unique name, menu hook, section, and presentation.
 * Configuration should remain unchanged while registered; use the static registration API
 * on the editor game thread and call UnregisterAll() during module shutdown.
 */
class TOROENGINEED_API FToroToolbarButton : public TSharedFromThis<FToroToolbarButton>
{
	/** Retains accepted buttons until their callbacks and menu entries are removed by UnregisterAll(). */
	static inline TMap<FName, TSharedPtr<FToroToolbarButton>> RegisteredEntries = {};

public:

	virtual ~FToroToolbarButton() = default;

	/**
	 * Creates and retains a configured button, registering its entries when ToolMenus is ready.
	 * The callback executes immediately if ToolMenus is already initialized; otherwise it is deferred.
	 * @tparam ButtonType Default-constructible concrete type derived from FToroToolbarButton.
	 * @return Accepted button name, or NAME_None for an invalid configuration or duplicate name.
	 *         An accepted name does not guarantee that an entry is already visible.
	 */
	template<typename ButtonType>
	static FName Register()
	{
		static_assert(TIsDerivedFrom<ButtonType, FToroToolbarButton>::Value,
			"ButtonType must derive from FToroToolbarButton");

		TSharedPtr<FToroToolbarButton> Button = MakeShareable(new ButtonType());
		if (Button->GetFName().IsNone())
		{
			TORO_LOG_WARNING(TEXT("Attempting to register a button with an invalid name"));
			Button.Reset();
			return NAME_None;
		}

		if (Button->MenuHook.IsNone() || Button->SectionName.IsNone())
		{
			TORO_LOG_WARNING(TEXT("Cannot register button %s with menu %s and section %s; both must be set"), *Button->GetName(), *Button->MenuHook.ToString(), *Button->SectionName.ToString());

			Button.Reset();
			return NAME_None;
		}

		if (RegisteredEntries.Contains(Button->GetFName()))
		{
			TORO_LOG_WARNING(TEXT("Attempting to register multiple buttons with name %s"), *Button->GetName());

			Button.Reset();
			return NAME_None;
		}

		RegisteredEntries.Add(Button->GetFName(), Button);
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateSP(
			Button.ToSharedRef(), &FToroToolbarButton::Register
		));

		TORO_LOG_INFO(TEXT("Registered button with name %s"), *Button->GetName());
		return Button->GetFName();
	}

	/**
	 * Cancels pending startup callbacks, removes owned menu entries, and releases all retained buttons.
	 * @note Automatically called by ToroEngineEd -> Module Shutdown.
	 */
	static void UnregisterAll()
	{
		if (!RegisteredEntries.IsEmpty())
		{
			for (TPair<FName, TSharedPtr<FToroToolbarButton>>& Entry : RegisteredEntries)
			{
				if (Entry.Value.IsValid())
				{
					Entry.Value->Unregister();
					Entry.Value.Reset();
				}
			}

			TORO_LOG_INFO(TEXT("Unregistered all buttons"));
		}

		RegisteredEntries.Empty();
	}

	/** Returns the registration key used for duplicate detection and menu-entry identity. */
	FORCEINLINE FName GetFName() const
	{
		return Name;
	}

	/** Returns the registration key as a string for logging or diagnostics. */
	FORCEINLINE FString GetName() const
	{
		return Name.ToString();
	}

	/** Supplies the toolbar icon; the base implementation uses the Core style's unknown-instance icon. */
	virtual FSlateIcon GetSlateIcon();

protected:

	/** Unique registration and entry key; must be set before static registration. */
	FName Name = NAME_None;

	/** User-visible button label. */
	FText Label = FText::GetEmpty();

	/** Hover text describing the action. */
	FText Tooltip = FText::GetEmpty();

	/** ToolMenus path of the toolbar to extend; must be set by the derived constructor. */
	FName MenuHook = NAME_None;

	/** Named section to find or create within MenuHook; must be set by the derived constructor. */
	FName SectionName = NAME_None;

	/** Defers entry construction to toolbar generation and enables the ShouldShowEntry() predicate. */
	bool bConditional = false;

	/** Whether the static entry or dynamic entry callback has been installed, regardless of visibility. */
	bool bRegistered = false;

	/**
	 * Creates a button key prefixed with the compiling module's name.
	 * @param InName Local button name; NAME_None remains invalid and is rejected during registration.
	 */
	FToroToolbarButton(const FName& InName)
		: Name(InName.IsNone() ? NAME_None : FName(FString::Printf(TEXT("%s.%s"), TEXT(UE_MODULE_NAME), *InName.ToString())))
	{}

	/**
	 * Installs a static entry or a conditional construction callback once ToolMenus is ready.
	 * Called through the static registration API, after shared ownership has been established.
	 */
	virtual void Register();

	/**
	 * Cancels this button's startup callback and removes its owned menu entries.
	 * Overrides should call the base implementation to preserve cleanup.
	 */
	virtual void Unregister();

	/**
	 * Adds the context-bound button entry when unconditional or accepted by ShouldShowEntry().
	 * @param Section Destination section; conditional calls receive the generated toolbar's context.
	 */
	virtual void BuildButtonEntry(FToolMenuSection& Section);

	/**
	 * Determines whether a conditional button is added when its toolbar is generated.
	 * The base implementation accepts every context; unconditional buttons bypass this predicate.
	 * Changes after generation require the toolbar to be regenerated.
	 * @param Context Context of the toolbar being generated, used to inspect its editor or assets.
	 * @return True to add the entry, or false to omit it from this generated toolbar.
	 */
	virtual bool ShouldShowEntry(const FToolMenuContext& Context) const;

	/**
	 * Executes the action using the context of the toolbar in which the button was clicked.
	 * @param Context Toolbar context; asset editor toolbars can provide UAssetEditorToolkitMenuContext.
	 */
	virtual void Execute(const FToolMenuContext& Context) = 0;
};
