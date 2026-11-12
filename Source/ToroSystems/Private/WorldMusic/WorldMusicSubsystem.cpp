// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "WorldMusic/WorldMusicSubsystem.h"
#include "WorldMusic/WorldMusicSystemConfig.h"
#include "Framework/ToroWorldSettings.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/ToroUtilities.h"
#include "Utilities/ToroLogging.h"
#include "MetasoundSource.h"

float FWorldMusicParam::GetResult() const
{
	float Sum = 0.0f, Product = 1.0f;
	Entries->ForEachValue([&Sum, &Product](const TPair<EToroValueModifierType, float>& Pair) -> bool
	{
		switch (Pair.Key)
		{
		case EToroValueModifierType::Add:
			Sum += Pair.Value;
			break;

		case EToroValueModifierType::Multiply:
			Product *= Pair.Value;
			break;
		}

		return true;
	});

	return Sum * Product;
}

bool FWorldMusicSource::IsValid() const
{
	return IsInitialized() && ::IsValid(Source);
}

void FWorldMusicSource::Stop() const
{
	if (IsValid())
	{
		if (FMath::IsNearlyZero(FadeTime) || Source->bIsPaused)
		{
			Source->Stop();
		}
		else
		{
			Source->FadeOut(FadeTime, 0.0f);
		}
	}
}

void FWorldMusicSource::Start() const
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
			Source->FadeIn(FadeTime, 1.0f);
		}
	}
}

void FWorldMusicSource::SetPause(bool bInPaused) const
{
	if (IsValid())
	{
		Source->SetPaused(bInPaused);
	}
}

void FWorldMusicSource::Destroy()
{
	if (::IsValid(Source))
	{
		Source->OnAudioFinishedNative.Clear();
		if (Source->IsPlaying() || Source->bIsPaused)
		{
			Source->Stop();
		}

		Source->bAllowAnyoneToDestroyMe = true;
		Source->DestroyComponent();
	}

	Source = nullptr;
	FadeTime = -1.0f;
}

UWorldMusicSubsystem::UWorldMusicSubsystem()
	: bPauseParamUpdates(false)
{
	DipRequests->OnChanged().AddUObject(this, &UWorldMusicSubsystem::OnParamsChanged);
}

UWorldMusicSubsystem* UWorldMusicSubsystem::Get(const UObject* Context)
{
	const UWorld* World = ToroEngine::GetWorld(Context);
	return IsValid(World) ? World->GetSubsystem<UWorldMusicSubsystem>() : nullptr;
}

bool UWorldMusicSubsystem::ChangeTrack(USoundBase* InTrack, float FadeTime, bool bResetParams)
{
	if (!IsSystemValid())
	{
		TORO_LOG_ERROR(
			TEXT("[%s] Cannot set music track %s: world settings or accumulators are invalid."),
			*GetName(), *GetNameSafe(InTrack)
		);
		return false;
	}

	if (!IsValid(InTrack))
	{
		TORO_LOG_WARNING(
			TEXT("[%s] Cannot set music track: track %s is invalid."),
			*GetName(), *GetNameSafe(InTrack)
		);
		return false;
	}

	CleanupSources();

	if (bResetParams)
	{
		TGuardValue ParamsGuard(bPauseParamUpdates, true);

		DipRequests->Clear(true);
		Parameters.Empty();

		for (const TPair<FGameplayTag, float>& Param : GetConfig().BaseParameters)
		{
			AddParameter(WorldSettings.Get(), Param.Key, Param.Value);
		}
	}

	if (Sources[0].IsValid() && Sources[0]->Sound == InTrack)
	{
		// Already playing, no change needed
		OnParamsChanged();
		return true;
	}

	return StartTrackInternal(InTrack, FadeTime);
}

void UWorldMusicSubsystem::SetTracksPaused(bool bPaused)
{
	for (const FWorldMusicSource& Source : Sources)
	{
		Source.SetPause(bPaused);
	}
}

bool UWorldMusicSubsystem::RequestDip(const UObject* Requester) const
{
	return DipRequests->Add(Requester);
}

bool UWorldMusicSubsystem::ReleaseDip(const UObject* Requester) const
{
	return DipRequests->Remove(Requester);
}

bool UWorldMusicSubsystem::AddParameter(const UObject* Source,
	FGameplayTag Tag, float Value, EToroValueModifierType Type)
{
	if (WorldMusicTags::IsValid(Tag))
	{
		if (FWorldMusicParam* FoundParam = Parameters.Find(Tag))
		{
			return (*FoundParam)->Add(Source, {Type, Value});
		}

		FWorldMusicParam& Param = Parameters.Add(Tag);
		Param->OnChanged().AddUObject(this, &UWorldMusicSubsystem::OnParamsChanged);
		return Param->Add(Source, {Type, Value});
	}
	return false;
}

