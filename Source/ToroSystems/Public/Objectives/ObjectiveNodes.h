// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "AsyncMessageId.h"
#include "GameFlags/GameFlagManager.h"
#include "SMStateInstance.h"
#include "GameplayTagContainer.h"
#include "SMTransitionInstance.h"
#include "Framework/ToroPlayerCharacter.h"
#include "StructUtils/InstancedStruct.h"
#include "ObjectiveNodes.generated.h"

class ATeleportPoint;
class UDataLayerAsset;

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FObjectiveCountTaskInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = Task, meta = (ClampMin = 1))
	uint8 Amount = 1;

	UPROPERTY(EditAnywhere, Category = Task, meta = (MultiLine = false))
	FText Label;
};

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FObjectiveFlagTaskInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = Flag)
	EGameFlagScope Scope = EGameFlagScope::SlotSave;

	UPROPERTY(EditAnywhere, Category = Task, meta = (MultiLine = false))
	FText Label;
};

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FObjectiveFlagInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = Flag)
	EGameFlagScope Scope = EGameFlagScope::SlotSave;

	UPROPERTY(EditAnywhere, Category = Flag)
	bool bState = true;
};

UCLASS(NotBlueprintable, BlueprintType, PrioritizeCategories = (Objective))
class TOROSYSTEMS_API UObjectiveNode final : public USMStateInstance
{
	GENERATED_BODY()

public:

	UObjectiveNode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Objective)
	bool bSavable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Objective)
	FText DisplayTitle = INVTEXT("Objectives");

	UPROPERTY(EditAnywhere, Category = Objective, meta = (ForceInlineRow = true, Categories = "Objective"))
	TMap<FGameplayTag, FObjectiveCountTaskInfo> CountTasks;

	UPROPERTY(EditAnywhere, Category = Objective, meta = (ForceInlineRow = true))
	TMap<FGameplayTag, FObjectiveFlagTaskInfo> FlagTasks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Objective, EditFixedSize)
	TArray<FGameplayTag> TaskDisplayOrder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|GamePhase",
		meta = (Bitmask, BitmaskEnum = "/Script/ToroSystems.EPlayerCapability"))
	int32 PlayerRestrictions = static_cast<int32>(EPlayerCapability::None);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|GamePhase")
	TSoftObjectPtr<ATeleportPoint> Teleporter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|GamePhase")
	bool bContinuous = true;

	UPROPERTY(EditAnywhere, Category = "Objective|GamePhase", meta = (ForceInlineRow = true))
	TMap<TSoftObjectPtr<UDataLayerAsset>, bool> DataLayers;

private:

	UPROPERTY(EditDefaultsOnly, Category = "Objective|Events", meta = (ForceInlineRow = true))
	TMap<FGameplayTag, FObjectiveFlagInfo> Flags;

	UPROPERTY(EditDefaultsOnly, Category = "Objective|Events", meta = (ForceInlineRow = true))
	TMap<FAsyncMessageId, FInstancedStruct> Events;

	virtual void OnStateBegin_Implementation() override;
#if WITH_EDITOR
	void SyncTaskDisplayOrder();

	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void OnPreCompileValidate_Implementation(USMCompilerLog* CompilerLog) const override;
#endif
};

UCLASS(NotBlueprintable, BlueprintType, PrioritizeCategories = (Objective))
class TOROSYSTEMS_API UObjectiveTransition final : public USMTransitionInstance
{
	GENERATED_BODY()

public:

	UObjectiveTransition();

private:

	virtual bool CanEnterTransition_Implementation() const override;
};
