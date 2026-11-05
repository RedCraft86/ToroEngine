// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "WorldMusic/WorldMusicSubsystem.h"
#include "Framework/ToroWorldSettings.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"
#include "Utilities/ToroUtilities.h"
#include "Utilities/ToroLogging.h"
#include "MetasoundSource.h"
#include "Engine/World.h"

namespace
{
	/** Returns the additive sum times the multiplier product, or zero for an empty array. */
	float ProcessModifiers(const TArray<TPair<EWorldMusicModType, float>>& Modifiers)
	{
		float Sum = 0.0f, Product = 1.0f;
		for (const TPair<EWorldMusicModType, float>& Modifier : Modifiers)
		{
			switch (Modifier.Key)
			{
			case EWorldMusicModType::Additive:
				Sum += Modifier.Value;
				break;

			case EWorldMusicModType::Multiplier:
				Product *= Modifier.Value;
				break;
			}
		}
		return Sum * Product;
	}
}

bool FWorldMusicSourceRef::IsValid() const
{
	return ::IsValid(Source) && FadeTime >= 0.0f && FMath::IsFinite(FadeTime);
}

void FWorldMusicSourceRef::Start() const
{
	if (IsValid())
	{
		Source->SetPaused(false);
		if (FMath::IsNearlyZero(FadeTime))
		{
			Source->Play();
		}
		else
		{
			Source->FadeIn(FadeTime);
		}
	}
}

void FWorldMusicSourceRef::Stop() const
{
	if (IsValid())
	{
		if (FMath::IsNearlyZero(FadeTime))
		{
			Source->Stop();
		}
		else
		{
			Source->FadeOut(FadeTime, 0.0f);
		}
	}
}

void FWorldMusicSourceRef::SetPaused(bool bPaused) const
{
	if (IsValid())
	{
		Source->SetPaused(bPaused);
	}
}

void FWorldMusicSourceRef::Destroy()
{
	if (::IsValid(Source))
	{
		Source->OnAudioFinishedNative.Clear();
		Source->Stop();

		Source->bAllowAnyoneToDestroyMe = true;
		Source->DestroyComponent();

		Source = nullptr;
	}
}

UWorldMusicSubsystem::UWorldMusicSubsystem(): DefaultFadeTime(1.0f), LastMoodIntensity(0.0f)
{
	DipRequests = TAccumulatorSet<TWeakObjectPtr<const UObject>>::Make(
		[](const TWeakObjectPtr<const UObject>& Key) -> bool
		{
			return Key.IsValid();
		}
	);
	if (DipRequests.IsValid())
	{
		DipRequests->OnChanged().AddUObject(this, &UWorldMusicSubsystem::OnUpdateParameters);
	}

	Pressure = TAccumulatorMap<TWeakObjectPtr<const UObject>, TPair<EWorldMusicModType, float>>::Make(
		[](const TWeakObjectPtr<const UObject>& Key, const TPair<EWorldMusicModType, float>&) -> bool
		{
			return Key.IsValid();
		}
	);
	if (Pressure.IsValid())
	{
		Pressure->OnChanged().AddUObject(this, &UWorldMusicSubsystem::OnUpdateParameters);
	}

	Tension = TAccumulatorMap<TWeakObjectPtr<const UObject>, TPair<EWorldMusicModType, float>>::Make(
		[](const TWeakObjectPtr<const UObject>& Key, const TPair<EWorldMusicModType, float>&) -> bool
		{
			return Key.IsValid();
		}
	);
	if (Tension.IsValid())
	{
		Tension->OnChanged().AddUObject(this, &UWorldMusicSubsystem::OnUpdateParameters);
	}
}

UWorldMusicSubsystem* UWorldMusicSubsystem::Get(const UObject* Context)
{
	const UWorld* World = ToroEngine::GetWorld(Context);
	return IsValid(World) ? World->GetSubsystem<UWorldMusicSubsystem>() : nullptr;
}

bool UWorldMusicSubsystem::SetCurrentTrack(USoundBase* InTrack, float FadeTime, bool bResetParams)
{
	if (!IsSystemValid())
	{
		TORO_LOG_ERROR(TEXT("[%s] Cannot set music track %s: world settings or accumulators are invalid."),
			*GetName(), *GetNameSafe(InTrack));
		return false;
	}

	if (!IsValid(InTrack))
	{
		TORO_LOG_WARNING(TEXT("[%s] Cannot set music track: track %s is invalid."),
			*GetName(), *GetNameSafe(InTrack));
		return false;
	}

	CleanupAudioSources();

	if (!AudioSources[0].IsValid() || AudioSources[0].Source->Sound != InTrack
		|| AudioSources[0].Source->GetPlayState() == EAudioComponentPlayState::Stopped)
	{
		if (bResetParams)
		{
			DipRequests->GetInnerSet().Empty();
			Pressure->GetInnerMap().Empty();
			Tension->GetInnerMap().Empty();

			if (GetConfig().BasePressure.IsSet())
			{
				Pressure->GetInnerMap().Add(WorldSettings.Get(), {
					EWorldMusicModType::Additive, GetConfig().BasePressure.Get(0.0f)
				});
			}

			if (GetConfig().BaseTension.IsSet())
			{
				Tension->GetInnerMap().Add(WorldSettings.Get(), {
					EWorldMusicModType::Additive, GetConfig().BaseTension.Get(0.0f)
				});
			}
		}

		return StartTrackInternal(InTrack, FadeTime);
	}

	return true;
}

