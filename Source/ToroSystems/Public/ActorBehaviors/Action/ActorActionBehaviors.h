// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UObject/SoftObjectPtr.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "ActorActionBehaviors.generated.h"

class AActor;
class AStaticMeshActor;

/** Sets or toggles the 'Hidden in Game' state for a collection of actors. */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Actor] Change Visibility")
class TOROSYSTEMS_API UVisibilityActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	/** Whether to set or toggle visibility. */
	UPROPERTY(EditAnywhere, Category = Settings)
	EToroActionApplyType Type = EToroActionApplyType::Set;

	/** The visibility state to apply (only used when Type is 'Set'). */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (EditCondition = "Type == EToroActionApplyType::Set", EditConditionHides))
	bool bVisible = false;

	/** The actors whose visibility will be modified. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSet<TSoftObjectPtr<AActor>> Targets;

	virtual void OnTriggered() override;
};

/** Enables or disables collision for a collection of actors. */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Actor] Enable Collision")
class TOROSYSTEMS_API UCollisionActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	/** Whether to set or toggle collision. */
	UPROPERTY(EditAnywhere, Category = Settings)
	EToroActionApplyType Type = EToroActionApplyType::Set;

	/** The collision state to apply (only used when Type is 'Set'). */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (EditCondition = "Type == EToroActionApplyType::Set", EditConditionHides))
	bool bCollisionEnabled = false;

	/** The actors whose collision state will be modified. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSet<TSoftObjectPtr<AActor>> Targets;

	virtual void OnTriggered() override;
};

/** Sets or toggles activation for actors implementing IActivatableObject. */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Actor] Enabled State")
class TOROSYSTEMS_API UActorEnabledActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	/** Whether to set or toggle activation. */
	UPROPERTY(EditAnywhere, Category = Settings)
	EToroActionApplyType Type = EToroActionApplyType::Set;

	/** The enabled state to apply (only used when Type is 'Set'). */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (EditCondition = "Type == EToroActionApplyType::Set", EditConditionHides))
	bool bEnabled = false;

	/** Target actors. Restricted to those that implement the EnabledState interface. */
	UPROPERTY(EditAnywhere, Category = Settings, meta = (MustImplement = "/Script/ToroEngine.ActivatableObject"))
	TSet<TSoftObjectPtr<AActor>> Targets;

	virtual void OnTriggered() override;
};

/** Adds or removes actor tags on a collection of actors. */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Actor] Apply Tags")
class TOROSYSTEMS_API UActorTagsActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	/** Tags to append to the targets. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSet<FName> AddTags;

	/** Tags to strip from the targets. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSet<FName> RemoveTags;

	/** The actors whose tags array will be modified. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSet<TSoftObjectPtr<AActor>> Targets;

	virtual void OnTriggered() override;
};

/**
 * Updates 'Custom Primitive Data' on Static Mesh Actors.
 * This is an extremely efficient way to update material parameters without creating dynamic material instances.
 */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Actor] Primitive Data")
class TOROSYSTEMS_API UPrimitiveDataActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	/** Whether to also change the component's default primitive data. */
	UPROPERTY(EditAnywhere, Category = Settings)
	bool bOverrideDefault = true;

	/**
	 * Map of Primitive Data Indices and their target values.
	 * Key: The index of the primitive data float (0-31).
	 * Value: The new float value for the material.
	 */
	UPROPERTY(EditAnywhere, Category = Settings)
	TMap<uint8, float> PrimitiveData;

	/** The actors whose primitive data will be modified. */
	UPROPERTY(EditAnywhere, Category = Settings)
	TSet<TSoftObjectPtr<AStaticMeshActor>> Targets;

	virtual void OnTriggered() override;
};
