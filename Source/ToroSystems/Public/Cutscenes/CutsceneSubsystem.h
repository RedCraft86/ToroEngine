// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UE5Coro.h"
#include "CutsceneTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "CutsceneSubsystem.generated.h"

class AToroPlayerHUD;
class ACutsceneActor;
class ULevelSequencePlayer;

UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class UCutsceneSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	ECutscenePlayResult StartCutscene(ACutsceneActor* Actor);
	void CutsceneDestroyed(const ACutsceneActor* Actor);
	void StopActiveCutscene();

	void SkipActiveCutscene();
	[[nodiscard]] bool CanSkipCutscene(float RemainingHoldTime) const;

	[[nodiscard]] ACutsceneActor* GetCutscene() const;
	[[nodiscard]] ULevelSequencePlayer* GetSequence() const;
	[[nodiscard]] bool IsActive() const;

private:

	TWeakObjectPtr<const AToroPlayerHUD> HUD;
	TWeakObjectPtr<ACutsceneActor> Cutscene;
	bool bSkipFadeInProgress = false;

	UE5Coro::TCoroutine<> FadeAndSkip(TWeakObjectPtr<ACutsceneActor> Actor, float FadeDuration);

	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
};
