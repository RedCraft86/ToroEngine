// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Action/Actor/AmbientSoundActionBehavior.h"
#include "Components/AudioComponent.h"
#include "Sound/AmbientSound.h"

void UAmbientSoundActionBehavior::OnTriggered()
{
	const AAmbientSound* Actor = Target.Get();
	UAudioComponent* Audio = IsValid(Actor) ? Actor->GetAudioComponent() : nullptr;
	if (!IsValid(Audio))
	{
		return;
	}

	switch (Action)
	{
	case EToroAmbientSoundAction::Play:
		if (FadeTime >= 0.1f)
		{
			Audio->FadeIn(FadeTime, 1.0f, StartTime);
		}
		else
		{
			Audio->Play(StartTime);
		}
		break;
	case EToroAmbientSoundAction::Stop:
		if (FadeTime >= 0.1f)
		{
			Audio->FadeOut(FadeTime, 0.0f);
		}
		else
		{
			Audio->Stop();
		}
		break;
	case EToroAmbientSoundAction::Pause:
		Audio->SetPaused(true);
		break;
	case EToroAmbientSoundAction::Unpause:
		Audio->SetPaused(false);
		break;
	}
}
