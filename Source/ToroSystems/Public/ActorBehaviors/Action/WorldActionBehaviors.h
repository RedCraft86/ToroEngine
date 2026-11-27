// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "AsyncMessageId.h"
#include "UObject/SoftObjectPtr.h"
#include "StructUtils/InstancedStruct.h"
#include "WorldPartition/DataLayer/DataLayerInstance.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "WorldActionBehaviors.generated.h"

class ALevelSequenceActor;
class UDataLayerAsset;

/**
 * Requests runtime states for World Partition Data Layers when triggered.
 */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Misc] Data Layer")
class TOROSYSTEMS_API UDataLayerActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	/** Desired runtime state for each Data Layer asset assigned to this world. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TMap<TObjectPtr<UDataLayerAsset>, EDataLayerRuntimeState> DataLayerStates;

	virtual void OnTriggered() override;
};

/**
 * Triggers a Level Sequence (Cinematic) within the world.
 * Allows adjusting how the sequence is played out.
 */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Misc] Level Sequence")
class TOROSYSTEMS_API ULevelSequenceActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	/**
	 * The specific LevelSequenceActor in the world that should begin playing.
	 */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSoftObjectPtr<ALevelSequenceActor> Sequence;

	/**
	 * The sequence will go reverse from its current playback position.
	 */
	UPROPERTY(EditAnywhere, Category = Settings)
	bool bReverse = false;

	/**
	 * The sequence will rewind back to the start (to the end if reversing) before playing.
	 */
	UPROPERTY(EditAnywhere, Category = Settings)
	bool bRestart = false;

	virtual void OnTriggered() override;
};

/**
 * Calls a 'Remote Event' in the Level Blueprint.
 * This is a way to trigger logic defined strictly within the current map's level blueprint.
 */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Misc] Remote Event")
class TOROSYSTEMS_API URemoteEventActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	/** The case-sensitive name of the Custom Event to trigger in the Level Blueprint. */
	UPROPERTY(EditAnywhere, Category = Settings)
	FName EventName;

	virtual void OnTriggered() override;
};

/**
 * Broadcasts a message via the AsyncMessageSystem to all bound listeners.
 * Used for triggering events like UI updates, achievement unlocks, or world state changes.
 */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Misc] Async Message")
class TOROSYSTEMS_API UAsyncMessageActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	/**
	 * The unique identifier/channel for this message.
	 */
	UPROPERTY(EditAnywhere, Category = Settings)
	FAsyncMessageId MessageId;

	/**
	 * Flexible data payload.
	 * In the Editor, you can select any USTRUCT type to send with this message.
	 */
	UPROPERTY(EditAnywhere, Category = Settings)
	FInstancedStruct Payload;

	virtual void OnTriggered() override;
};
