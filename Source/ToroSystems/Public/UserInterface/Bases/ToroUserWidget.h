// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CommonActivatableWidget.h"
#include "ToroUserWidget.generated.h"

class SSpacer;
class SCommonAnimatedSwitcher;
enum class ECommonSwitcherTransition : uint8;
enum class ETransitionCurve : uint8;

/**
 * Activatable widget that animates its Designer content when activation changes.
 * CommonUI activation notifications remain immediate; presentation transitions block pointer
 * input and temporarily keep the widget drawable until its intended visibility can be restored.
 * Disable this widget's transitions when its container supplies the presentation animation.
 * Passive presentation does not participate in activation focus by default; interactive subclasses
 * can enable Supports Activation Focus and provide a CommonUI desired input configuration.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, PrioritizeCategories = (Activation))
class TOROSYSTEMS_API UToroUserWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnTransitionFinished, UToroUserWidget*)

	UToroUserWidget();

	/**
	 * Controls this widget's activation transitions without changing its activation state.
	 * @param bEnable False to finish current playback instantly; true to animate subsequent activation changes.
	 */
	UFUNCTION(BlueprintCallable, Category = ActivatableWidget)
	void SetEnableTransitions(bool bEnable);

	/** Whether activation changes animate instead of switching instantly. */
	UFUNCTION(BlueprintPure, Category = ActivatableWidget)
	[[nodiscard]] bool GetEnableTransitions() const { return bEnableTransitions; }

	/**
	 * Sets the visual effect, applying it after any current transition finishes.
	 * @param Transition Fade, translation, or zoom effect used by the switcher.
	 */
	UFUNCTION(BlueprintCallable, Category = ActivatableWidget)
	void SetTransitionType(ECommonSwitcherTransition Transition);

	/** Configured visual effect, including any pending change. */
	UFUNCTION(BlueprintPure, Category = ActivatableWidget)
	[[nodiscard]] FORCEINLINE ECommonSwitcherTransition GetTransitionType() const
	{
		return TransitionType;
	}

	/**
	 * Sets the easing curve, applying it after any current transition finishes.
	 * @param CurveType Easing used by the switcher's transition sequence.
	 */
	UFUNCTION(BlueprintCallable, Category = ActivatableWidget)
	void SetTransitionCurveType(ETransitionCurve CurveType);

	/** Configured easing curve, including any pending change. */
	UFUNCTION(BlueprintPure, Category = ActivatableWidget)
	[[nodiscard]] FORCEINLINE ETransitionCurve GetTransitionCurveType() const
	{
		return TransitionCurveType;
	}

	/**
	 * Sets the transition duration, applying it after any current transition finishes.
	 * @param Duration Switcher duration in seconds; negative values clamp to zero for instant changes.
	 */
	UFUNCTION(BlueprintCallable, Category = ActivatableWidget)
	void SetTransitionDuration(float Duration);

	/** Configured switcher duration in seconds, including any pending change. */
	UFUNCTION(BlueprintPure, Category = ActivatableWidget)
	[[nodiscard]] FORCEINLINE float GetTransitionDuration() const
	{
		return TransitionDuration;
	}

	/**
	 * Sets all transition settings together, applying them after any current transition finishes.
	 * @param Duration Switcher duration in seconds; negative values clamp to zero for instant changes.
	 * @param CurveType Easing used by the switcher's transition sequence.
	 * @param Transition Fade, translation, or zoom effect used by the switcher.
	 */
	UFUNCTION(BlueprintCallable, Category = ActivatableWidget)
	void SetTransitionParams(float Duration, ETransitionCurve CurveType, ECommonSwitcherTransition Transition);

	/** Whether the Slate switcher has a playing transition sequence; false when Slate is unavailable. */
	UFUNCTION(BlueprintPure, Category = ActivatableWidget)
	[[nodiscard]] bool IsTransitioning() const;

	/**
	 * Provides C++ subscription access to completed presentation transitions, including instant changes.
	 * Delivery is deferred on the game thread until after visibility and input cleanup, allowing
	 * listeners to start another transition safely. Callers must not supersede activation before delivery;
	 * queued callbacks are not explicitly canceled. Delivery requires valid, idle Slate outside teardown.
	 * Idle synchronization does not produce a new notification.
	 */
	[[nodiscard]] FORCEINLINE FOnTransitionFinished::RegistrationType& OnTransitionFinished()
	{
		return OnTransitionFinishedEvent;
	}

	virtual void RemoveFromParent() override;

protected:

	/** Enables this widget's own transitions; disable when an owning container supplies them. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Activation|Transition")
	bool bEnableTransitions;

	/** Visual effect used when showing or hiding the Designer content. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Activation|Transition", meta = (EditCondition = "bEnableTransitions"))
	ECommonSwitcherTransition TransitionType;

	/** Easing applied to the switcher's animation sequence. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Activation|Transition", meta = (EditCondition = "bEnableTransitions"))
	ETransitionCurve TransitionCurveType;

	/** Switcher duration in seconds; zero switches instantly and negative values are clamped when applied. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Activation|Transition", meta = (EditCondition = "bEnableTransitions"))
	float TransitionDuration;

	TSharedPtr<SSpacer> InputGuard;
	TSharedPtr<SCommonAnimatedSwitcher> AnimSwitcher;
	FOnTransitionFinished OnTransitionFinishedEvent;
	TOptional<ESlateVisibility> RestoreVisibility;
	bool bTransitionCompletionPending;

	void UpdateTransitionParams();
	void UpdateTransitionState();
	void RestorePendingVisibility();
	void OnTransitionStateChanged(bool bTransitioning);

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void SynchronizeProperties() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void OnWidgetRebuilt() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
};
