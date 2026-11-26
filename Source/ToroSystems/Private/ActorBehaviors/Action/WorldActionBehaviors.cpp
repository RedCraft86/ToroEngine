// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Action/WorldActionBehaviors.h"
#include "AsyncGameplayMessageSystem.h"
#include "AsyncMessageWorldSubsystem.h"
#include "Dialogues/DialogueGraph.h"
#include "Dialogues/DialogueSubsystem.h"
#include "Engine/World.h"
#include "GameFlagManager.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Libraries/ToroWorldLibrary.h"
#include "MovieSceneSequencePlayer.h"
#include "Objectives/ObjectiveSubsystem.h"
#include "Objectives/ObjectiveSystemConfig.h"
#include "StructUtils/StructView.h"

void UDataLayerActionBehavior::OnTriggered()
{
	for (const auto& [DataLayer, State] : DataLayerStates)
	{
		UToroWorldLibrary::SetDataLayerState(this, DataLayer.Get(), State);
	}
}

void ULevelSequenceActionBehavior::OnTriggered()
{
	const ALevelSequenceActor* ActorPtr = Sequence.Get();
	if (ULevelSequencePlayer* SequencePlayer = ActorPtr ? ActorPtr->GetSequencePlayer() : nullptr)
	{
		if (bReverse)
		{
			if (bRestart)
			{
				SequencePlayer->GoToEndAndStop();
			}

			SequencePlayer->PlayReverse();
		}
		else
		{
			if (bRestart)
			{
				SequencePlayer->SetPlaybackPosition(FMovieSceneSequencePlaybackParams(
					SequencePlayer->GetStartTime().Time, EUpdatePositionMethod::Jump));
			}

			SequencePlayer->Play();
		}
	}
}

void URemoteEventActionBehavior::OnTriggered()
{
	UToroWorldLibrary::CallRemoteEvent(this, EventName);
}

void UAsyncMessageActionBehavior::OnTriggered()
{
	if (MessageId.IsValid())
	{
		const TSharedPtr<FAsyncGameplayMessageSystem> System = UAsyncMessageWorldSubsystem::
			GetSharedMessageSystem<FAsyncGameplayMessageSystem>(GetWorld());
		if (System.IsValid())
		{
			System->QueueMessageForBroadcast(MessageId, FConstStructView(Payload));
		}
	}
}

bool UDialogueActionBehavior::CanTrigger() const
{
	return Dialogue && !Dialogue->HasAnyClassFlags(CLASS_Abstract);
}

void UDialogueActionBehavior::OnTriggered()
{
	UDialogueSubsystem::Start(this, Dialogue);
}

bool UObjectiveTaskActionBehavior::CanTrigger() const
{
	const UWorld* World = GetWorld();
	return Amount > 0 && ObjectiveTags::IsValid(Task) && World && World->GetSubsystem<UObjectiveSubsystem>();
}

void UObjectiveTaskActionBehavior::OnTriggered()
{
	if (const UWorld* World = GetWorld())
	{
		if (UObjectiveSubsystem* Objectives = World->GetSubsystem<UObjectiveSubsystem>())
		{
			Objectives->FinishTask(Task, Amount);
		}
	}
}

bool UGameFlagActionBehavior::CanTrigger() const
{
	return Flag.IsValid() && UGameFlagManager::Get(this);
}

void UGameFlagActionBehavior::OnTriggered()
{
	if (UGameFlagManager* Flags = UGameFlagManager::Get(this))
	{
		Flags->SetGameFlag(Flag, Scope);
	}
}
