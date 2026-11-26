// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "AsyncMessageId.h"
#include "GameFlagManager.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"
#include "StructUtils/InstancedStruct.h"
#include "WorldPartition/DataLayer/DataLayerInstance.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "WorldActionBehaviors.generated.h"

class ALevelSequenceActor;
class UDataLayerAsset;
class UDialogueGraph;

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

/** Starts a dialogue for the local player when triggered. */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Misc] Start Dialogue")
class TOROSYSTEMS_API UDialogueActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, Category = Settings, meta = (AllowAbstract = false))
	TSubclassOf<UDialogueGraph> Dialogue;

	virtual bool CanTrigger() const override;
	virtual void OnTriggered() override;
};

/** Adds progress to an objective count task. */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Misc] Advance Count Task")
class TOROSYSTEMS_API UObjectiveTaskActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, Category = Settings, meta = (Categories = "Objective"))
	FGameplayTag Task;

	/** Progress added per trigger for a count task. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (ClampMin = 1, UIMin = 1))
	uint8 Amount = 1;

	virtual bool CanTrigger() const override;
	virtual void OnTriggered() override;
};

/** Sets a game flag in its configured scope. */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Misc] Set Game Flag")
class TOROSYSTEMS_API UGameFlagActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, Category = Settings)
	FGameplayTag Flag;

	UPROPERTY(EditAnywhere, Category = Settings)
	EGameFlagScope Scope = EGameFlagScope::SlotSave;

	virtual bool CanTrigger() const override;
	virtual void OnTriggered() override;
};
