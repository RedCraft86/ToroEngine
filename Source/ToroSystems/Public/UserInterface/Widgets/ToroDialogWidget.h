// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroMenuWidget.h"
#include "ToroDialogWidget.generated.h"

class UTextBlock;
class UPanelWidget;
class UCommonTextButton;
class UCommonButtonStyle;

DECLARE_DYNAMIC_DELEGATE_OneParam(FToroDialogReturnDelegate, int32, ButtonIndex);

DECLARE_DELEGATE_OneParam(FToroDialogReturnDelegateNative, int32);

/**
 * Modal choice dialog presented through the owning player's master menu stack.
 * Factories synchronously load the concrete class configured in FToroUserInterfaceConfig.
 * That class must supply TitleBlock, BodyBlock, and a multi-child ButtonContainer through BindWidget.
 * Choice clicks and automatic selection return the original choice index; Back returns INDEX_NONE.
 * Completion deactivates the dialog before notifying its caller. Covering it with another menu does
 * not complete it, and its countdown advances only while it is active and ticking.
 */
UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType)
class UToroDialogWidget final : public UToroMenuWidget
{
	GENERATED_BODY()

public:

	UToroDialogWidget();

	/**
	 * Creates and pushes a configured dialog with a Blueprint completion callback.
	 * Requires a valid player controller, an on-screen Toro master UI, and a concrete configured dialog class.
	 * Creation failures are logged and do not invoke OnClosed.
	 * @param TitleText Heading displayed above the dialog body.
	 * @param BodyText Message presented alongside the choices.
	 * @param Choices Button labels; empty or whitespace-only labels are skipped without renumbering other choices.
	 * @param bShouldPauseGame Whether to request game pause while active, requiring an owning AToroPlayerController.
	 * @param OnClosed Receives the selected choice index, or INDEX_NONE on Back, after deactivation.
	 * @param AutoSelectIdx Original choice index to select on timeout; INDEX_NONE or an unavailable button disables it.
	 * @param AutoSelectTime Active ticking seconds before automatic selection; values below 3 disable it.
	 *        The countdown displays remaining seconds rounded up to an integer.
	 * @param PlayerIdx Player controller index, with local players first, then available remote players.
	 */
	UFUNCTION(BlueprintCallable, Category = UserInterface, DisplayName = "Prompt User Dialog",
		meta = (WorldContext = ContextObject, AutoCreateRefTerm = Choices, AdvancedDisplay = 6))
	static TOROSYSTEMS_API void Create(
		const UObject* ContextObject,
		const FText TitleText,
		const FText BodyText,
		const TArray<FText>& Choices,
		bool bShouldPauseGame,
		FToroDialogReturnDelegate OnClosed,
		int32 AutoSelectIdx = -1,
		float AutoSelectTime = 5.0f,
		int32 PlayerIdx = 0
	);

	/**
	 * Creates and pushes a configured dialog, returning access to bind its native completion callback.
	 * Requires a valid player controller, an on-screen Toro master UI, and a concrete configured dialog class.
	 * Bind the returned delegate immediately; do not retain the reference beyond the dialog's lifetime.
	 * @param TitleText Heading displayed above the dialog body.
	 * @param BodyText Message presented alongside the choices.
	 * @param Choices Button labels; empty or whitespace-only labels are skipped without renumbering other choices.
	 * @param bShouldPauseGame Whether to request game pause while active, requiring an owning AToroPlayerController.
	 * @param AutoSelectIdx Original choice index to select on timeout; INDEX_NONE or an unavailable button disables it.
	 * @param AutoSelectTime Active ticking seconds before automatic selection; values below 3 disable it.
	 *        The countdown displays remaining seconds rounded up to an integer.
	 * @param PlayerIdx Player controller index, with local players first, then available remote players.
	 * @return Registration access for a callback receiving the original choice index or INDEX_NONE on Back,
	 *         after deactivation. On logged creation failure, returns a shared dummy delegate that never fires
	 *         and is unbound again on the next failed creation.
	 */
	[[nodiscard]] static TOROSYSTEMS_API FToroDialogReturnDelegateNative::RegistrationType& Create(
		const UObject* ContextObject,
		const FText& TitleText,
		const FText& BodyText,
		const TArray<FText>& Choices,
		bool bShouldPauseGame,
		int32 AutoSelectIdx = INDEX_NONE,
		float AutoSelectTime = 5.0f,
		int32 PlayerIdx = 0
	);

private:

	/** CommonUI button style applied to every generated choice button. */
	UPROPERTY(EditAnywhere, Category = Appearance)
	TSubclassOf<UCommonButtonStyle> ChoiceButtonStyle;

	/** Button padding applied to every generated choice button. */
	UPROPERTY(EditAnywhere, Category = Appearance)
	FMargin ChoiceButtonPadding;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleBlock;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> BodyBlock;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPanelWidget> ButtonContainer;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCommonTextButton>> Buttons;

	UPROPERTY(Transient)
	FToroDialogReturnDelegate BPDelegate;

	FToroDialogReturnDelegateNative NativeDelegate;

	float AutoSelectTimer = 0.0f;
	FText AutoSelectChoice = FText::GetEmpty();
	int32 AutoSelectIndex = INDEX_NONE;
	int32 SelectedIndex = INDEX_NONE;

	[[nodiscard]] static UToroDialogWidget* CreateInternal(
		const UObject* ContextObject,
		const FText& TitleText,
		const FText& BodyText,
		const TArray<FText>& Choices,
		bool bShouldPauseGame,
		int32 AutoSelectIdx,
		float AutoSelectTime,
		int32 PlayerIdx
	);

	void InitializeDialog(const FText& TitleText, const FText& BodyText, const TArray<FText>& Choices);
	void SetupAutoSelection(int32 Index, float Time, const TArray<FText>& Choices);
	void OnOptionClicked(int32 Source);

	virtual bool NativeOnHandleBackAction() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
