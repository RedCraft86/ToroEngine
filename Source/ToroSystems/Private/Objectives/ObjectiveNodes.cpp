// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Objectives/ObjectiveNodes.h"
#include "Objectives/ObjectiveGraph.h"
#include "Objectives/ObjectiveSubsystem.h"
#include "AsyncGameplayMessageSystem.h"
#include "AsyncMessageWorldSubsystem.h"
#include "Utilities/ToroUtilities.h"
#if WITH_EDITOR
#include "SMNodeInstance.h"
#endif

UObjectiveNode::UObjectiveNode()
{
#if WITH_EDITORONLY_DATA
	bHideFromDropDownIfRulesFail = true;
	bHideFromContextMenuIfRulesFail = true;
	ConnectionRules.AllowedInStateMachines.Emplace_GetRef(UObjectiveRootNode::StaticClass()).bIncludeChildren = false;
	ConnectionRules.AllowedInboundStates.Emplace_GetRef(USMEntryStateInstance::StaticClass()).bIncludeChildren = false;
	ConnectionRules.AllowedInboundStates.Emplace_GetRef(StaticClass()).bIncludeChildren = true;
	ConnectionRules.AllowedOutboundStates.Emplace_GetRef(StaticClass()).bIncludeChildren = true;
#endif
}

void UObjectiveNode::OnStateBegin_Implementation()
{
	Super::OnStateBegin_Implementation();

	if (UGameFlagManager* FlagManager = UGameFlagManager::Get(GetContext()))
	{
		for (const TPair<FGameplayTag, FObjectiveFlagInfo>& Flag : Flags)
		{
			if (!Flag.Key.IsValid())
			{
				continue;
			}

			if (Flag.Value.bState)
			{
				FlagManager->SetGameFlag(Flag.Key, Flag.Value.Scope);
			}
			else
			{
				FlagManager->ClearGameFlag(Flag.Key, Flag.Value.Scope);
			}
		}
	}

	const TSharedPtr<FAsyncGameplayMessageSystem> System = UAsyncMessageWorldSubsystem
		::GetSharedMessageSystem<FAsyncGameplayMessageSystem>(ToroEngine::GetWorld(GetContext()));
	if (System.IsValid())
	{
		for (const TPair<FAsyncMessageId, FInstancedStruct>& Event : Events)
		{
			if (!Event.Key.IsValid())
			{
				continue;
			}

			System->QueueMessageForBroadcast(Event.Key, FConstStructView(Event.Value));
		}
	}

	if (UObjectiveSubsystem* Subsystem = Cast<UObjectiveSubsystem>(GetContext()))
	{
		Subsystem->ArriveAtState(this);
	}
}

#if WITH_EDITOR
void UObjectiveNode::OnPreCompileValidate_Implementation(USMCompilerLog* CompilerLog) const
{
	Super::OnPreCompileValidate_Implementation(CompilerLog);

	TArray<USMTransitionInstance*> Outgoing;
	GetOutgoingTransitions(Outgoing, false);
	if (Outgoing.Num() > 1)
	{
		CompilerLog->Log(ESMCompilerLogType::Error,
			TEXT("An objective step can have at most one outgoing transition. Remove the extra transitions.")
		);
	}
}
#endif

UObjectiveTransition::UObjectiveTransition()
{
#if WITH_EDITORONLY_DATA
	bAutoPlaceTransitionIfRulesPass = true;
	FSMNodeConnectionRule& Rule = ConnectionRules.AllowedConnections.AddDefaulted_GetRef();
	Rule.FromState = FSMStateClassRule(UObjectiveNode::StaticClass());
	Rule.ToState = FSMStateClassRule(UObjectiveNode::StaticClass());
	Rule.InStateMachine = FSMStateMachineClassRule(UObjectiveRootNode::StaticClass());
	Rule.InStateMachine.bIncludeChildren = false;
#endif
}

bool UObjectiveTransition::CanEnterTransition_Implementation() const
{
	const UObjectiveSubsystem* Subsystem = Cast<UObjectiveSubsystem>(GetContext());
	return Subsystem && Subsystem->CanAdvanceFrom(Cast<UObjectiveNode>(GetPreviousStateInstance()));
}
