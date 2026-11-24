// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Dialogues/DialogueTransitions.h"
#include "Dialogues/DialogueGraph.h"
#include "Dialogues/DialogueNodes.h"
#include "Dialogues/DialogueSubsystem.h"

bool UDialogueTransitionBase::IsAvailable() const
{
	if (!RequiredFlag.IsValid())
	{
		return true;
	}

	const UGameFlagManager* Flags = UGameFlagManager::Get(GetContext());
	return Flags && Flags->HasGameFlag(RequiredFlag, FlagScope);
}

bool UDialogueTransitionBase::CanEnterTransition_Implementation() const
{
	const UDialogueSubsystem* Dialogue = Cast<UDialogueSubsystem>(GetContext());
	return Dialogue && Dialogue->PendingTransition.Get() == this && IsAvailable();
}

void UDialogueTransitionBase::OnTransitionEntered_Implementation()
{
	if (UDialogueSubsystem* Dialogue = Cast<UDialogueSubsystem>(GetContext()))
	{
		if (Dialogue->PendingTransition.Get() == this)
		{
			Dialogue->PendingTransition.Reset();
		}
	}
}

UDialogueContinueTransition::UDialogueContinueTransition()
{
#if WITH_EDITORONLY_DATA
	bAutoPlaceTransitionIfRulesPass = true;
	FSMNodeConnectionRule& Rule = ConnectionRules.AllowedConnections.AddDefaulted_GetRef();
	Rule.FromState = FSMStateClassRule(UDialogueNodeBase::StaticClass());
	Rule.ToState = FSMStateClassRule(UDialogueChoiceNode::StaticClass());
	Rule.ToState.bIncludeChildren = false;
	Rule.ToState.bNOT = true;
	Rule.InStateMachine = FSMStateMachineClassRule(UDialogueRootNode::StaticClass());
	Rule.InStateMachine.bIncludeChildren = false;
#endif
}

UDialogueChoiceTransition::UDialogueChoiceTransition()
{
#if WITH_EDITORONLY_DATA
	bAutoPlaceTransitionIfRulesPass = true;
	FSMNodeConnectionRule& Rule = ConnectionRules.AllowedConnections.AddDefaulted_GetRef();
	Rule.FromState = FSMStateClassRule(UDialogueNodeBase::StaticClass());
	Rule.ToState = FSMStateClassRule(UDialogueChoiceNode::StaticClass());
	Rule.ToState.bIncludeChildren = false;
	Rule.InStateMachine = FSMStateMachineClassRule(UDialogueRootNode::StaticClass());
	Rule.InStateMachine.bIncludeChildren = false;
#endif
}
