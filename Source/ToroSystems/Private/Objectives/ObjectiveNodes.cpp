// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Objectives/ObjectiveNodes.h"
#include "Objectives/ObjectiveGraph.h"
#include "Objectives/ObjectiveSubsystem.h"
#include "AsyncGameplayMessageSystem.h"
#include "AsyncMessageWorldSubsystem.h"
#include "Utilities/ToroUtilities.h"
#if WITH_EDITOR
#include "SMNodeInstance.h"
#include "UObject/UnrealType.h"
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
void UObjectiveNode::SyncTaskDisplayOrder()
{
	TSet<FGameplayTag> Remaining;
	for (const auto& Task : CountTasks)
	{
		Remaining.Add(Task.Key);
	}
	for (const auto& Task : FlagTasks)
	{
		Remaining.Add(Task.Key);
	}
	Remaining.Remove(FGameplayTag());

	TaskDisplayOrder.RemoveAll([&Remaining](const FGameplayTag& Tag)
	{
		return Remaining.Remove(Tag) == 0;
	});

	TArray<FGameplayTag> NewTags = Remaining.Array();
	NewTags.Sort([](const FGameplayTag& A, const FGameplayTag& B)
	{
		return A.GetTagName().LexicalLess(B.GetTagName());
	});
	TaskDisplayOrder.Append(NewTags);
}

void UObjectiveNode::PostLoad()
{
	Super::PostLoad();
	SyncTaskDisplayOrder();
}

void UObjectiveNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName Property = PropertyChangedEvent.GetMemberPropertyName();
	if (Property == GET_MEMBER_NAME_CHECKED(UObjectiveNode, CountTasks)
		|| Property == GET_MEMBER_NAME_CHECKED(UObjectiveNode, FlagTasks)
		|| Property == GET_MEMBER_NAME_CHECKED(UObjectiveNode, TaskDisplayOrder))
	{
		SyncTaskDisplayOrder();
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

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
