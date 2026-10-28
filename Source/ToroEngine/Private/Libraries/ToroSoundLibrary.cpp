// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroSoundLibrary.h"
#include "Utilities/ToroLogging.h"
#include "Sound/AmbientSound.h"

void UToroSoundLibrary::PlayAmbientSound(const AAmbientSound* Target, float StartTime)
{
	if (!FMath::IsFinite(StartTime))
	{
		TORO_REPORT_ERROR(TEXT("StartTime must be finite."));
		return;
	}

	UAudioComponent* Audio = IsValid(Target) ? Target->GetAudioComponent() : nullptr;
	if (IsValid(Audio))
	{
		Audio->Play(FMath::Max(StartTime, 0.0f));
	}
}

void UToroSoundLibrary::StopAmbientSound(const AAmbientSound* Target, float Delay)
{
	if (!FMath::IsFinite(Delay))
	{
		TORO_REPORT_ERROR(TEXT("Delay must be finite."));
		return;
	}

	const float AdjustedDelay = FMath::Max(Delay, 0.0f);
	UAudioComponent* Audio = IsValid(Target) ? Target->GetAudioComponent() : nullptr;
	if (IsValid(Audio))
	{
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
	if (!FMath::IsFinite(Duration))
	{
		TORO_REPORT_ERROR(TEXT("Duration must be finite."));
		co_return;
	}

	if (!FMath::IsFinite(TargetLevel))
	{
		TORO_REPORT_ERROR(TEXT("TargetLevel must be finite."));
		co_return;
	}

	if (!FMath::IsFinite(StartTime))
	{
		TORO_REPORT_ERROR(TEXT("StartTime must be finite."));
		co_return;
	}

	const float AdjustedDuration = FMath::Max(Duration, 0.0f);
	UAudioComponent* Audio = IsValid(Target) ? Target->GetAudioComponent() : nullptr;
	if (IsValid(Audio))
	{
		Audio->FadeIn(AdjustedDuration, TargetLevel, FMath::Max(StartTime, 0.0f), FadeCurve);
		if (AdjustedDuration > 0.0f)
		{
			co_await UE5Coro::Latent::Seconds(AdjustedDuration);
		}
	}
}

FVoidCoroutine UToroSoundLibrary::FadeOutAmbientSound(FLatentActionInfo LatentInfo,
	const AAmbientSound* Target, float Duration, float TargetLevel, EAudioFaderCurve FadeCurve)
{
	if (!FMath::IsFinite(Duration))
	{
		TORO_REPORT_ERROR(TEXT("Duration must be finite."));
		co_return;
	}

	if (!FMath::IsFinite(TargetLevel))
	{
		TORO_REPORT_ERROR(TEXT("TargetLevel must be finite."));
		co_return;
	}

	const float AdjustedDuration = FMath::Max(Duration, 0.0f);
	UAudioComponent* Audio = IsValid(Target) ? Target->GetAudioComponent() : nullptr;
	if (IsValid(Audio))
	{
		Audio->FadeOut(AdjustedDuration, TargetLevel, FadeCurve);
		if (AdjustedDuration > 0.0f)
		{
			co_await UE5Coro::Latent::Seconds(AdjustedDuration);
		}
	}
}
