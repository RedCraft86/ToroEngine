// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroSoundLibrary.h"
#include "Sound/AmbientSound.h"

void UToroSoundLibrary::PlayAmbientSound(const AAmbientSound* Target, float StartTime)
{
	UAudioComponent* Audio = IsValid(Target) ? Target->GetAudioComponent() : nullptr;
	if (IsValid(Audio))
	{
		Audio->Play(FMath::IsFinite(StartTime) ? FMath::Max(StartTime, 0.0f) : 0.0f);
	}
}

void UToroSoundLibrary::StopAmbientSound(const AAmbientSound* Target, float Delay)
{
	UAudioComponent* Audio = IsValid(Target) ? Target->GetAudioComponent() : nullptr;
	if (IsValid(Audio))
	{
		const float AdjustedDelay = FMath::IsFinite(Delay) ? FMath::Max(Delay, 0.0f) : 0.0f;
		if (AdjustedDelay > UE_KINDA_SMALL_NUMBER)
		{
			Audio->StopDelayed(AdjustedDelay);
		}
		else
		{
			Audio->Stop();
		}
	}
}

FVoidCoroutine UToroSoundLibrary::FadeInAmbientSound(FLatentActionInfo LatentInfo,
	const AAmbientSound* Target, float Duration, float TargetLevel, float StartTime, EAudioFaderCurve FadeCurve)
{
	if (!FMath::IsFinite(StartTime) || !FMath::IsFinite(TargetLevel))
	{
		co_return;
	}

	const float AdjustedDuration = FMath::IsFinite(Duration) ? FMath::Max(0.0f, Duration) : 0.0f;
	UAudioComponent* Audio = IsValid(Target) ? Target->GetAudioComponent() : nullptr;
	if (IsValid(Audio))
	{
		Audio->FadeIn(AdjustedDuration, TargetLevel, StartTime, FadeCurve);
		if (AdjustedDuration > 0.0f)
		{
			co_await UE5Coro::Latent::Seconds(AdjustedDuration);
		}
	}
}

FVoidCoroutine UToroSoundLibrary::FadeOutAmbientSound(FLatentActionInfo LatentInfo,
	const AAmbientSound* Target, float Duration, float TargetLevel, EAudioFaderCurve FadeCurve)
{
	if (!FMath::IsFinite(TargetLevel))
	{
		co_return;
	}

	const float AdjustedTime = FMath::IsFinite(Duration) ? FMath::Max(0.0f, Duration) : 0.0f;
	UAudioComponent* Audio = IsValid(Target) ? Target->GetAudioComponent() : nullptr;
	if (IsValid(Audio))
	{
		Audio->FadeOut(AdjustedTime, TargetLevel, FadeCurve);
		if (AdjustedTime > 0.0f)
		{
			co_await UE5Coro::Latent::Seconds(AdjustedTime);
		}
	}
}
