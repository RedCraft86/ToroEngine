// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameplayTagContainer.h"
#include "DataTypes/MiscEnums.h"
#include "DataTypes/RequestSet.h"
#include "DataTypes/Accumulators.h"
#include "Components/AudioComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "WorldMusicSubsystem.generated.h"

class USoundBase;
class UAudioComponent;
class AToroWorldSettings;
struct FWorldMusicSystemConfig;

/**
 * Combines one contribution per weak UObject source on the game thread.
 * Contributions replace earlier values from the same source and are pruned after source garbage collection.
 */
class FWorldMusicParam final
{
	TSharedRef<TUnorderedAccumulator<TPair<EToroValueModifierType, float>>> Entries;

public:

	FWorldMusicParam()
		: Entries(TUnorderedAccumulator<TPair<EToroValueModifierType, float>>::Make())
	{}

	FWorldMusicParam(const FWorldMusicParam&) = delete;
	FWorldMusicParam& operator=(const FWorldMusicParam&) = delete;

	[[nodiscard]] FORCEINLINE auto& operator->() { return Entries; }
	[[nodiscard]] float GetResult() const;
};

/**
 * Retains a music audio component and its fade duration until explicit destruction.
 * A default instance is an empty slot; copies refer to the same component.
 */
USTRUCT(BlueprintInternalUseOnly)
struct FWorldMusicSource final
{
	GENERATED_BODY()

private:

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> Source;

	float FadeTime = -1.0f;

public:

	FWorldMusicSource() = default;

	FWorldMusicSource(UAudioComponent* InSource, float InFadeTime)
		: Source(InSource), FadeTime(InFadeTime)
	{}

	[[nodiscard]] FORCEINLINE UAudioComponent* operator*() { return Source; }
	[[nodiscard]] FORCEINLINE UAudioComponent* operator->() { return Source; }
	[[nodiscard]] FORCEINLINE bool IsInitialized() const { return FadeTime >= 0.0f; }
	[[nodiscard]] bool IsValid() const;

	void Stop() const;
	void Start() const;
	void SetPause(bool bInPaused) const;
	void Destroy();
};

/**
 * Manages world music playback, outgoing fades, and source-owned parameter contributions in game and PIE worlds.
 * Requires AToroWorldSettings and game-thread access. Requesters are weakly referenced; remove their requests
 * explicitly when effects end before their lifetime does. Invalid requesters are pruned after garbage collection.
 * Only the current track receives parameter updates; outgoing tracks retain their last values.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class UWorldMusicSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

	friend class FWorldMusicParam;

public:

	UWorldMusicSubsystem();

	/** Returns the context world's music subsystem, or nullptr if the world or subsystem is unavailable. */
	TOROSYSTEMS_API static UWorldMusicSubsystem* Get(const UObject* Context);

	/**
	 * Selects the current track, allowing the outgoing track to fade using its own stored duration.
	 * Requesting the current track updates parameters without restarting playback.
	 * @param InTrack Sound to play; must be valid.
	 * @param FadeTime New track's fade-in duration in seconds, also used when it later fades out; negative values use zero.
	 * @param bResetParams Clear contributions and dip requests, then restore configured base contributions, even for the same track.
	 * @return True if the track is already current or a new audio component was created; false if settings, sound, or creation is invalid.
	 */
	UFUNCTION(BlueprintCallable, Category = WorldMusic)
	TOROSYSTEMS_API bool ChangeTrack(USoundBase* InTrack, float FadeTime, bool bResetParams);

	/**
	 * Changes pause state for all currently managed tracks, including outgoing fades.
	 * Does not persist a pause preference for subsequently created tracks.
	 * @param bPaused Whether to pause playback.
	 */
	UFUNCTION(BlueprintCallable, Category = WorldMusic)
	TOROSYSTEMS_API void SetTracksPaused(bool bPaused);

	/**
	 * Requests dipping through the current track's Dipped boolean parameter while any requests remain.
	 * @param Requester Object identifying a single, non-stacking request; not retained.
	 * @return True if added; false for invalid or duplicate requesters.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = WorldMusic, meta = (DefaultToSelf = Requester))
	TOROSYSTEMS_API bool RequestDip(const UObject* Requester) const;

	/**
	 * Releases one object's dip request; other outstanding requests keep dipping enabled.
	 * @param Requester Object whose request should end.
	 * @return True if removed; false for invalid requesters or absent requests.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = WorldMusic, meta = (DefaultToSelf = Requester))
	TOROSYSTEMS_API bool ReleaseDip(const UObject* Requester) const;

	/**
	 * Adds or replaces one source's contribution to a parameter, notifying the current track immediately.
	 * The result is the sum of additive values times the product of multipliers; without additive values it is zero.
	 * @param Source Contribution owner, weakly referenced; must be valid.
	 * @param Tag Descendant of WorldMusic, excluding the root. The root prefix is removed for the audio parameter name.
	 * @param Value Additive contribution or multiplicative factor.
	 * @param Type How this contribution participates in aggregation.
	 * @return True if accepted, including replacement; false for an invalid source or unsupported tag.
	 */
	UFUNCTION(BlueprintCallable, Category = WorldMusic, meta = (DefaultToSelf = Source))
	TOROSYSTEMS_API bool AddParameter(const UObject* Source,
		UPARAM(meta = (Categories = "WorldMusic")) FGameplayTag Tag,
		float Value, EToroValueModifierType Type = EToroValueModifierType::Add);

	/**
	 * Removes a source's contribution and updates the current track with the remaining aggregate.
	 * Empty parameter entries remain registered and evaluate to zero.
	 * @param Source Object whose contribution should end.
	 * @param Tag Parameter identifying the contribution to remove.
	 * @return True if removed; false if the source is invalid or its contribution does not exist.
	 */
	UFUNCTION(BlueprintCallable, Category = WorldMusic, meta = (DefaultToSelf = Source))
	TOROSYSTEMS_API bool RemoveParameter(const UObject* Source,
		UPARAM(meta = (Categories = "WorldMusic")) FGameplayTag Tag);

private:

	/** Slot zero is reserved for the current track; remaining slots hold outgoing tracks. */
	UPROPERTY(Transient)
	TArray<FWorldMusicSource> Sources;

	bool bPauseParamUpdates;
	TWeakObjectPtr<AToroWorldSettings> WorldSettings;
	TMap<FGameplayTag, FWorldMusicParam> Parameters;
	TSharedRef<FRequestSet> DipRequests = FRequestSet::Make();

	void OnParamsChanged();
	void OnAudioFinished(UAudioComponent* Component);

	[[nodiscard]] bool IsSystemValid() const;
	[[nodiscard]] const FWorldMusicSystemConfig& GetConfig() const;

	/**
	 * Installs a new current source, retires its predecessor through the configured fade, then starts playback.
	 * @param InTrack Valid sound to play.
	 * @param InFadeTime Fade duration in seconds; negative values are clamped to zero.
	 * @return True if an audio component was created; does not guarantee audible playback.
	 */
	bool StartTrackInternal(USoundBase* InTrack, float InFadeTime);

	/** Destroys stopped or unusable sources and removes outgoing slots while reserving slot zero. */
	void CleanupSources();

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
};