void UWorldMusicSubsystem::SetTracksPaused(bool bPaused)
{
	for (FWorldMusicSourceRef& Source : AudioSources)
	{
		Source.SetPaused(bPaused);
	}
}

void UWorldMusicSubsystem::RequestDip(const UObject* Source) const
{
	if (DipRequests)
	{
		DipRequests->Add(Source);
	}
}

void UWorldMusicSubsystem::RemoveDip(const UObject* Source) const
{
	if (DipRequests)
	{
		DipRequests->Remove(Source);
	}
}

void UWorldMusicSubsystem::AddPressure(const UObject* Source, float Value, EWorldMusicModType Type) const
{
	if (Pressure)
	{
		Pressure->Add(Source, {Type, Value});
	}
}

void UWorldMusicSubsystem::RemovePressure(const UObject* Source) const
{
	if (Pressure)
	{
		Pressure->Remove(Source);
	}
}

void UWorldMusicSubsystem::AddTension(const UObject* Source, float Value, EWorldMusicModType Type) const
{
	if (Tension)
	{
		Tension->Add(Source, {Type, Value});
	}
}

void UWorldMusicSubsystem::RemoveTension(const UObject* Source) const
{
	if (Tension)
	{
		Tension->Remove(Source);
	}
}

bool UWorldMusicSubsystem::IsSystemValid() const
{
	return WorldSettings.IsValid()
		&& DipRequests.IsValid()
		&& Pressure.IsValid()
		&& Tension.IsValid();
}

const FWorldMusicSystemConfig& UWorldMusicSubsystem::GetConfig() const
{
	return WorldSettings->GetWorldMusicConfig();
}

void UWorldMusicSubsystem::SetAudioParameters(float FinalPressure, float FinalTension)
{
	if (IsSystemValid())
	{
		static const FName PARAM_Dipped(TEXT("Dipped"));
		static const FName PARAM_Pressure(TEXT("Pressure"));
		static const FName PARAM_Tension(TEXT("Tension"));
		static const FName PARAM_MoodDelta(TEXT("MoodDelta"));

		// Keep weights nonnegative and summing to one for a normalized mood intensity.
		constexpr float PressureWeight = 0.7f;
		constexpr float TensionWeight = 1.0f - PressureWeight;

		FinalPressure = FMath::Clamp(FinalPressure, 0.0f, 1.0f);
		FinalTension = FMath::Clamp(FinalTension, 0.0f, 1.0f);

		const float CurrentMood = FinalPressure * PressureWeight + FinalTension * TensionWeight;
		const float MoodDelta = CurrentMood - LastMoodIntensity;
		LastMoodIntensity = CurrentMood;

		for (int32 i = 0; i < AudioSources.Num(); i++)
		{
			if (AudioSources[i].IsValid())
			{
				const bool bIsCurrent = i == 0;
				AudioSources[i].Source->SetBoolParameter(
					PARAM_Dipped,
					!DipRequests->IsEmpty()
				);

				AudioSources[i].Source->SetFloatParameter(
					PARAM_Pressure,
					bIsCurrent ? FinalPressure : 0.0f
				);

				AudioSources[i].Source->SetFloatParameter(
					PARAM_Tension,
					bIsCurrent ? FinalTension : 0.0f
				);

				AudioSources[i].Source->SetFloatParameter(
					PARAM_MoodDelta,
					bIsCurrent ? MoodDelta : 0.0f
				);
			}
		}
	}
}

bool UWorldMusicSubsystem::StartTrackInternal(USoundBase* InTrack, float InFadeTime)
{
	if (!FMath::IsFinite(InFadeTime) || InFadeTime < 0.0f)
	{
		TORO_LOG_WARNING(TEXT("[%s] Music track %s has invalid fade duration %g; using zero seconds."),
			*GetName(), *GetNameSafe(InTrack), InFadeTime
		);
		InFadeTime = 0.0f;
	}

	UAudioComponent* NewSource = UGameplayStatics::CreateSound2D(this, InTrack);
	if (!IsValid(NewSource))
	{
		TORO_LOG_ERROR(TEXT("[%s] Cannot start music track %s: audio component creation failed."),
			*GetName(), *GetNameSafe(InTrack));
		return false;
	}

	NewSource->SetPaused(true);
	NewSource->OnAudioFinishedNative.AddUObject(this, &UWorldMusicSubsystem::OnAudioFinished);

	CleanupAudioSources();
	const FWorldMusicSourceRef OutgoingRef = AudioSources[0];
	AudioSources[0] = FWorldMusicSourceRef(NewSource, InFadeTime);
	if (OutgoingRef.IsValid())
	{
		AudioSources.Add(OutgoingRef);
		OutgoingRef.Stop();
	}

	OnUpdateParameters();
	AudioSources[0].Start();
	return true;
}

