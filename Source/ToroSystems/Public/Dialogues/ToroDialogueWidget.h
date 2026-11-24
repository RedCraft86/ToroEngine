// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "TimerManager.h"
#include "UserInterface/Bases/ToroMenuWidget.h"
#include "ToroDialogueWidget.generated.h"

class UTextBlock;
class UCommonButtonStyle;
class UExpressiveTextBlock;
class UCommonButtonBase;
class UDialogueSubsystem;
class UPanelWidget;
struct FDialoguePrompt;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType)
class UToroDialogueWidget final : public UToroMenuWidget
{
	GENERATED_BODY()

public:

	UToroDialogueWidget();

protected:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeakerName;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UExpressiveTextBlock> LineText;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> NextLineButton;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPanelWidget> ChoiceContainer;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> DialogueAnimation;

	UPROPERTY(EditDefaultsOnly, Category = Dialogue, meta = (ClampMin = 0.0f))
	float NextLineDelay = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = Dialogue)
	FMargin ChoiceButtonPadding;

	UPROPERTY(EditDefaultsOnly, Category = Dialogue)
	TSubclassOf<UCommonButtonStyle> ChoiceButtonStyle;

	FTimerHandle NextLineTimer;
	TWeakObjectPtr<UDialogueSubsystem> Subsystem;
	FGuid PromptId;
	FGuid ContinueId;

	void OnNextLine() const;
	void OnChoice(FGuid ChoicePromptId, FGuid TransitionId) const;
	void OnDialogueChanged(const FDialoguePrompt& Prompt);
	void AddChoiceButton(const FText& Label, const FGuid& ChoicePromptId, const FGuid& TransitionId);
	void UnlockNextLine() const;

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual bool NativeOnHandleBackAction() override;
	virtual void NativeOnInitialized() override;
};
