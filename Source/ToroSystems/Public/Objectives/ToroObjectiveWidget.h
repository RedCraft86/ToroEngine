// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Styling/SlateBrush.h"
#include "GameplayTagContainer.h"
#include "UserInterface/Bases/ToroAutoWidget.h"
#include "ToroObjectiveWidget.generated.h"

class UTextBlock;
class UPanelWidget;
class UWidgetAnimation;
class UCommonTextStyle;
class UObjectiveNode;
class UObjectiveSubsystem;
class UGameFlagManager;
enum class EGameFlagScope : uint8;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType)
class UToroObjectiveWidget final : public UToroAutoWidget
{
	GENERATED_BODY()

public:

	UToroObjectiveWidget();

protected:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidget> RootContainer;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPanelWidget> TaskContainer;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> ObjectiveAnimation;

	UPROPERTY(EditAnywhere, Category = Objective)
	TSubclassOf<UCommonTextStyle> TaskTextStyle;

	UPROPERTY(EditAnywhere, Category = Objective)
	FSlateBrush TaskStrikeBrush;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:

	TWeakObjectPtr<UGameFlagManager> FlagManager;
	TWeakObjectPtr<UObjectiveSubsystem> Objectives;
	TWeakObjectPtr<const UObjectiveNode> DisplayedStep;

	void HandleObjectiveUpdated(const UObjectiveNode* Step);
	void HandleTaskUpdated(const FGameplayTag& Task, uint8 Progress) const;
	void HandleFlagChanged(FGameplayTag Flag, bool bRemoved, EGameFlagScope Scope) const;
	void RefreshTasks() const;
	void AddTaskRow(const FText& Label, bool bCompleted) const;
};
