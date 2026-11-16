// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserInterface/Bases/ToroAutoWidget.h"
#include "ToroNotifyWidget.generated.h"

class UCanvasPanel;
class UCommonTextStyle;
class UCommonTextBlock;

UENUM(BlueprintInternalUseOnly)
enum class EToroNotifyPhase : uint8
{
	Entering,
	Visible,
	Leaving
};

USTRUCT(BlueprintInternalUseOnly)
struct FToroNotifyEntry
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> Widget = nullptr;

	FText Text;
	float Height = 0.0f;
	float CurrentY = 0.0f;
	float StartY = 0.0f;
	float TargetY = 0.0f;
	float MoveElapsed = 0.0f;
	float PhaseElapsed = 0.0f;
	float HoldDuration = 0.0f;
	EToroNotifyPhase Phase = EToroNotifyPhase::Entering;

	FToroNotifyEntry()
		: Text(FText::GetEmpty())
	{}

	FToroNotifyEntry(const FText& InText)
		: Text(InText)
	{}
};

/**
 * Displays independently timed notifications in a stack growing down from the canvas's top center.
 * Entries enter from below, fade out in place, and slide upward as gaps close.
 * Blueprint subclasses must provide EntryCanvas and a TextStyle, with native ticking enabled.
 * Uses the overlay layer by default so notifications remain visible above menus.
 */
UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType)
class UToroNotifyWidget final : public UToroAutoWidget
{
	GENERATED_BODY()

public:

	UToroNotifyWidget();

	/**
	 * Adds a notification through the HUD's configured auto-widget.
	 * Text matching the newest remaining entry is accepted without adding another or refreshing its lifetime.
	 * @param ExtraTime Additional seconds included in the shared text reading-time calculation.
	 * @return True if accepted; false for blank text, an unavailable widget, or missing required configuration.
	 */
	UFUNCTION(BlueprintCallable, Category = Notification, meta = (WorldContext = ContextObject))
	static TOROSYSTEMS_API bool AddNotification(const UObject* ContextObject, FText Text, float ExtraTime = 0.0f);

	/**
	 * Removes all notifications immediately, without exit animations.
	 * @return False if the HUD or notification widget is unavailable; otherwise true, even if already empty.
	 */
	UFUNCTION(BlueprintCallable, Category = Notification, meta = (WorldContext = ContextObject))
	static TOROSYSTEMS_API bool ClearNotifications(const UObject* ContextObject);

private:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCanvasPanel> EntryCanvas;

	/** Shared CommonUI text style class applied to new notifications. */
	UPROPERTY(EditDefaultsOnly, Category = Notification)
	TSubclassOf<UCommonTextStyle> TextStyle;

	/** Text wrapping and entry width, in local UI units. */
	UPROPERTY(EditDefaultsOnly, Category = Notification, meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float EntryWidth = 750.0f;

	/** Vertical gap between entries, in local UI units. */
	UPROPERTY(EditDefaultsOnly, Category = Notification, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float EntrySpacing = 8.0f;

	/** Distance below the target position at which a new entry appears, in local UI units. */
	UPROPERTY(EditDefaultsOnly, Category = Notification, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float EntranceOffset = 24.0f;

	/** Seconds for each entrance and exit fade; zero makes opacity changes immediate. */
	UPROPERTY(EditDefaultsOnly, Category = Notification, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float FadeDuration = 0.2f;

	/** Seconds for entrance and gap-closing movement; zero makes position changes immediate. */
	UPROPERTY(EditDefaultsOnly, Category = Notification, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MoveDuration = 0.2f;

	/** Extra reading seconds per existing entry when a notification is added, allowing time to read a busy stack. */
	UPROPERTY(EditDefaultsOnly, Category = Notification, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float PerItemExtraTime = 1.0f;

	UPROPERTY(Transient)
	TArray<FToroNotifyEntry> Entries;

	void RecalculateTargets();
	void ClearNotifications();
	bool AddNotification(const FText& Text, float ExtraTime);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
