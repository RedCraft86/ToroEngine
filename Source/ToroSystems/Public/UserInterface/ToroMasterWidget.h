// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Blueprint/UserWidget.h"
#include "ToroMasterWidget.generated.h"

class UOverlay;
class UActivatableWidgetOverlay;
class UCommonActivatableWidgetStack;
class UCommonActivatableWidget;

/**
 * Native root UI with gameplay HUD, interactive menus, and important overlays, in that draw order.
 * Gameplay HUD fades out while the menu stack displays an entry and fades back in when it empties.
 * Screen fading independently multiplies gameplay and menu opacity through a shared wrapper.
 * The top overlay layer remains independent of both fades. Layers are created when rebuilt.
 */
UCLASS(NotBlueprintable, BlueprintType)
class TOROSYSTEMS_API UToroMasterWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/**
	 * Adds an exact-class gameplay HUD entry, or returns and reactivates an existing outgoing entry.
	 * Requires the layers to have been constructed; repeated adds do not change overlay order.
	 * @param InClass Concrete activatable widget class to display.
	 * @return The entry, or nullptr if the layer is unbuilt, the class is invalid, or creation fails.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = UserInterface, meta = (DeterminesOutputType = InClass))
	UCommonActivatableWidget* AddGameplayWidget(UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> InClass) const;

	/**
	 * Deactivates the exact-class gameplay HUD entry and removes it after its outgoing fade.
	 * Does nothing if the layer is unbuilt or no matching entry exists.
	 * @param InClass Class of the entry to remove.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = UserInterface)
	void RemoveGameplayWidget(UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> InClass) const;

	/**
	 * Pushes a menu unless its exact class is already in the stack.
	 * Returns an existing match without reordering or reactivating it, including an outgoing entry.
	 * Requires the layers to have been constructed.
	 * @param InClass Concrete activatable widget class to display.
	 * @return The entry, or nullptr if the layer is unbuilt, the class is invalid, or creation fails.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = UserInterface, meta = (DeterminesOutputType = InClass))
	UCommonActivatableWidget* AddMenuWidget(UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UToroMenuWidgetBase> InClass) const;

	/**
	 * Removes the exact-class menu entry using the stack's normal removal and transition behavior.
	 * Does nothing if the layer is unbuilt or no matching entry exists.
	 * @param InClass Class of the menu to remove.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = UserInterface)
	void RemoveMenuWidget(UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UToroMenuWidgetBase> InClass) const;

	/**
	 * Pushes an existing menu instance, allowing distinct entries of the same class.
	 * Does nothing if the layer is unbuilt, the widget is invalid, or this instance is already in the stack.
	 * Repeated adds do not reorder or reactivate the instance.
	 * @param Widget Instance created for the appropriate owning player and configured before adding.
	 *        Supplied instances are not pooled; the caller remains responsible for their lifetime.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = UserInterface)
	void AddMenuWidgetInstance(UToroMenuWidgetBase* Widget) const;

	/**
	 * Removes a specific menu instance using the stack's normal removal and transition behavior.
	 * Does nothing if the layer is unbuilt, the widget is invalid, or it is absent from this stack.
	 * @param Widget Instance to remove, leaving other entries of the same class untouched.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = UserInterface)
	void RemoveMenuWidgetInstance(UToroMenuWidgetBase* Widget) const;

	/**
	 * Adds an exact-class top overlay entry, or returns and reactivates an existing outgoing entry.
	 * Requires the layers to have been constructed; repeated adds do not change overlay order.
	 * @param InClass Concrete activatable widget class to display.
	 * @return The entry, or nullptr if the layer is unbuilt, the class is invalid, or creation fails.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = UserInterface, meta = (DeterminesOutputType = InClass))
	UCommonActivatableWidget* AddOverlayWidget(UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> InClass) const;

	/**
	 * Deactivates the exact-class top overlay entry and removes it after its outgoing fade.
	 * Does nothing if the layer is unbuilt or no matching entry exists.
	 * @param InClass Class of the entry to remove.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = UserInterface)
	void RemoveOverlayWidget(UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> InClass) const;

	/**
	 * Sets the gameplay HUD fade duration and transition duration of all three layers.
	 * May be called before the layers are built; construction applies the stored value.
	 * @param InDuration Duration in seconds. Nonfinite values and values below 0.1 are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = UserInterface)
	void SetFadeDuration(float InDuration);

	/** Gets the shared full-transition duration in seconds. */
	UFUNCTION(BlueprintPure, Category = UserInterface)
	[[nodiscard]] FORCEINLINE float GetFadeDuration() const
	{
		return FadeDuration;
	}

	/**
	 * Immediately adjusts the shared opacity of gameplay and menu layers without animating a transition.
	 * Combines with the menu-driven gameplay fade; the top overlay remains unaffected.
	 * Input and hit testing are unchanged, including when the layers are fully transparent.
	 * May be called before construction; the stored amount is applied when layers are built.
	 * @param InAmount Fade amount clamped to [0, 1]: zero adds no fading, one makes the wrapped layers transparent.
	 */
	UFUNCTION(BlueprintCallable, Category = UserInterface)
	void SetScreenFade(float InAmount);

	/** Returns the stored screen fade amount in [0, 1], including before construction. */
	UFUNCTION(BlueprintPure, Category = UserInterface)
	[[nodiscard]] FORCEINLINE float GetScreenFade() const
	{
		return ScreenFade;
	}

protected:

	/** Wraps gameplay and menu layers to apply screen fading without affecting the top overlay layer. */
	UPROPERTY(Transient)
	TObjectPtr<UOverlay> ScreenFadeLayer;

	/** Bottom, noninteractive gameplay HUD layer, faded and then collapsed while a menu is displayed. */
	UPROPERTY(Transient)
	TObjectPtr<UActivatableWidgetOverlay> GameplayLayer;

	/** Middle layer for interactive menus such as pause, inventory, and quick-time events. */
	UPROPERTY(Transient)
	TObjectPtr<UCommonActivatableWidgetStack> MenuLayer;

	/** Top, noninteractive layer for important overlays such as notices and framerate displays. */
	UPROPERTY(Transient)
	TObjectPtr<UActivatableWidgetOverlay> OverlayLayer;

	bool bShowGameplay = true;
	float FadeDuration = 0.5f;
	float ScreenFade = 0.0f;

	void ApplyFadeDuration() const;

	void BindMenuLayer();
	void UnbindMenuLayer() const;
	void OnMenuDisplayChanged(UCommonActivatableWidget* ActiveWidget);

	virtual void NativeDestruct() override;
	virtual void NativeConstruct() override;
	virtual void SynchronizeProperties() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
};
