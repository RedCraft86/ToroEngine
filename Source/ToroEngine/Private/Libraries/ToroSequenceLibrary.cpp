// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroSequenceLibrary.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "MovieSceneSequence.h"
#include "UObject/Stack.h"

namespace
{
	UE5Coro::TCoroutine<> AwaitSequenceFinished(ULevelSequencePlayer& Player)
	{
		co_await Player.OnFinished;
	}

	UE5Coro::TCoroutine<> AwaitSequenceStopped(ULevelSequencePlayer& Player)
	{
		co_await Player.OnStop;
	}
}

void UToroSequenceLibrary::StopLevelSequence(const ALevelSequenceActor* Target, EToroSequenceStopType StopType)
{
	const TWeakObjectPtr<ULevelSequencePlayer> SequencePlayer(IsValid(Target) ? Target->GetSequencePlayer() : nullptr);
	if (SequencePlayer.IsValid())
	{
		const EMovieSceneCompletionModeOverride OrigCompletion = SequencePlayer->GetCompletionModeOverride();
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
		if (SequencePlayer.IsValid())
		{
			SequencePlayer->SetCompletionModeOverride(OrigCompletion);
		}
	}
}

bool UToroSequenceLibrary::PlayLevelSequence(const ALevelSequenceActor* Target,
	EToroAnimationPlayMode PlayMode, float PlayRate, bool bResumePlay)
{
	if (!FMath::IsFinite(PlayRate))
	{
		FFrame::KismetExecutionMessage(TEXT("PlayRate must be finite."), ELogVerbosity::Error);
		return false;
	}

	if (!IsValid(Target) || !IsValid(Target->GetSequencePlayer())
		|| !IsValid(Target->GetSequencePlayer()->GetSequence())
		|| (PlayMode != EToroAnimationPlayMode::Forward && PlayMode != EToroAnimationPlayMode::Reverse))
	{
		return false;
	}

	const TWeakObjectPtr<const ALevelSequenceActor> WeakTarget(Target);
	const TWeakObjectPtr Player = Target->GetSequencePlayer();
	Player->SetPlayRate(FMath::Max(PlayRate, 0.1f));
	if (!bResumePlay)
	{
		Player->RewindForReplay();
		if (!WeakTarget.IsValid()
			|| !Player.IsValid()
			|| WeakTarget->GetSequencePlayer() != Player.Get()
			|| !IsValid(Player->GetSequence()))
		{
			return false;
		}
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

	if (!FMath::IsFinite(PlayRate))
	{
		FFrame::KismetExecutionMessage(TEXT("PlayRate must be finite."), ELogVerbosity::Error);
		co_return;
	}

	if (!IsValid(Target) || !IsValid(Target->GetSequencePlayer())
		|| !IsValid(Target->GetSequencePlayer()->GetSequence())
		|| (PlayMode != EToroAnimationPlayMode::Forward && PlayMode != EToroAnimationPlayMode::Reverse))
	{
		co_return;
	}

	const TWeakObjectPtr<const ALevelSequenceActor> WeakTarget(Target);
	const TWeakObjectPtr Player = Target->GetSequencePlayer();
	if (!bResumePlay)
	{
		// This needs to be called before we bind the delegates since it calls Stop()
		Player->RewindForReplay();

		if (!WeakTarget.IsValid()
			|| !Player.IsValid()
			|| WeakTarget->GetSequencePlayer() != Player.Get()
			|| !IsValid(Player->GetSequence()))
		{
			co_return;
		}
	}

	const float ExpectedTime = (PlayMode == EToroAnimationPlayMode::Forward ? Player->GetEndTime() : Player->GetStartTime()).AsSeconds();
	auto OnSequenceEnded = UE5Coro::Race(AwaitSequenceFinished(*Player), AwaitSequenceStopped(*Player));
	if (PlayLevelSequence(Target, PlayMode, PlayRate, true))
	{
		co_await OnSequenceEnded;
		bSuccess = Player.IsValid() && FMath::IsNearlyEqual(Player->GetCurrentTime().AsSeconds(), ExpectedTime, 0.1f);
	}
}
