// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Components/Overlay.h"
#include "Blueprint/UserWidgetPool.h"
#include "ActivatableWidgetOverlay.generated.h"

class UCommonActivatableWidget;

/**
 * Displays independently active Common UI widgets with one entry per exact class.
 * Entries fade in on activation and return to a pool after fading out on deactivation.
 * Re-adding an outgoing entry reverses its fade. Entries should call DeactivateWidget()
 * for animated self-removal; RemoveFromParent() detaches immediately.
 * The overlay controls entry render opacity during transitions.
 */
UCLASS(NotBlueprintable, BlueprintType, HideFunctions = (
	AddChild, AddChildToOverlay, RemoveChild, RemoveChildAt, ClearChildren, ReplaceOverlayChildAt))
class TOROSYSTEMS_API UActivatableWidgetOverlay : public UOverlay
{
	GENERATED_BODY()

public:

	UActivatableWidgetOverlay();

	/**
	 * Creates or reuses a pooled entry and activates it, or returns the existing exact-class entry.
	 * An existing outgoing entry is reactivated without changing its overlay order.
	 * Pooled entries may retain state from previous uses; initialize application-specific state as needed.
	 * @param InClass Concrete activatable widget class to display.
	 * @return The entry, or nullptr if the class is invalid or creation fails.
	 */
	UFUNCTION(BlueprintCallable, Category = Overlay, meta = (DeterminesOutputType = InClass))
	UCommonActivatableWidget* AddWidget(UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> InClass);

	/**
	 * Deactivates the exact-class entry, leaving it attached until its outgoing fade finishes.
	 * Does nothing if no entry exists, or it is already leaving.
	 * @param InClass Class of the entry to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = Overlay)
	void RemoveWidget(UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> InClass);

	/**
	 * Sets the duration of a full opacity transition; also affects transitions already in progress.
	 * @param InDuration Duration in seconds. Nonfinite values and values below 0.1 are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = UserInterface)
	void SetFadeDuration(float InDuration);

	/** Gets the configured duration in seconds for a full opacity transition. */
	UFUNCTION(BlueprintPure, Category = UserInterface)
	[[nodiscard]] FORCEINLINE float GetFadeDuration() const
	{
		return FadeDuration;
	}

private:

	struct FEntry
	{
		TWeakObjectPtr<UCommonActivatableWidget> Widget;
		ESlateVisibility ActiveVisibility = ESlateVisibility::SelfHitTestInvisible;
		bool bActive = true;
	};

	/** Full opacity transition duration in seconds, with a minimum of 0.1. */
	UPROPERTY(EditAnywhere, Category = UserInterface, meta = (ClampMin = 0.1f, UIMin = 0.1f))
	float FadeDuration = 0.4f;

	UPROPERTY(Transient)
	FUserWidgetPool WidgetPool;

	bool bPauseTicking = false;
	bool bUpdatingSlateResources = false;
	TWeakPtr<FActiveTimerHandle> TickHandle;
	TMap<TWeakObjectPtr<UCommonActivatableWidget>, bool> Animations;
	TMap<TSubclassOf<UCommonActivatableWidget>, FEntry> Widgets;

	void RegisterEntry(UCommonActivatableWidget* Widget);
	FEntry* FindEntry(const UCommonActivatableWidget* Widget);

	void HandleActivated(UCommonActivatableWidget* Widget);
	void HandleDeactivated(UCommonActivatableWidget* Widget);
	void UnbindDelegates(const UCommonActivatableWidget* Widget) const;

	void ClearTickTimer();
	void StartTickTimer(const TSharedPtr<SWidget>& InWidget);
	EActiveTimerReturnType Tick(double, float DeltaTime);

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void OnSlotRemoved(UPanelSlot* InSlot) override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void BeginDestroy() override;

	using UOverlay::AddChild;
	using UOverlay::AddChildToOverlay;
	using UOverlay::InsertChildAt;
	using UOverlay::ShiftChild;
	using UOverlay::RemoveChild;
	using UOverlay::RemoveChildAt;
	using UOverlay::ClearChildren;
	using UOverlay::ReplaceOverlayChildAt;
#if WITH_EDITOR
	using UOverlay::ReplaceChild;
	using UOverlay::ReplaceChildAt;
#endif
};
