// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Templates/SubclassOf.h"
#include "Components/ToroActorComponent.h"
#include "ToroBehaviorComponent.generated.h"

class AActor;

/**
 * Base behavior component with automatic sibling-reference binding and editor duplicate validation.
 * Registration binds direct reflected FToroBehaviorRef properties, including inherited ones,
 * to the owning actor before the parent construction hook runs. Unregistration clears
 * their owner bindings and cached components. Containers and nested structs are not traversed.
 * Editor validation rejects other components matching DuplicateCheckClass(), including subclasses.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent, ChildCanTick = true))
class TOROSYSTEMS_API UToroBehaviorComponent : public UToroActorComponent
{
	GENERATED_BODY()

public:

	UToroBehaviorComponent();

protected:

	virtual void OnRegister() override;
	virtual void OnUnregister() override;

#if WITH_EDITOR
	/**
	 * Selects the component class used to enforce editor-only uniqueness on the owning actor.
	 * Defaults to this component's class. Overrides should return that class or an ancestor;
	 * returning a shared ancestor prevents sibling subclasses from coexisting on an actor.
	 * A null or invalid class skips the duplicate check while preserving superclass validation.
	 */
	virtual TSubclassOf<UToroBehaviorComponent> DuplicateCheckClass() const;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:

	/**
	 * Rebinds direct reflected FToroBehaviorRef properties, including inherited properties,
	 * and clears their component caches without resolving any references.
	 * Only element zero of each property is visited; containers and nested structs are not traversed.
	 * @param Owner Actor to bind, or nullptr to clear the owner bindings.
	 */
	void SetBehaviorRefTypeOwners(const AActor* Owner);
};
