// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SequencerSectionBP.h"
#include "SequencerTrackBP.h"
#include "SequencerTrackInstanceBP.h"
#include "CutsceneSubtitleTrack.generated.h"

UCLASS(NotBlueprintable, DisplayName = "Cutscene Subtitle")
class TOROSYSTEMS_API UCutsceneSubtitleSection final : public USequencerSectionBP
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = Subtitle, meta = (MultiLine = true))
	FText Subtitle;
};

UCLASS(NotBlueprintable)
class TOROSYSTEMS_API UCutsceneSubtitleTrackInstance final : public USequencerTrackInstanceBP
{
	GENERATED_BODY()

	virtual void OnEndUpdateInputs() override;
	virtual void OnDestroyed() override;

	void ClearSubtitle();
	bool bHasPublishedCue = false;
};

UCLASS(NotBlueprintable, DisplayName = "Cutscene Subtitles")
class TOROSYSTEMS_API UCutsceneSubtitleTrack final : public USequencerTrackBP
{
	GENERATED_BODY()

public:

	UCutsceneSubtitleTrack();

#if WITH_EDITORONLY_DATA
	virtual FText GetDefaultDisplayName() const override;
#endif
};
