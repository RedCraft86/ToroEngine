// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Dialogues/DialogueNodes.h"
#include "Dialogues/DialogueGraph.h"
#include "Dialogues/DialogueSubsystem.h"
#include "Characters/ToroCharacterDatabase.h"
#include "Framework/ToroDatabaseSubsystem.h"
#include "AsyncGameplayMessageSystem.h"
#include "AsyncMessageWorldSubsystem.h"
#include "Utilities/ToroUtilities.h"
#include "GameFlagManager.h"
#if WITH_EDITOR
#include "SMNodeInstance.h"
#include "SMTransitionInstance.h"
#endif

UDialogueNodeBase::UDialogueNodeBase()
{
#if WITH_EDITORONLY_DATA
	bHideFromDropDownIfRulesFail = true;
	bHideFromContextMenuIfRulesFail = true;
	ConnectionRules.AllowedInStateMachines.Emplace_GetRef(UDialogueRootNode::StaticClass()).bIncludeChildren = false;
	ConnectionRules.AllowedInboundStates.Emplace_GetRef(USMEntryStateInstance::StaticClass()).bIncludeChildren = false;
	ConnectionRules.AllowedInboundStates.Emplace_GetRef(StaticClass()).bIncludeChildren = true;
	ConnectionRules.AllowedOutboundStates.Emplace_GetRef(StaticClass()).bIncludeChildren = true;
#endif
}

FText UDialogueNodeBase::GetSpeakerName() const
{
	return INVTEXT("<Missing Name>");
}

void UDialogueNodeBase::OnStateBegin_Implementation()
{
	Super::OnStateBegin_Implementation();
	if (UDialogueSubsystem* Dialogue = Cast<UDialogueSubsystem>(GetContext()))
	{
		Dialogue->PresentNode(this);
	}

	if (UGameFlagManager* FlagManager = UGameFlagManager::Get(GetContext()))
	{
		for (const TPair<FGameplayTag, FDialogueFlagInfo>& Flag : Flags)
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
}

#if WITH_EDITOR
void UDialogueNodeBase::OnPreCompileValidate_Implementation(USMCompilerLog* CompilerLog) const
{
	Super::OnPreCompileValidate_Implementation(CompilerLog);

	TArray<USMTransitionInstance*> Outgoing;
	GetOutgoingTransitions(Outgoing, false);

	int32 LineCount = 0;
	int32 ChoiceCount = 0;
	for (const USMTransitionInstance* Transition : Outgoing)
	{
		const USMStateInstance_Base* Destination = Transition->GetNextStateInstance();
		if (Destination && Destination->IsA<UDialogueLineNode>())
		{
			LineCount++;
		}
		else if (Destination && Destination->IsA<UDialogueChoiceNode>())
		{
			ChoiceCount++;
		}
		else
		{
			CompilerLog->Log(ESMCompilerLogType::Error,
				TEXT("Dialogue transitions must lead to a Dialogue Line or Dialogue Choice node."));
			return;
		}
	}

	if (LineCount > 1 || (LineCount == 1 && ChoiceCount > 0))
	{
		CompilerLog->Log(ESMCompilerLogType::Error,
			TEXT("A dialogue node can have one outgoing Dialogue Line or any number of outgoing Dialogue Choices. ")
			TEXT("Remove the extra Line or mixed connections."));
	}
}
#endif

UDialogueLineNode::UDialogueLineNode()
{
#if WITH_EDITORONLY_DATA
	bUseCustomColors = true;
	NodeColor = FLinearColor(0.12f, 0.55f, 0.22f);
	NodeEndStateColor = NodeColor;
#endif
}

FText UDialogueLineNode::GetSpeakerName() const
{
	if (ToroCharacterTags::IsValid(Speaker))
	{
		if (UToroDatabaseSubsystem* Database = UToroDatabaseSubsystem::Get(GetContext()))
		{
			if (const FToroCharacterEntry* CharInfo = Database->FindData<FToroCharacterEntry>(Speaker))
			{
				return CharInfo->DisplayName;
			}
		}
	}

	return Speaker.IsValid()
		? FText::FromName(Speaker.GetTagLeafName())
		: Super::GetSpeakerName();
}

UDialogueChoiceNode::UDialogueChoiceNode()
{
#if WITH_EDITORONLY_DATA
	bUseCustomColors = true;
	NodeColor = FLinearColor(0.12f, 0.38f, 0.78f);
	NodeEndStateColor = NodeColor;
#endif
}

FText UDialogueChoiceNode::GetSpeakerName() const
{
	if (UToroDatabaseSubsystem* Database = UToroDatabaseSubsystem::Get(GetContext()))
	{
		if (const FToroCharacterEntry* CharInfo = Database->FindData
			<FToroCharacterEntry>(ToroCharacterTags::Player))
		{
			return CharInfo->DisplayName;
		}
	}

	return INVTEXT("You");
}

#if WITH_EDITOR
void UDialogueChoiceNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChanged)
{
	Super::PostEditChangeProperty(PropertyChanged);
	if (AutoShortText != 0)
	{
		const FString LineStr = Line.Result.ToString().TrimStartAndEnd();
		if (LineStr.IsEmpty())
		{
			ShortText = INVTEXT("<Missing Line>");
			return;
		}

		FString ShortStr = LineStr.Mid(0, FMath::Min(LineStr.Len(), static_cast<int32>(AutoShortText)));
		if (LineStr.Len() > AutoShortText)
		{
			while (ShortStr.RemoveFromEnd(TEXT("."))) {}
			ShortStr.Append(TEXT("..."));
		}

		ShortText = FText::FromString(ShortStr);
	}
}
#endif