void UWorldMusicSubsystem::CleanupAudioSources()
{
	if (AudioSources.IsEmpty())
	{
		AudioSources.Emplace();
	}
	else if (!AudioSources[0].IsValid())
	{
		AudioSources[0] = FWorldMusicSourceRef();
	}

	for (int32 i = AudioSources.Num() - 1; i > 0; i--)
	{
		if (!AudioSources[i].IsValid())
		{
			AudioSources.RemoveAtSwap(i);
		}
	}
}

void UWorldMusicSubsystem::OnPostGarbageCollect() const
{
	if (DipRequests.IsValid())
	{
		DipRequests->Cleanup();
	}
	if (Pressure.IsValid())
	{
		Pressure->Cleanup();
	}
	if (Tension.IsValid())
	{
		Tension->Cleanup();
	}
}

void UWorldMusicSubsystem::OnAudioFinished(UAudioComponent* Component)
{
	if (!AudioSources.IsEmpty() && AudioSources[0].Source == Component)
	{
		AudioSources[0] = FWorldMusicSourceRef();
	}

	for (int32 i = AudioSources.Num() - 1; i > 0; i--)
	{
		if (AudioSources[i].Source == Component || !AudioSources[i].IsValid())
		{
			AudioSources.RemoveAtSwap(i);
		}
	}

	if (IsValid(Component))
	{
		Component->OnAudioFinishedNative.RemoveAll(this);
	}
}

void UWorldMusicSubsystem::OnUpdateParameters()
{
	if (IsSystemValid())
	{
		float FinalPressure = 0.0f;
		if (Pressure.IsValid())
		{
			TArray<TPair<EWorldMusicModType, float>> Modifiers;
			Pressure->GetInnerMap().GenerateValueArray(Modifiers);
			FinalPressure = ProcessModifiers(Modifiers);
		}

		float FinalTension = 0.0f;
		if (Tension.IsValid())
		{
			TArray<TPair<EWorldMusicModType, float>> Modifiers;
			Tension->GetInnerMap().GenerateValueArray(Modifiers);
			FinalTension = ProcessModifiers(Modifiers);
		}

		SetAudioParameters(FinalPressure, FinalTension);
	}
}

bool UWorldMusicSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game
		|| WorldType == EWorldType::PIE;
}

void UWorldMusicSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject(this, &UWorldMusicSubsystem::OnPostGarbageCollect);

	const UWorld* World = GetWorld();
	if (!ensureMsgf(IsValid(World), TEXT("[%s] Cannot initialize World Music due to an invalid world."), *GetName()))
	{
		return;
	}

	WorldSettings = Cast<AToroWorldSettings>(World->GetWorldSettings());
	if (!ensureMsgf(WorldSettings.IsValid(),
		TEXT("[%s] Cannot initialize World Music due to invalid World Settings actor.")
		TEXT("Did you forget to change it to AToroWorldSettings or its subclass?"), *GetName()))
	{
		return;
	}

	if (!ensureMsgf(DipRequests.IsValid() && Pressure.IsValid() && Tension.IsValid(),
		TEXT("[%s] Cannot initialize World Music due to DipRequests, Pressure, and Tension accumulators being null."),
		*GetName()))
	{
		return;
	}

	if (GetConfig().BasePressure.IsSet())
	{
		Pressure->Add(WorldSettings.Get(), {
			EWorldMusicModType::Additive, GetConfig().BasePressure.Get(0.0f)
		});
	}

	if (GetConfig().BaseTension.IsSet())
	{
		Tension->Add(WorldSettings.Get(), {
			EWorldMusicModType::Additive, GetConfig().BaseTension.Get(0.0f)
		});
	}

	DefaultTrack = GetConfig().DefaultTrack.LoadSynchronous();
	DefaultFadeTime = GetConfig().DefaultFadeTime;
}

void UWorldMusicSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (IsSystemValid() && IsValid(DefaultTrack))
	{
		StartTrackInternal(DefaultTrack, DefaultFadeTime);
	}
}

void UWorldMusicSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::GetPostGarbageCollect().RemoveAll(this);

	for (auto It = AudioSources.CreateIterator(); It; ++It)
	{
		It->Destroy();
		It.RemoveCurrentSwap();
	}

	LastMoodIntensity = 0.0f;
	DefaultTrack = nullptr;
	WorldSettings.Reset();
	DipRequests.Reset();
	Pressure.Reset();
	Tension.Reset();

	Super::Deinitialize();
}
