// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Action/SoundActionBehaviors.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/AmbientSound.h"
#include "Sound/SoundBase.h"
#include "CoreTypes.h"

void USound2DActionBehavior::OnTriggered()
{
	if (USoundBase* SoundPtr = Sound.LoadSynchronous())
	{
		UGameplayStatics::PlaySound2D(this, SoundPtr, Volume, Pitch, StartTime);
	}
}

void UAmbientSoundActionBehavior::OnTriggered()
{
	const AAmbientSound* ActorPtr = SoundActor.Get();
	if (UAudioComponent* AudioComp = ActorPtr ? ActorPtr->GetAudioComponent() : nullptr)
	{
		if (Action == EToroAmbientSoundAction::Play)
		{
			if (FadeTime >= 0.1f)
			{
				AudioComp->FadeIn(FadeTime, 1.0f, StartTime);
			}
			else
			{
				AudioComp->Play(StartTime);
			}
		}
		else if (Action == EToroAmbientSoundAction::Stop)
		{
			if (FadeTime >= 0.1f)
			{
				AudioComp->FadeOut(FadeTime, 0.0f);
			}
			else
			{
				AudioComp->Stop();
			}
		}
		else
		{
			AudioComp->SetPaused(Action == EToroAmbientSoundAction::Pause);
		}
	}
}
