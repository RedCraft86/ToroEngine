// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Dialogues/DialogueSubsystem.h"
#include "Dialogues/DialogueGraph.h"
#include "Dialogues/DialogueNodes.h"
#include "Dialogues/DialogueTransitions.h"
#include "Dialogues/Widgets/ToroDialogueWidget.h"
#include "UserInterface/UserInterfaceConfig.h"
#include "Framework/ToroPlayerHUD.h"
#include "Utilities/ToroUtilities.h"
#include "Misc/MessageDialog.h"
#include "SMStateInstance.h"

namespace
{
	TSubclassOf<UToroDialogueWidget> GetDialogueWidgetClass()
	{
		const FUserInterfaceConfig* Config = FUserInterfaceConfig::Get();
		return Config ? Config->DialogueWidgetClass.LoadSynchronous() : nullptr;
	}
}

UDialogueSubsystem::UDialogueSubsystem()
{
}

EDialogueResultType UDialogueSubsystem::Start(const UObject* ContextObject,
	TSubclassOf<UDialogueGraph> Dialogue, int32 PlayerIdx)
{
	const UWorld* World = ToroEngine::GetWorld(ContextObject);
	if (!IsValid(World))
	{
		return EDialogueResultType::InvalidContext;
	}

	UDialogueSubsystem* Subsystem = World->GetSubsystem<UDialogueSubsystem>();
	if (!IsValid(Subsystem))
	{
		return EDialogueResultType::InvalidContext;
	}

	if (IsValid(Subsystem->StateMachine))
	{
		return EDialogueResultType::DialogueExists;
	}
	if (!IsValid(Dialogue) || Dialogue->HasAnyClassFlags(CLASS_Abstract))
	{
		return EDialogueResultType::InvalidDialogue;
	}

	const AToroPlayerHUD* HUD = AToroPlayerHUD::Get(ContextObject, PlayerIdx);
	const TSubclassOf<UToroDialogueWidget> WidgetClass = GetDialogueWidgetClass();
	if (!IsValid(HUD) || !IsValid(WidgetClass))
	{
		return EDialogueResultType::InvalidContext;
	}

	HUD->RemoveWidgetByClass(EToroMasterWidgetType::Menu, WidgetClass);
	UCommonActivatableWidget* Widget = HUD->AddWidgetByClass(EToroMasterWidgetType::Menu, WidgetClass);
	if (!Widget)
	{
		return EDialogueResultType::InvalidContext;
	}

	const EDialogueResultType Result = Subsystem->OpenDialogue(Dialogue);
	if (Result != EDialogueResultType::Success)
	{
		HUD->RemoveWidgetByInstance(EToroMasterWidgetType::Menu, Widget);
	}
	return Result;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UDialogueSubsystem::OnDialogueEnded(USMInstance* Instance)
{
	if (!Instance || Instance != StateMachine.Get())
	{
		return;
	}

	Instance->OnStateMachineStoppedEvent.RemoveAll(this);

	const UDialogueRootNode* RootNode = Cast<UDialogueRootNode>(Instance->GetRootStateMachineNodeInstance());
	if (RootNode && DialogueTags::IsValid(RootNode->DialogueTag))
	{
		if (!FlagManager.IsValid())
		{
			FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
				INVTEXT(
					"Seen Dialogue data was not saved properly, it is recommended that you "
					"restart the game before continuing to avoid any more data losses."
				), INVTEXT("Dialogue Save Error")
			);
		}
		else
		{
			FlagManager->SetGameFlag(RootNode->DialogueTag, RootNode->RememberScope);
		}
	}

	CurrentPrompt = {};
	OfferedTransitions.Reset();
	PendingTransition.Reset();
	StateMachine = nullptr;
	DialogueClosed.Broadcast();
}

void UDialogueSubsystem::PresentNode(const UDialogueNodeBase* Node)
{
	if (!IsValid(Node) || !IsValid(StateMachine) || !StateMachine->IsActive()
		|| Node->GetStateMachineInstance(true) != StateMachine)
	{
		return;
	}

	CurrentPrompt = {};
	CurrentPrompt.Id = FGuid::NewGuid();
	CurrentPrompt.Speaker = Node->GetSpeakerName();
	CurrentPrompt.Line = Node->Line.Result;
	OfferedTransitions.Reset();

	TArray<USMTransitionInstance*> Outgoing;
	Node->GetOutgoingTransitions(Outgoing, false);

	for (USMTransitionInstance* Edge : Outgoing)
	{
		UDialogueTransitionBase* Transition = Cast<UDialogueTransitionBase>(Edge);
		if (!Transition || !Transition->IsAvailable())
		{
			continue;
		}

		const FGuid& Id = Transition->GetGuid();
		if (!Id.IsValid())
		{
			continue;
		}

		if (CurrentPrompt.Choices.IsEmpty() && Transition->IsA<UDialogueContinueTransition>())
		{
			CurrentPrompt.ContinueId = Id;
			OfferedTransitions.Add(Id, Transition);
			break;
		}

		if (Transition->IsA<UDialogueChoiceTransition>())
		{
			if (const UDialogueChoiceNode* Choice = Cast<UDialogueChoiceNode>(Transition->GetNextStateInstance()))
			{
				const FText& Label = Choice->ShortText.IsEmpty() ? Choice->Line.Result : Choice->ShortText;
				CurrentPrompt.Choices.Add({Id, Label});
				OfferedTransitions.Add(Id, Transition);
			}
		}
	}

	DialoguePromptChanged.Broadcast(CurrentPrompt);
}

bool UDialogueSubsystem::SubmitInput(const FGuid& PromptId, const FGuid& TransitionId)
{
	if (bSubmittingInput || !PromptId.IsValid() || PromptId != CurrentPrompt.Id
		|| !IsValid(StateMachine) || !StateMachine->IsActive())
	{
		return false;
	}

	TGuardValue<bool> Submitting(bSubmittingInput, true);
	if (!TransitionId.IsValid())
	{
		if (!OfferedTransitions.IsEmpty())
		{
			return false;
		}
		StateMachine->Stop();
		return true;
	}

	const TWeakObjectPtr<UDialogueTransitionBase>* Offered = OfferedTransitions.Find(TransitionId);
	UDialogueTransitionBase* Transition = Offered ? Offered->Get() : nullptr;
	if (!Transition)
	{
		return false;
	}

	const USMStateInstance_Base* Source = Transition->GetPreviousStateInstance();
	if (!Source || !Source->IsActive())
	{
		return false;
	}

	PendingTransition = Transition;
	const bool bTaken = Transition->EvaluateFromManuallyBoundEvent();
	PendingTransition.Reset();
	if (!bTaken && !Transition->IsAvailable())
	{
		PresentNode(Cast<UDialogueNodeBase>(Source));
	}
	return bTaken;
}

EDialogueResultType UDialogueSubsystem::OpenDialogue(TSubclassOf<UDialogueGraph> Dialogue)
{
	if (IsValid(StateMachine))
	{
		return EDialogueResultType::DialogueExists;
	}

	StateMachine = NewObject<UDialogueGraph>(this, Dialogue);
	if (!StateMachine)
	{
		return EDialogueResultType::StateMachineError;
	}

	StateMachine->SetStopOnEndState(false);
	StateMachine->OnStateMachineStoppedEvent.AddDynamic(this, &UDialogueSubsystem::OnDialogueEnded);
	StateMachine->Initialize(this);
	StateMachine->Start();
	return EDialogueResultType::Success;
}

void UDialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FlagManager = UGameFlagManager::Get(this);
}

bool UDialogueSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
