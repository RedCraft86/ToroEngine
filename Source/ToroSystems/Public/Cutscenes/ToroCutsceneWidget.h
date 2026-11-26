// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserInterface/Bases/ToroMenuWidget.h"
#include "ToroCutsceneWidget.generated.h"

class UTextBlock;
class UProgressBar;
class ACutsceneActor;
class UCutsceneSubsystem;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType)
class UToroCutsceneWidget final : public UToroMenuWidget
{
	GENERATED_BODY()

public:

	UToroCutsceneWidget();

private:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UProgressBar> SkipProgress;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> SkipKeyText;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> SubtitleText;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidget> SubtitleContainer;

	UPROPERTY(EditDefaultsOnly, Category = Cutscene)
	FKey SkipKey = EKeys::Enter;

	UPROPERTY(EditDefaultsOnly, Category = Cutscene, meta = (ClampMin = 0.0f, Units = "s"))
	float SkipHoldTime = 1.0f;

	bool bSkipHeld = false;
	float SkipHoldCounter = 0.0f;
	TWeakObjectPtr<UCutsceneSubsystem> Subsystem;

	void CompleteSkip();
	void ResetSkipHold();
	void BeginSkipHold();
	void OnUpdateSubtitle(const FText& NewText) const;

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
