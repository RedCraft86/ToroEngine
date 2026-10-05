// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Libraries/ToroSoundLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/AmbientSound.h"

void UToroSoundLibrary::PlayAmbientSound(const AAmbientSound* Target, float StartTime)
{
	if (UAudioComponent* Audio = IsValid(Target) ? Target->GetAudioComponent() : nullptr)
	{
		Audio->Play(StartTime);
	}
}

void UToroSoundLibrary::StopAmbientSound(const AAmbientSound* Target, float Delay)
{
	if (UAudioComponent* Audio = IsValid(Target) ? Target->GetAudioComponent() : nullptr)
	{
		const float AbsDelay = FMath::Abs(Delay);
		if (AbsDelay > UE_KINDA_SMALL_NUMBER)
		{
			Audio->StopDelayed(AbsDelay);
		}
		else
		{
			Audio->Stop();
		}
	}
}

FVoidCoroutine UToroSoundLibrary::FadeInAmbientSound(FLatentActionInfo LatentInfo, const AAmbientSound* Target,
	float Duration, float TargetLevel, float StartTime, EAudioFaderCurve FadeCurve)
{
	if (!FMath::IsFinite(Duration))
	{
		co_return;
	}

	const float AdjustedTime = FMath::Max(0.0f, Duration);
	if (UAudioComponent* Audio = IsValid(Target) ? Target->GetAudioComponent() : nullptr)
	{
		Audio->FadeIn(AdjustedTime, TargetLevel, StartTime, FadeCurve);
		if (AdjustedTime > 0.0f)
		{
			co_await UE5Coro::Latent::Seconds(AdjustedTime);
		}
	}

	co_return;
}

FVoidCoroutine UToroSoundLibrary::FadeOutAmbientSound(FLatentActionInfo LatentInfo, const AAmbientSound* Target,
	float Duration, float TargetLevel, EAudioFaderCurve FadeCurve)
{
	if (!FMath::IsFinite(Duration))
	{
		co_return;
	}

	const float AdjustedTime = FMath::Max(0.0f, Duration);
	if (UAudioComponent* Audio = IsValid(Target) ? Target->GetAudioComponent() : nullptr)
	{
		Audio->FadeOut(AdjustedTime, TargetLevel, FadeCurve);
		if (AdjustedTime > 0.0f)
		{
			co_await UE5Coro::Latent::Seconds(AdjustedTime);
		}
	}

	co_return;
}
