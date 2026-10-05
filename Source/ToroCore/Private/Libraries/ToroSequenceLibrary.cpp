// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Libraries/ToroSequenceLibrary.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"

namespace
{
	/**
	 * Waits for the sequence player's natural-finish notification.
	 */
	UE5Coro::TCoroutine<> WaitForSequenceFinished(ULevelSequencePlayer& Player)
	{
		co_await Player.OnFinished;
	}

	/**
	 * Waits for the sequence player's stop notification.
	 */
	UE5Coro::TCoroutine<> WaitForSequenceStopped(ULevelSequencePlayer& Player)
	{
		co_await Player.OnStop;
	}
}

void UToroSequenceLibrary::StopLevelSequence(const ALevelSequenceActor* Target, EToroSequenceStopType StopType)
{
	if (ULevelSequencePlayer* SequencePlayer = IsValid(Target) ? Target->GetSequencePlayer() : nullptr)
	{
		switch (StopType)
		{
		case EToroSequenceStopType::Default:
			SequencePlayer->SetCompletionModeOverride(EMovieSceneCompletionModeOverride::None);
			SequencePlayer->Stop();
			break;

		case EToroSequenceStopType::Reset:
			SequencePlayer->SetCompletionModeOverride(EMovieSceneCompletionModeOverride::ForceRestoreState);
			SequencePlayer->Stop();
			break;

		case EToroSequenceStopType::CurrentTime:
			SequencePlayer->SetCompletionModeOverride(EMovieSceneCompletionModeOverride::ForceKeepState);
			SequencePlayer->StopAtCurrentTime();
			break;

		case EToroSequenceStopType::SkipToEnd:
			SequencePlayer->SetCompletionModeOverride(EMovieSceneCompletionModeOverride::ForceKeepState);
			SequencePlayer->GoToEndAndStop();
			break;
		}
	}
}

FVoidCoroutine UToroSequenceLibrary::PlayLevelSequence(FLatentActionInfo LatentInfo, bool& bSuccess,
	const ALevelSequenceActor* Target, float PlayRate, bool bWaitForFinished)
{
	bSuccess = false;
	if (!FMath::IsFinite(PlayRate))
	{
		co_return;
	}

	if (ULevelSequencePlayer* Player = IsValid(Target) ? Target->GetSequencePlayer() : nullptr)
	{
		Player->SetPlayRate(FMath::Max(0.1f, PlayRate));
		if (bWaitForFinished)
		{
			auto Ended = UE5Coro::Race(
				WaitForSequenceFinished(*Player),
				WaitForSequenceStopped(*Player) // Stop listening when stopped
			);

			Player->Play();
			co_await Ended;
		}
		else
		{
			Player->Play();
		}

		bSuccess = true;
	}

	co_return;
}

FVoidCoroutine UToroSequenceLibrary::ReverseLevelSequence(FLatentActionInfo LatentInfo, bool& bSuccess,
	const ALevelSequenceActor* Target, float PlayRate, bool bWaitForFinished)
{
	bSuccess = false;
	if (!FMath::IsFinite(PlayRate))
	{
		co_return;
	}

	if (ULevelSequencePlayer* Player = IsValid(Target) ? Target->GetSequencePlayer() : nullptr)
	{
		Player->SetPlayRate(FMath::Max(0.1f, PlayRate));
		if (bWaitForFinished)
		{
			auto Ended = UE5Coro::Race(
				WaitForSequenceFinished(*Player),
				WaitForSequenceStopped(*Player) // Stop listening when stopped
			);

			Player->PlayReverse();
			co_await Ended;
		}
		else
		{
			Player->PlayReverse();
		}

		bSuccess = true;
	}

	co_return;
}
