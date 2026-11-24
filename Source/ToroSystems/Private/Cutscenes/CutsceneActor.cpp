// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Cutscenes/CutsceneActor.h"
#include "Cutscenes/CutsceneSubsystem.h"
#include "LevelSequencePlayer.h"

ACutsceneActor::ACutsceneActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlaybackSettings.bHidePlayer = true;
	PlaybackSettings.bDisableLookAtInput = true;
	PlaybackSettings.bDisableMovementInput = true;
}

ECutscenePlayResult ACutsceneActor::PlayCutscene()
{
	if (Subsystem.IsValid())
	{
		const ECutscenePlayResult Result = Subsystem->StartCutscene(this);
		if (Result == ECutscenePlayResult::Success)
		{
			GetSequencePlayer()->Play();
			if (UGameFlagManager* FlagManager = UGameFlagManager::Get(this))
			{
				FlagManager->SetGameFlag(CutsceneTag, RememberScope);
			}
			else
			{
				FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok,
					INVTEXT(
						"Seen Cutscene data was not saved properly, it is recommended that you "
						"restart the game before continuing to avoid any more data losses."
					), INVTEXT("Cutscene Save Error")
				);
			}
		}
		return Result;
	}

	return ECutscenePlayResult::InvalidContext;
}

void ACutsceneActor::SetTrackSubtitle(const FText& Text)
{
	if (!Subsystem.IsValid() || Subsystem->GetCutscene() != this)
	{
		ActiveSubtitle = FText::GetEmpty();
		return;
	}

	if (!ActiveSubtitle.EqualTo(Text))
	{
		ActiveSubtitle = Text;
		OnSubtitleEvent.ExecuteIfBound(ActiveSubtitle);
	}
}

void ACutsceneActor::OnSequenceStopped()
{
	OnStoppedEvent.Broadcast(this);
	ActiveSubtitle = FText::GetEmpty();
	OnSubtitleEvent.ExecuteIfBound(ActiveSubtitle);

	OnStoppedEvent.Clear();
	OnSubtitleEvent.Unbind();
}

void ACutsceneActor::BeginPlay()
{
	Super::BeginPlay();
	Subsystem = GetWorld()->GetSubsystem<UCutsceneSubsystem>();
	GetSequencePlayer()->OnStop.AddDynamic(this, &ACutsceneActor::OnSequenceStopped);
}

void ACutsceneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetSequencePlayer()->OnStop.RemoveAll(this);
	if (Subsystem.IsValid())
	{
		Subsystem->CutsceneDestroyed(this);
	}

	OnStoppedEvent.Clear();
	OnSubtitleEvent.Unbind();
	Super::EndPlay(EndPlayReason);
}
