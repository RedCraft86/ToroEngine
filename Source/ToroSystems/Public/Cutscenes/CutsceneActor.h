// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CutsceneTypes.h"
#include "GameFlagManager.h"
#include "LevelSequenceActor.h"
#include "CutsceneActor.generated.h"

class UCutsceneSubsystem;
class UMovieSceneSequencePlayer;

UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class ACutsceneActor final : public ALevelSequenceActor
{
	GENERATED_BODY()

public:

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnStopped, const ACutsceneActor*)
	DECLARE_DELEGATE_OneParam(FOnSubtitle, const FText&)

	ACutsceneActor(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = Cutscene)
	ECutscenePlayResult PlayCutscene();

	UFUNCTION(BlueprintCallable, Category = Cutscene)
	void SetTrackSubtitle(const FText& Text);

	const FText& GetActiveSubtitle() const { return ActiveSubtitle; }

	float GetSkipFadeDuration() const { return SkipFadeDuration; }

	[[nodiscard]] FORCEINLINE FOnStopped::RegistrationType& OnStopped()
	{
		return OnStoppedEvent;
	}

	[[nodiscard]] FORCEINLINE FOnSubtitle::RegistrationType& OnSubtitle()
	{
		return OnSubtitleEvent;
	}

private:

	UPROPERTY(EditAnywhere, Category = "Cutscene", meta = (ClampMin = 0.0f, Units = "s"))
	float SkipFadeDuration = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Cutscene|Memory", meta = (Categories = "Cutscene"))
	FGameplayTag CutsceneTag;

	UPROPERTY(EditAnywhere, Category = "Cutscene|Memory")
	EGameFlagScope RememberScope = EGameFlagScope::Transient;

	FText ActiveSubtitle;
	FOnStopped OnStoppedEvent;
	FOnSubtitle OnSubtitleEvent;
	TWeakObjectPtr<UCutsceneSubsystem> Subsystem;

	UFUNCTION()
	void OnSequenceStopped();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
