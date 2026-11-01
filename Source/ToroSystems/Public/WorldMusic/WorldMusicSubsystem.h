// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DataTypes/AccumulatorSet.h"
#include "DataTypes/AccumulatorMap.h"
#include "Subsystems/WorldSubsystem.h"
#include "WorldMusicSubsystem.generated.h"

class USoundBase;
class UAudioComponent;
class AToroWorldSettings;
struct FWorldMusicSystemConfig;

/** Determines whether a modifier contributes to the additive sum or multiplier product. */
UENUM(BlueprintType)
enum class EWorldMusicModType : uint8
{
	/** Adds to the scalar's sum; zero is neutral. */
	Additive,

	/** Scales the additive sum; one is neutral. */
	Multiplier
};

/**
 * Pairs a music component with the fade duration retained for both starting and stopping it.
 * Playback operations require a live component and a finite, nonnegative fade duration.
 */
USTRUCT(BlueprintInternalUseOnly)
struct FWorldMusicSourceRef final
{
	GENERATED_BODY()

	/** Component retained while this track is playing or fading out. */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> Source;

	/** Fade duration in seconds; zero or nearly zero switches playback immediately. */
	UPROPERTY(Transient)
	float FadeTime;

	FWorldMusicSourceRef()
		: Source(nullptr)
		, FadeTime(0.0f)
	{}

	FWorldMusicSourceRef(UAudioComponent* InSource, float InFadeTime)
		: Source(InSource)
		, FadeTime(InFadeTime)
	{}

	/** Returns whether the component is live and its fade duration is finite and nonnegative. */
	[[nodiscard]] bool IsValid() const;

	/** Unpauses and starts playback with the stored fade duration if this source is valid. */
	void Start() const;

	/** Stops playback with the stored fade duration if this source is valid. */
	void Stop() const;

	/** Pauses or resumes playback if this source is valid. */
	void SetPaused(bool bPaused) const;

	/** Immediately stops and destroys a live component, regardless of its fade duration. */
	void Destroy();
};

/**
 * Manages nonspatial music and configurable per-track crossfades in game and PIE worlds.
 * Reads baseline configuration and loads the default track during initialization, then attempts playback at world BeginPlay.
 * Weak requester identities control dipping, pressure, and tension; stale requests are pruned after garbage collection
 * and during successful accumulator mutations.
 * MetaSounds can consume Dipped, Pressure, Tension, and MoodDelta inputs. Call this system on the game thread.
 * MoodDelta is the change in pressure plus tension since the previous parameter update: positive for increases,
 * negative for decreases, and zero when unchanged. Outgoing tracks receive zero pressure, tension, and MoodDelta.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class UWorldMusicSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	UWorldMusicSubsystem();

	/** Returns the context world's music subsystem, or nullptr if the world or subsystem is unavailable. */
	TOROSYSTEMS_API static UWorldMusicSubsystem* Get(const UObject* Context);

	/**
	 * Crossfades to a different track, or restarts a stopped track; failures are logged.
	 * Requesting the current active track, including paused playback, succeeds without changes or parameter resets.
	 * If component creation succeeds, the outgoing track fades out using the duration stored when it started.
	 * @param InTrack Sound to play. Parameter-driven behavior requires compatible MetaSound inputs.
	 * @param FadeTime Incoming fade duration in seconds, also retained for its eventual fade-out.
	 *        When starting a track, negative or non-finite values log a warning and become zero;
	 *        zero or nearly zero durations switch immediately.
	 * @param bResetParams Clears dip requests and external modifiers, then restores configured baseline contributions.
	 *        Reset occurs before component creation and remains in effect if creation fails.
	 *        On successful creation, the incoming track receives the reset's mood delta in one parameter update.
	 * @return True if the track is already current or its component was created and playback requested;
	 *         false if the system or track is invalid, or component creation fails. Does not confirm audible playback.
	 */
	UFUNCTION(BlueprintCallable, Category = WorldMusic)
	TOROSYSTEMS_API bool SetCurrentTrack(USoundBase* InTrack, float FadeTime, bool bResetParams);

	/** Pauses or resumes all currently tracked tracks, including outgoing crossfades. */
	UFUNCTION(BlueprintCallable, Category = WorldMusic)
	TOROSYSTEMS_API void SetTracksPaused(bool bPaused);

	/** Requests dipping for a valid source; repeated requests from the same source do not stack. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = WorldMusic)
	TOROSYSTEMS_API void RequestDip(const UObject* Source) const;

	/** Removes the source's dip request; music remains dipped while other requests exist. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = WorldMusic)
	TOROSYSTEMS_API void RemoveDip(const UObject* Source) const;

	/**
	 * Adds or replaces one pressure modifier for a valid source without retaining the source.
	 * @param Source Request identity; invalid sources are ignored.
	 * @param Value Additive contribution or multiplier. Values are not clamped.
	 * @param Type Final pressure is the additive sum multiplied by the multiplier product.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = WorldMusic)
	TOROSYSTEMS_API void AddPressure(const UObject* Source, float Value,
		EWorldMusicModType Type = EWorldMusicModType::Additive) const;

	/** Removes the source's pressure modifier; an absent source is harmless. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = WorldMusic)
	TOROSYSTEMS_API void RemovePressure(const UObject* Source) const;

	/**
	 * Adds or replaces one tension modifier for a valid source without retaining the source.
	 * @param Source Request identity; invalid sources are ignored.
	 * @param Value Additive contribution or multiplier. Values are not clamped.
	 * @param Type Final tension is the additive sum multiplied by the multiplier product.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = WorldMusic)
	TOROSYSTEMS_API void AddTension(const UObject* Source, float Value,
		EWorldMusicModType Type = EWorldMusicModType::Additive) const;

	/** Removes the source's tension modifier; an absent source is harmless. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = WorldMusic)
	TOROSYSTEMS_API void RemoveTension(const UObject* Source) const;

private:

	UPROPERTY(Transient)
	TArray<FWorldMusicSourceRef> AudioSources;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> DefaultTrack;

	float DefaultFadeTime;
	float LastMoodIntensity;
	TWeakObjectPtr<AToroWorldSettings> WorldSettings;
	TSharedPtr<TAccumulatorSet<TWeakObjectPtr<const UObject>>> DipRequests;
	TSharedPtr<TAccumulatorMap<TWeakObjectPtr<const UObject>, TPair<EWorldMusicModType, float>>> Pressure;
	TSharedPtr<TAccumulatorMap<TWeakObjectPtr<const UObject>, TPair<EWorldMusicModType, float>>> Tension;

	[[nodiscard]] bool IsSystemValid() const;
	[[nodiscard]] const FWorldMusicSystemConfig& GetConfig() const;
	void SetAudioParameters(float FinalPressure, float FinalTension);
	bool StartTrackInternal(USoundBase* InTrack, float InFadeTime);
	void CleanupAudioSources();

	void OnPostGarbageCollect() const;
	void OnAudioFinished(UAudioComponent* Component);
	void OnUpdateParameters();

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
};
