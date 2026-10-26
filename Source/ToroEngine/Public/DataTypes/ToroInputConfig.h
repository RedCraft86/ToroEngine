// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroInputConfig.generated.h"

/** Selects whether input is handled by the game, UI, or both. */
UENUM(BlueprintType)
enum class EToroInputMode : uint8
{
	/** UI handles input first; unhandled input can reach the game. */
	GameAndUI,

	/** Routes input to the game and suppresses cursor visibility through ShouldShowMouse(). */
	GameOnly,

	/** Routes input only to UI; prefer GameAndUI when game input should remain available. */
	UI_Only // Prefer GameAndUI
};

/**
 * Stores player input routing, cursor, capture, and optional UI focus settings.
 * GetInputMode() prepares an internally cached native mode; it does not apply settings to a player controller.
 * Cursor visibility is queried separately through ShouldShowMouse().
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FToroInputConfig final
{
	GENERATED_BODY()

	/** Input routing mode used when preparing the native input mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	EToroInputMode InputMode;

	/** Requests cursor visibility in GameAndUI and UI_Only; ignored in GameOnly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config, meta = (EditCondition = "InputMode != EToroInputMode::GameOnly", EditConditionHides))
	bool bShowMouseCursor;

	/** Viewport mouse-lock behavior for GameAndUI and UI_Only. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config, meta = (EditCondition = "InputMode != EToroInputMode::GameOnly", EditConditionHides))
	EMouseLockMode MouseLock;

	/** Whether temporary mouse capture hides the cursor in GameAndUI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config, meta = (EditCondition = "InputMode == EToroInputMode::GameAndUI", EditConditionHides))
	bool bHideCursorOnCapture;

	/** Whether GameOnly consumes the mouse-down event that causes capture. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config, meta = (EditCondition = "InputMode == EToroInputMode::GameOnly", EditConditionHides))
	bool bConsumeCaptureMouseDown;

	/** Optional focus target for UI-capable modes; this weak reference does not keep the UUserWidget alive. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Config, meta = (DisplayThumbnail = false))
	TWeakObjectPtr<UUserWidget> FocusWidget;

private:

	static inline const FInputModeGameAndUI DefaultMode = {};

	FInputModeGameAndUI GameAndUI;
	FInputModeGameOnly GameOnly;
	FInputModeUIOnly UI_Only;

public:

	FToroInputConfig()
		: InputMode(EToroInputMode::GameOnly)
		, bShowMouseCursor(false)
		, MouseLock(EMouseLockMode::LockAlways)
		, bHideCursorOnCapture(true)
		, bConsumeCaptureMouseDown(true)
		, FocusWidget(nullptr)
	{}

	FToroInputConfig(
		const EToroInputMode InInputMode,
		const bool bInShowMouseCursor,
		const EMouseLockMode InMouseLock = EMouseLockMode::LockAlways,
		const bool bInHideCursorOnCapture = true,
		const bool bInConsumeCaptureMouseDown = true,
		UUserWidget* InFocusWidget = nullptr
	)
		: InputMode(InInputMode)
		, bShowMouseCursor(bInShowMouseCursor)
		, MouseLock(InMouseLock)
		, bHideCursorOnCapture(bInHideCursorOnCapture)
		, bConsumeCaptureMouseDown(bInConsumeCaptureMouseDown)
		, FocusWidget(InFocusWidget)
	{}

	/** Returns whether cursor visibility is requested and the selected mode permits it. */
	[[nodiscard]] FORCEINLINE bool ShouldShowMouse() const
	{
		return InputMode != EToroInputMode::GameOnly && bShowMouseCursor;
	}

	/**
	 * Updates and returns the cached native input mode for the current settings without applying it to a controller.
	 * UI-capable modes use the valid focus widget's Slate widget, or clear their cached target when it is invalid.
	 * @return A reference owned by this config, valid only while it exists and subject to later updates or Reset().
	 *         Unrecognized mode values return a clean GameAndUI mode.
	 */
	[[nodiscard]] const FInputModeDataBase& GetInputMode();

	/**
	 * Resets cached native modes and clears FocusWidget without applying changes to a player controller.
	 * Preserves InputMode and all cursor, locking, and capture settings.
	 */
	void Reset();
};
