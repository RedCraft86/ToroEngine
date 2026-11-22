// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Action/WorldActionBehaviors.h"
#include "AsyncGameplayMessageSystem.h"
#include "AsyncMessageWorldSubsystem.h"
#include "Engine/LatentActionManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Libraries/ToroWorldLibrary.h"
#include "MovieSceneSequencePlayer.h"
#include "StructUtils/StructView.h"
#include "Utilities/ToroUtilities.h"
#include "CoreTypes.h"

void ULevelStreamingActionBehavior::OnTriggered()
{
	const auto MakeStreamRequest = [this]
	{
		FLatentActionInfo Info = ToroEngine::LatentInfo(this);
		Info.Linkage = INDEX_NONE;
		return Info;
	};

	for (const TPair<TSoftObjectPtr<UWorld>, bool>& Level : Load)
	{
		if (!Level.Key.IsNull())
		{
			UGameplayStatics::LoadStreamLevelBySoftObjectPtr(this,
				Level.Key, Level.Value, false, MakeStreamRequest());
		}
	}
	for (const TSoftObjectPtr<UWorld>& Level : Unload)
	{
		if (!Level.IsNull())
		{
			UGameplayStatics::UnloadStreamLevelBySoftObjectPtr(this, Level, MakeStreamRequest(), false);
		}
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
