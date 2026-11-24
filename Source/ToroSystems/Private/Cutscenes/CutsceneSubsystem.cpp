// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Cutscenes/CutsceneSubsystem.h"
#include "Cutscenes/CutsceneActor.h"
#include "Cutscenes/ToroCutsceneWidget.h"
#include "UserInterface/UserInterfaceConfig.h"
#include "Libraries/ToroCameraLibrary.h"
#include "Framework/ToroPlayerHUD.h"
#include "Utilities/ToroUtilities.h"
#include "LevelSequencePlayer.h"
#include "Engine/LatentActionManager.h"
#include "Engine/World.h"

namespace
{
	constexpr float SkipWaitTime = 0.1f;

	TSubclassOf<UToroCutsceneWidget> GetCutsceneWidgetClass()
	{
		const FUserInterfaceConfig* Config = FUserInterfaceConfig::Get();
		return Config ? Config->CutsceneWidgetClass.LoadSynchronous() : nullptr;
	}
}

ECutscenePlayResult UCutsceneSubsystem::StartCutscene(ACutsceneActor* Actor)
{
	if (!IsValid(Actor) || !Actor->GetSequencePlayer())
	{
		return ECutscenePlayResult::InvalidCutscene;
	}

	if (bSkipFadeInProgress || GetCutscene() != nullptr)
	{
		return ECutscenePlayResult::CutscenePlaying;
	}

	HUD = AToroPlayerHUD::Get(this);
	if (!HUD.IsValid())
	{
		return ECutscenePlayResult::InvalidContext;
	}

	Cutscene = Actor;
	if (!HUD->AddWidgetByClass(EToroMasterWidgetType::Menu, GetCutsceneWidgetClass()))
	{
		HUD.Reset();
		Cutscene.Reset();
		return ECutscenePlayResult::InvalidContext;
	}

	Cutscene->OnStopped().RemoveAll(this);
	Cutscene->OnStopped().AddUObject(this, &UCutsceneSubsystem::CutsceneDestroyed);
	return ECutscenePlayResult::Success;
}

void UCutsceneSubsystem::CutsceneDestroyed(const ACutsceneActor* Actor)
{
	if (Actor == Cutscene.Get())
	{
		StopActiveCutscene();
	}
}

void UCutsceneSubsystem::StopActiveCutscene()
{
	if (HUD.IsValid())
	{
		HUD->RemoveWidgetByClass(EToroMasterWidgetType::Menu, GetCutsceneWidgetClass());
		HUD.Reset();
	}
	if (Cutscene.IsValid())
	{
		Cutscene->OnStopped().RemoveAll(this);
		if (ULevelSequencePlayer* Player = Cutscene->GetSequencePlayer())
		{
			Player->GoToEndAndStop();
		}
	}
	Cutscene.Reset();
}

bool UCutsceneSubsystem::CanSkipCutscene(float RemainingHoldTime) const
{
	const ACutsceneActor* Actor = GetCutscene();
	const ULevelSequencePlayer* Player = GetSequence();
	if (bSkipFadeInProgress || !Actor || !IsValid(Player) || !Player->IsPlaying())
	{
		return false;
	}

	const float FadeDuration = Actor->GetSkipFadeDuration();
	const float PlayRate = FMath::Abs(Player->GetPlayRate());
	if (!FMath::IsFinite(FadeDuration) || FadeDuration < 0.0f
		|| !FMath::IsFinite(RemainingHoldTime) || RemainingHoldTime < 0.0f
		|| !FMath::IsFinite(PlayRate) || PlayRate <= 0.0f)
	{
		return false;
	}

	const double CurrentTime = Player->GetCurrentTime().AsSeconds();
	const double Remaining = Player->IsReversed()
		? CurrentTime - Player->GetStartTime().AsSeconds()
		: Player->GetEndTime().AsSeconds() - CurrentTime;
	return Remaining >= (2.0 * FadeDuration + RemainingHoldTime + SkipWaitTime) * PlayRate;
}

void UCutsceneSubsystem::SkipActiveCutscene()
{
	if (CanSkipCutscene(0.0f))
	{
		bSkipFadeInProgress = true;
		FadeAndSkip(Cutscene, Cutscene->GetSkipFadeDuration());
	}
}

UE5Coro::TCoroutine<> UCutsceneSubsystem::FadeAndSkip(TWeakObjectPtr<ACutsceneActor> Actor, float FadeDuration)
{
	bool bFadedOut = false;
	co_await UToroCameraLibrary::StartCameraFade(
		ToroEngine::LatentInfo(this), this, bFadedOut,
		FLinearColor::Black, FadeDuration, 0.0f, 1.0f, false, true
	);

	if (Actor.IsValid() && Actor == Cutscene)
	{
		if (bFadedOut)
		{
			UToroCameraLibrary::SetCameraFade(this, FLinearColor::Black, 1.0f, false);
		}
		StopActiveCutscene();
	}

	if (bFadedOut)
	{
		bool bFadedIn = false;
		co_await UE5Coro::Latent::Seconds(SkipWaitTime);
		co_await UToroCameraLibrary::StartCameraFade(
			ToroEngine::LatentInfo(this), this, bFadedIn,
			FLinearColor::Black, FadeDuration, 1.0f, 0.0f, false, false
		);
		if (!bFadedIn)
		{
			UToroCameraLibrary::StopCameraFade(this);
		}
	}

	bSkipFadeInProgress = false;
}

void UCutsceneSubsystem::Deinitialize()
{
	if (bSkipFadeInProgress)
	{
		GetWorld()->GetLatentActionManager().RemoveActionsForObject(this);
		UToroCameraLibrary::StopCameraFade(this);
		bSkipFadeInProgress = false;
	}
	Super::Deinitialize();
}

ACutsceneActor* UCutsceneSubsystem::GetCutscene() const
{
	return Cutscene.Get();
}

ULevelSequencePlayer* UCutsceneSubsystem::GetSequence() const
{
	return Cutscene.IsValid() ? Cutscene->GetSequencePlayer() : nullptr;
}

bool UCutsceneSubsystem::IsActive() const
{
	const ULevelSequencePlayer* Player = GetSequence();
	return IsValid(Player) && Player->IsPlaying();
}

bool UCutsceneSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
