// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Cutscenes/CutsceneSubtitleTrack.h"
#include "Cutscenes/CutsceneActor.h"
#include "EntitySystem/MovieSceneInstanceRegistry.h"
#include "EntitySystem/MovieSceneSequenceInstance.h"
#include "LevelSequencePlayer.h"
#include "IMovieScenePlayer.h"

UCutsceneSubtitleTrack::UCutsceneSubtitleTrack()
{
	TrackType = ECustomSequencerTrackType::ObjectTrack;
	SupportedObjectType = ACutsceneActor::StaticClass();
	DefaultSectionType = UCutsceneSubtitleSection::StaticClass();
	TrackInstanceType = UCutsceneSubtitleTrackInstance::StaticClass();
	EvalOptions.bEvaluateInPostroll = false;
	EvalOptions.bEvaluateInPreroll = false;
	bSupportsMultipleRows = false;
	bSupportsBlending = false;
}

void UCutsceneSubtitleTrackInstance::OnEndUpdateInputs()
{
	using namespace UE::MovieScene;

	ACutsceneActor* Actor = Cast<ACutsceneActor>(GetAnimatedObject());
	const ULevelSequencePlayer* Player = Actor ? Actor->GetSequencePlayer() : nullptr;
	if (!Player)
	{
		ClearSubtitle();
		return;
	}

	const UCutsceneSubtitleSection* Cue = nullptr;
	const FInstanceRegistry* Registry = GetLinker()->GetInstanceRegistry();
	for (const FMovieSceneTrackInstanceInput& Input : UMovieSceneTrackInstance::GetInputs())
	{
		const UCutsceneSubtitleSection* Section = Cast<UCutsceneSubtitleSection>(Input.Section);
		if (!Section || !Registry->IsHandleValid(Input.InstanceHandle))
		{
			continue;
		}

		const FSequenceInstance& Instance = Registry->GetInstance(Input.InstanceHandle);
		if (FPlayerIndexPlaybackCapability::GetPlayer(Instance.GetSharedPlaybackState()) != Player)
		{
			continue;
		}

		if (!Cue || Section->GetOverlapPriority() > Cue->GetOverlapPriority())
		{
			Cue = Section;
		}
	}

	if (Cue)
	{
		bHasPublishedCue = true;
		Actor->SetTrackSubtitle(Cue->Subtitle);
	}
	else
	{
		ClearSubtitle();
	}
}

void UCutsceneSubtitleTrackInstance::OnDestroyed()
{
	ClearSubtitle();
}

void UCutsceneSubtitleTrackInstance::ClearSubtitle()
{
	if (bHasPublishedCue)
	{
		bHasPublishedCue = false;
		if (ACutsceneActor* Actor = Cast<ACutsceneActor>(GetAnimatedObject()))
		{
			Actor->SetTrackSubtitle(FText::GetEmpty());
		}
	}
}

#if WITH_EDITORONLY_DATA
FText UCutsceneSubtitleTrack::GetDefaultDisplayName() const
{
	return NSLOCTEXT("CutsceneSubtitleTrack", "TrackName", "Cutscene Subtitles");
}
#endif