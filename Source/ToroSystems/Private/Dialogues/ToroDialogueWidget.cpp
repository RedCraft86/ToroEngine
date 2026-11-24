// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Dialogues/ToroDialogueWidget.h"
#include "Dialogues/DialogueSubsystem.h"
#include "CommonButtonBase.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Widgets/ExpressiveTextBlock.h"
#include "UserInterface/Elements/CommonTextButton.h"

UToroDialogueWidget::UToroDialogueWidget()
{
}

void UToroDialogueWidget::OnNextLine() const
{
	if (Subsystem.IsValid() && PromptId.IsValid() && NextLineButton->IsInteractionEnabled())
	{
		const FGuid SubmittedPrompt = PromptId;
		const FGuid SubmittedEdge = ContinueId;
		Subsystem->SubmitInput(SubmittedPrompt, SubmittedEdge);
	}
}

void UToroDialogueWidget::OnChoice(FGuid ChoicePromptId, FGuid TransitionId) const
{
	if (Subsystem.IsValid() && ChoicePromptId == PromptId)
	{
		Subsystem->SubmitInput(ChoicePromptId, TransitionId);
	}
}

void UToroDialogueWidget::OnDialogueChanged(const FDialoguePrompt& Prompt)
{
	PromptId = Prompt.Id;
	ContinueId = Prompt.ContinueId;
	NextLineButton->SetIsInteractionEnabled(false);
	NextLineButton->SetVisibility(ESlateVisibility::Hidden);
	ChoiceContainer->ClearChildren();

	FTimerManager& Timers = GetWorld()->GetTimerManager();
	Timers.ClearTimer(NextLineTimer);
	SpeakerName->SetText(Prompt.Speaker);
	LineText->SetText(Prompt.Line);

	for (const FDialoguePromptChoice& Choice : Prompt.Choices)
	{
		AddChoiceButton(Choice.Label, Prompt.Id, Choice.TransitionId);
	}

	if (Prompt.Choices.IsEmpty())
	{
		if (NextLineDelay > 0.0f)
		{
			Timers.SetTimer(NextLineTimer, this, &UToroDialogueWidget::UnlockNextLine, NextLineDelay, false);
		}
		else
		{
			UnlockNextLine();
		}
	}

	PlayAnimation(DialogueAnimation);
}

void UToroDialogueWidget::AddChoiceButton(const FText& Label, const FGuid& ChoicePromptId, const FGuid& TransitionId)
{
	UCommonTextButton* Button = WidgetTree->ConstructWidget<UCommonTextButton>();
	Button->SetStyle(ChoiceButtonStyle);
	Button->SetPadding(ChoiceButtonPadding);
	Button->SetButtonText(Label);
	Button->OnClicked().AddUObject(this, &UToroDialogueWidget::OnChoice, ChoicePromptId, TransitionId);
	ChoiceContainer->AddChild(Button);
}

void UToroDialogueWidget::UnlockNextLine() const
{
	NextLineButton->SetIsInteractionEnabled(true);
	NextLineButton->SetVisibility(ESlateVisibility::Visible);
}

void UToroDialogueWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	Subsystem = GetWorld()->GetSubsystem<UDialogueSubsystem>();
	if (UDialogueSubsystem* Dialogue = Subsystem.Get())
	{
		Dialogue->OnDialoguePromptChanged().AddUObject(this, &UToroDialogueWidget::OnDialogueChanged);
		Dialogue->OnDialogueClosed().AddUObject(this, &UToroDialogueWidget::DeactivateWidget);
		const FDialoguePrompt& Prompt = Dialogue->GetCurrentPrompt();
		if (Prompt.Id.IsValid())
		{
			OnDialogueChanged(Prompt);
		}
	}
}

void UToroDialogueWidget::NativeOnDeactivated()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NextLineTimer);
	}

	if (UDialogueSubsystem* Dialogue = Subsystem.Get())
	{
		Dialogue->OnDialoguePromptChanged().RemoveAll(this);
		Dialogue->OnDialogueClosed().RemoveAll(this);
	}

	Subsystem.Reset();
	PromptId.Invalidate();
	ContinueId.Invalidate();
	NextLineButton->SetIsInteractionEnabled(false);
	NextLineButton->SetVisibility(ESlateVisibility::Hidden);
	ChoiceContainer->ClearChildren();
	Super::NativeOnDeactivated();
}

bool UToroDialogueWidget::NativeOnHandleBackAction()
{
	return true;
}

void UToroDialogueWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	NextLineButton->OnClicked().AddUObject(this, &UToroDialogueWidget::OnNextLine);
	NextLineButton->SetVisibility(ESlateVisibility::Hidden);
	NextLineButton->SetIsInteractionEnabled(false);
}
