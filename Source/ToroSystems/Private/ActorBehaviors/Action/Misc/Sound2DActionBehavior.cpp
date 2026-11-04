// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Action/Misc/Sound2DActionBehavior.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void USound2DActionBehavior::OnTriggered()
{
	if (USoundBase* SoundAsset = Sound.LoadSynchronous(); IsValid(SoundAsset))
	{
		UGameplayStatics::PlaySound2D(this, SoundAsset, Volume, Pitch, StartTime);
	}
}