bool UWorldMusicSubsystem::RemoveParameter(const UObject* Source, FGameplayTag Tag)
{
	if (FWorldMusicParam* Param = Parameters.Find(Tag))
	{
		return (*Param)->Remove(Source);
	}
	return false;
}

void UWorldMusicSubsystem::OnParamsChanged()
{
	if (IsSystemValid()
		&& !bPauseParamUpdates
		&& !Sources.IsEmpty()
		&& Sources[0].IsValid())
	{
		Sources[0]->SetBoolParameter(TEXT("Dipped"), !DipRequests->IsEmpty());
		for (const TPair<FGameplayTag, FWorldMusicParam>& Param : Parameters)
		{
			FString ParamName; // Name after the root tag
			FString TagString = Param.Key.ToString();
			if (!TagString.Split(TEXT("."), nullptr, &ParamName))
			{
				ParamName = TagString;
			}

			Sources[0]->SetFloatParameter(*ParamName, Param.Value.GetResult());
		}
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UWorldMusicSubsystem::OnAudioFinished(UAudioComponent* Component)
{
	if (!Sources.IsEmpty() && *Sources[0] == Component)
	{
		Sources[0].Destroy();
		return;
	}

	for (auto It = Sources.CreateIterator(); It; ++It)
	{
		if (It.GetIndex() != 0 && **It == Component)
		{
			It->Destroy();
			It.RemoveCurrentSwap();
			return;
		}
	}
}

bool UWorldMusicSubsystem::IsSystemValid() const
{
	return WorldSettings.IsValid();
}

const FWorldMusicSystemConfig& UWorldMusicSubsystem::GetConfig() const
{
	return WorldSettings->GetWorldMusicConfig();
}

bool UWorldMusicSubsystem::StartTrackInternal(USoundBase* InTrack, float InFadeTime)
{
	if (InFadeTime < 0.0f)
	{
		InFadeTime = 0.0f;
		TORO_LOG_WARNING(
			TEXT("[%s] Music track %s has invalid fade duration %g; using zero seconds."),
			*GetName(), *GetNameSafe(InTrack), InFadeTime
		);
	}

	UAudioComponent* NewSource = UGameplayStatics::CreateSound2D(this, InTrack);
	if (!IsValid(NewSource))
	{
		TORO_LOG_ERROR(
			TEXT("[%s] Cannot start music track %s: audio component creation failed."),
			*GetName(), *GetNameSafe(InTrack)
		);
		return false;
	}

	NewSource->bAutoDestroy = false;
	NewSource->OnAudioFinishedNative.AddUObject(this, &UWorldMusicSubsystem::OnAudioFinished);
	NewSource->SetPaused(true);

	if (Sources.IsEmpty())
	{
		Sources.Emplace();
	}

	const FWorldMusicSource OutgoingSource = Sources[0];
	Sources[0] = FWorldMusicSource(NewSource, InFadeTime);
	if (OutgoingSource.IsValid())
	{
		Sources.Add(OutgoingSource);
		OutgoingSource.Stop();
	}

	Sources[0].Start();
	OnParamsChanged();

	return true;
}

void UWorldMusicSubsystem::CleanupSources()
{
	if (Sources.IsEmpty())
	{
		Sources.Emplace();
		return;
	}

	for (auto It = Sources.CreateIterator(); It; ++It)
	{
		if (!It->IsValid() || (*It)->GetPlayState() == EAudioComponentPlayState::Stopped)
		{
			It->Destroy();
			if (It.GetIndex() != 0)
			{
				It.RemoveCurrentSwap();
			}
		}
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

	TGuardValue ParamsGuard(bPauseParamUpdates, true);
	for (const TPair<FGameplayTag, float>& Param : GetConfig().BaseParameters)
	{
		AddParameter(WorldSettings.Get(), Param.Key, Param.Value);
	}
}

void UWorldMusicSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (IsSystemValid())
	{
		if (UMetaSoundSource* Track = GetConfig().DefaultTrack.LoadSynchronous())
		{
			StartTrackInternal(Track, GetConfig().DefaultFadeTime);
		}
	}
}

void UWorldMusicSubsystem::Deinitialize()
{
	WorldSettings.Reset();

	Parameters.Empty();
	DipRequests->OnChanged().Clear();

	for (auto It = Sources.CreateIterator(); It; ++It)
	{
		It->Destroy();
		It.RemoveCurrentSwap();
	}

	Super::Deinitialize();
}
