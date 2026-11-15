// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Containers/Queue.h"
#include "GameplayTagContainer.h"
#include "UserInterface/Bases/ToroAutoWidget.h"
#include "ToroSubtitleWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;
class UToroDatabaseSubsystem;

/** One gameplay subtitle segment with optional additional reading time. */
USTRUCT(BlueprintType)
struct FToroSubtitleEntry final
{
	GENERATED_BODY()

	/** Spoken text; empty or whitespace-only lines are rejected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Subtitle)
	FText Line = FText::GetEmpty();

	/** Additional seconds added to the reading estimate before applying the configured minimum duration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Subtitle)
	float ExtraTime = 0.0f;

	/** Whether this entry contains non-whitespace text. */
	[[nodiscard]] FORCEINLINE bool IsValid() const
	{
		return !Line.IsEmptyOrWhitespace();
	}
};

/**
 * Presents incidental gameplay speech one line at a time through the HUD's configured auto-widget.
 * Lines from the active speaker queue in order; another speaker interrupts and discards pending lines.
 * Each line fades in and uses the shared UI text timing settings. Speaker names come from the character
 * database, falling back to a readable version of the speaker tag's leaf name.
 * Blueprint subclasses must supply SpeakerBlock, LineBlock, and TextAnimation; the animation plays
 * forward for each line and in reverse when the queue finishes.
 */
UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType)
class UToroSubtitleWidget final : public UToroAutoWidget
{
	GENERATED_BODY()

public:

	UToroSubtitleWidget();

	/**
	 * Submits a line for a speaker identified by a tag beneath Character.
	 * A line matching the currently displayed text is accepted without being queued again.
	 * @return True if accepted; false for invalid input or an unavailable HUD, subtitle widget, or world.
	 */
	UFUNCTION(BlueprintCallable, Category = Subtitles, meta = (WorldContext = ContextObject))
	static TOROSYSTEMS_API bool EnqueueSubtitle(const UObject* ContextObject,
		UPARAM(meta = (Categories = "Character")) FGameplayTag Speaker,
		const FToroSubtitleEntry& Entry);

	/**
	 * Submits an ordered sequence for one speaker using the same rules as EnqueueSubtitle.
	 * Invalid entries are skipped without rolling back accepted entries.
	 * @return True if every entry was accepted; false for an empty sequence or any failed submission.
	 */
	UFUNCTION(BlueprintCallable, Category = Subtitles, meta = (WorldContext = ContextObject))
	static TOROSYSTEMS_API bool EnqueueSubtitles(const UObject* ContextObject,
		UPARAM(meta = (Categories = "Character")) FGameplayTag Speaker,
		const TArray<FToroSubtitleEntry>& Entries);

private:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeakerBlock;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> LineBlock;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> TextAnimation;

	FTimerHandle LineTimer;
	FGameplayTag ActiveSpeaker;
	TQueue<FToroSubtitleEntry> LineQueue;
	TWeakObjectPtr<UToroDatabaseSubsystem> Database;

	void ShowNextLine();
	void FinishShowingLine();
	void UpdateTextBlocks(const FToroSubtitleEntry& Entry) const;
	bool Enqueue(const FGameplayTag& Speaker, const FToroSubtitleEntry& Entry);
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
