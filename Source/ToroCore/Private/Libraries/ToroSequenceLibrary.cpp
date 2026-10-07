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

bool UToroSequenceLibrary::PlayLevelSequence(const ALevelSequenceActor* Target, EToroAnimationPlayMode PlayMode, float PlayRate, bool bResumePlay)
{
	if (!IsValid(Target) || !IsValid(Target->GetSequencePlayer()) || !FMath::IsFinite(PlayRate))
	{
		return false;
	}

	const TWeakObjectPtr Player = Target->GetSequencePlayer();
	if (!bResumePlay)
	{
		Player->RewindForReplay();
	}

	switch (PlayMode)
	{
	case EToroAnimationPlayMode::Forward:
		Player->Play();
		break;

	case EToroAnimationPlayMode::Reverse:
		Player->PlayReverse();
		break;
	}

	return true;
}

FVoidCoroutine UToroSequenceLibrary::PlayLevelSequenceAsync(FLatentActionInfo LatentInfo, bool& bSuccess,
	const ALevelSequenceActor* Target, EToroAnimationPlayMode PlayMode, float PlayRate, bool bResumePlay)
{
	bSuccess = false;
	if (!IsValid(Target) || !IsValid(Target->GetSequencePlayer()) || !FMath::IsFinite(PlayRate))
	{
		co_return;
	}

	const TWeakObjectPtr Player = Target->GetSequencePlayer();
	if (!bResumePlay)
	{
		// This needs to be called before we bind the delegates since it calls Stop()
		Player->RewindForReplay();
	}

	const float ExpectedTime = (PlayMode == EToroAnimationPlayMode::Forward ? Player->GetEndTime() : Player->GetStartTime()).AsSeconds();
	auto OnSequenceEnded = UE5Coro::Race(WaitForSequenceFinished(*Player), WaitForSequenceStopped(*Player));
	if (PlayLevelSequence(Target, PlayMode, PlayRate, true))
	{
		co_await OnSequenceEnded;
		bSuccess = Player.IsValid() && FMath::IsNearlyEqual(Player->GetCurrentTime().AsSeconds(), ExpectedTime, 0.1f);
	}

	co_return;
}
