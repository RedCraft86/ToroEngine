// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Objectives/ToroObjectiveWidget.h"
#include "Objectives/ObjectiveSubsystem.h"
#include "Objectives/ObjectiveNodes.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "CommonTextBlock.h"
#include "Engine/World.h"
#include "GameFlagManager.h"
#include "Styling/CoreStyle.h"

UToroObjectiveWidget::UToroObjectiveWidget()
{
	AutoLayer = EToroAutoWidgetType::Gameplay;
	TaskStrikeBrush = *FCoreStyle::Get().GetBrush(TEXT("DefaultTextUnderline"));
}

void UToroObjectiveWidget::NativeConstruct()
{
	Super::NativeConstruct();

	FlagManager = UGameFlagManager::Get(this);
	if (UGameFlagManager* Flags = FlagManager.Get())
	{
		Flags->OnFlagStateChanged().AddUObject(this, &UToroObjectiveWidget::HandleFlagChanged);
	}

	if (const UWorld* World = GetWorld())
	{
		Objectives = World->GetSubsystem<UObjectiveSubsystem>();
	}

	if (UObjectiveSubsystem* Subsystem = Objectives.Get())
	{
		Subsystem->OnObjectiveUpdated().AddUObject(this, &UToroObjectiveWidget::HandleObjectiveUpdated);
		Subsystem->OnTaskUpdated().AddUObject(this, &UToroObjectiveWidget::HandleTaskUpdated);
		HandleObjectiveUpdated(Subsystem->GetCurrentStep());
	}
	else
	{
		HandleObjectiveUpdated(nullptr);
	}
}

void UToroObjectiveWidget::NativeDestruct()
{
	if (UObjectiveSubsystem* Subsystem = Objectives.Get())
	{
		Subsystem->OnObjectiveUpdated().RemoveAll(this);
		Subsystem->OnTaskUpdated().RemoveAll(this);
	}
	if (UGameFlagManager* Flags = FlagManager.Get())
	{
		Flags->OnFlagStateChanged().RemoveAll(this);
	}

	Objectives.Reset();
	FlagManager.Reset();
	DisplayedStep.Reset();
	Super::NativeDestruct();
}

void UToroObjectiveWidget::HandleObjectiveUpdated(const UObjectiveNode* Step)
{
	DisplayedStep = Step;
	TitleText->SetText(Step ? Step->DisplayTitle : FText::GetEmpty());
	const bool bHasTasks = Step && (!Step->CountTasks.IsEmpty() || !Step->FlagTasks.IsEmpty());
	SetVisibility(bHasTasks ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	RefreshTasks();

	if (bHasTasks)
	{
		RootContainer->SetVisibility(ESlateVisibility::HitTestInvisible);
		PlayAnimation(ObjectiveAnimation);
	}
	else
	{
		RootContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UToroObjectiveWidget::HandleTaskUpdated(const FGameplayTag& Task, uint8) const
{
	if (const UObjectiveNode* Step = DisplayedStep.Get(); Step && Step->CountTasks.Contains(Task))
	{
		RefreshTasks();
	}
}

void UToroObjectiveWidget::HandleFlagChanged(FGameplayTag Flag, bool, EGameFlagScope Scope) const
{
	const UObjectiveNode* Step = DisplayedStep.Get();
	const FObjectiveFlagTaskInfo* Task = Step ? Step->FlagTasks.Find(Flag) : nullptr;
	if (Task && Task->Scope == Scope)
	{
		RefreshTasks();
	}
}

void UToroObjectiveWidget::RefreshTasks() const
{
	TaskContainer->ClearChildren();
	const UObjectiveNode* Step = DisplayedStep.Get();
	const UObjectiveSubsystem* Subsystem = Objectives.Get();
	if (!Step || !Subsystem)
	{
		return;
	}

	const bool bShowCompletion = Step->CountTasks.Num() + Step->FlagTasks.Num() > 1;
	for (const auto& [Tag, Task] : Step->CountTasks)
	{
		const uint8 Progress = FMath::Min(Subsystem->GetTaskProgress(Tag), Task.Amount);
		const FText Label = Task.Amount > 1
			? FText::Format(INVTEXT("{0} ({1}/{2})"), Task.Label,
				static_cast<int32>(Progress), static_cast<int32>(Task.Amount))
			: Task.Label;
		AddTaskRow(Label, bShowCompletion && Progress >= Task.Amount);
	}

	const UGameFlagManager* Flags = FlagManager.Get();
	for (const auto& [Tag, Task] : Step->FlagTasks)
	{
		AddTaskRow(Task.Label, bShowCompletion && Flags && Flags->HasGameFlag(Tag, Task.Scope));
	}
}

void UToroObjectiveWidget::AddTaskRow(const FText& Label, bool bCompleted) const
{
	UCommonTextBlock* Row = WidgetTree->ConstructWidget<UCommonTextBlock>();
	Row->SetStyle(TaskTextStyle);
	Row->SetIsEnabled(!bCompleted);
	Row->SetText(Label);
	TaskContainer->AddChild(Row);

	if (bCompleted)
	{
		Row->SetStrikeBrush(TaskStrikeBrush);
	}
}
