// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UE5Coro.h"
#include "DataTypes/MiscEnums.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroWidgetLibrary.generated.h"

class UWidget;
class UButton;
class UPanelWidget;
class UWidgetAnimation;

/**
 * Blueprint helpers for widget animation playback, root and parent lookup, and direct button delegate notifications.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroWidgetLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Prepares animation states declared by Blueprint-generated classes across the widget's class hierarchy.
	 * Does not start playback; invalid targets and animations without valid movie scenes are skipped.
	 * @param Target User widget whose animation states are initialized.
	 */
	UFUNCTION(BlueprintCallable, Category = Widget, meta = (DefaultToSelf = Target))
	static void InitWidgetAnimations(UUserWidget* Target);

	/**
	 * Requests forward or reverse widget-animation playback with a rate clamped to at least 0.1.
	 * @param Target User widget receiving the animation.
	 * @param Animation Animation with a valid movie scene.
	 * @param PlayMode Forward or Reverse; unsupported modes are rejected.
	 * @param PlayRate Finite playback rate; values below 0.1 are silently clamped, with direction controlled by PlayMode.
	 *        Nonfinite rates are rejected with an error message.
	 * @param bResumePlay True preserves the position of an already-playing animation and reverses it if needed,
	 *        applying the requested rate. Native helpers restart paused or stopped animations. False restarts at the native direction boundary.
	 * @return True if native playback returns a valid animation handle; false for invalid inputs or unavailable animation state.
	 */
	UFUNCTION(BlueprintCallable, Category = Widget, meta = (DefaultToSelf = Target))
	static bool PlayWidgetAnimation(UUserWidget* Target, UWidgetAnimation* Animation,
		EToroAnimationPlayMode PlayMode = EToroAnimationPlayMode::Forward, float PlayRate = 1.0f, bool bResumePlay = true);

	/**
	 * Requests widget-animation playback and waits for the prepared animation state's finished event.
	 * Binds before playback so immediate completion is observed; explicit stops can also trigger the event.
	 * The wait does not exclusively own playback or isolate it from other animation changes.
	 * @param bSuccess True after the event when the state's time is within 0.1 seconds of the requested endpoint:
	 * the animation's end time for Forward or start time for Reverse. Invalid inputs and other ending positions leave it false.
	 * @param Target User widget receiving the animation.
	 * @param Animation Animation with a valid movie scene.
	 * @param PlayMode Forward or Reverse; unsupported modes are rejected.
	 * @param PlayRate Finite playback rate, silently clamped to at least 0.1 and applied when continuing active playback.
	 *        Nonfinite rates are rejected with an error message.
	 * @param bResumePlay True continues or reverses already-playing animations; paused or stopped animations restart through native helpers.
	 *        False restarts at the native direction boundary.
	 */
	UFUNCTION(BlueprintCallable, Category = Widget, meta = (Latent, LatentInfo = LatentInfo, DefaultToSelf = Target))
	static FVoidCoroutine PlayWidgetAnimationAsync(FLatentActionInfo LatentInfo, bool& bSuccess, UUserWidget* Target, UWidgetAnimation* Animation,
		EToroAnimationPlayMode PlayMode = EToroAnimationPlayMode::Forward, float PlayRate = 1.0f, bool bResumePlay = true);

	/**
	 * Gets the root widget of a valid user widget.
	 * @param Target User widget whose root is requested.
	 * @return The root widget, or nullptr for an invalid target or absent root.
	 */
	UFUNCTION(BlueprintPure, Category = Widget, meta = (DefaultToSelf = Target))
	static UWidget* GetWidgetRoot(UUserWidget* Target);

	/**
	 * Gets the widget's valid panel parent, falling back through its owning widget tree when no valid direct parent exists.
	 * The fallback returns the owning user widget's panel parent; it does not recursively traverse arbitrary outer objects.
	 * @param Target Widget whose panel parent is requested.
	 * @return The direct or fallback panel parent, or nullptr for an invalid target or unavailable parent.
	 */
	UFUNCTION(BlueprintPure, Category = Widget, meta = (DefaultToSelf = Target))
	static UPanelWidget* GetWidgetParent(UWidget* Target);

	/**
	 * Directly broadcasts the button's OnClicked delegate, including when the button is disabled.
	 * Does not synthesize input or update Slate pressed or hovered state.
	 * @param Target Button to notify; invalid targets are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = Button, DisplayName = "Click Button", meta = (DefaultToSelf = Target))
	static void ButtonClick(UButton* Target);

	/**
	 * Directly broadcasts the button's OnPressed delegate, including when the button is disabled.
	 * Does not synthesize input or update Slate pressed or hovered state.
	 * @param Target Button to notify; invalid targets are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = Button, DisplayName = "Press Button", meta = (DefaultToSelf = Target))
	static void ButtonPress(UButton* Target);

	/**
	 * Directly broadcasts the button's OnReleased delegate, including when the button is disabled.
	 * Does not synthesize input or update Slate pressed or hovered state.
	 * @param Target Button to notify; invalid targets are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = Button, DisplayName = "Release Button", meta = (DefaultToSelf = Target))
	static void ButtonRelease(UButton* Target);

	/**
	 * Directly broadcasts the button's OnHovered delegate, including when the button is disabled.
	 * Does not synthesize input or update Slate pressed or hovered state.
	 * @param Target Button to notify; invalid targets are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = Button, DisplayName = "Hover Button", meta = (DefaultToSelf = Target))
	static void ButtonHover(UButton* Target);

	/**
	 * Directly broadcasts the button's OnUnhovered delegate, including when the button is disabled.
	 * Does not synthesize input or update Slate pressed or hovered state.
	 * @param Target Button to notify; invalid targets are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = Button, DisplayName = "Unhover Button", meta = (DefaultToSelf = Target))
	static void ButtonUnhover(UButton* Target);
};
