// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Engine/StaticMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "InstancedStaticMeshActor.generated.h"

/**
 * Static mesh actor whose default mesh component supports instancing.
 * Editor construction rebuilds component instances from the authored local transforms,
 * restoring an identity entry if the array is empty. In game worlds, component
 * initialization adds one local identity instance only if the component has no instances.
 * Serialized instances and instances added before initialization completes are preserved.
 */
UCLASS(meta = (ChildCanTick = true))
class TOROENGINE_API AInstancedStaticMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:

	/**
	 * Replaces the inherited mesh component with an ISM component and disables native ticking.
	 * Uses engine cube and basic material defaults outside commandlets and dedicated servers.
	 * @param ObjectInit Initializer carrying default-subobject overrides, including derived HISM types.
	 */
	AInstancedStaticMeshActor(const FObjectInitializer& ObjectInit);

#if WITH_EDITORONLY_DATA
	/**
	 * Editor-authored component-local transforms used to rebuild instances during construction.
	 * This array is editor-only; cooked component instances retain their own serialized data.
	 */
	UPROPERTY(Category = StaticMesh, EditAnywhere, meta = (MakeEditWidget, DisplayPriority = 1))
	TArray<FTransform> Instances = {FTransform::Identity};
#endif

	/**
	 * Casts the inherited mesh component to the requested type without creating a component.
	 * @tparam T Requested mesh component type; defaults to UInstancedStaticMeshComponent.
	 * @return The component as T, or null when absent or incompatible with the requested type.
	 */
	template <typename T = UInstancedStaticMeshComponent>
	T* GetMeshComponent() const
	{
		return Cast<T>(GetStaticMeshComponent());
	}

#if WITH_EDITOR

	/**
	 * Clears component instances and rebuilds them from the editor array in local space.
	 * Does nothing while instance synchronization is locked or the component is unavailable.
	 * This replaces component-only instance data; it does not insert a default for an empty array.
	 */
	void UpdateInstances() const;

	/**
	 * Replaces the editor array with successfully read component-local instance transforms.
	 * Temporarily locks reconstruction synchronization while copying. Leaves the array
	 * unchanged when the component is unavailable.
	 */
	void CopyInstancesFromComponent();
#endif

protected:

	virtual void PostInitializeComponents() override;

#if WITH_EDITOR
	/** Temporarily suppresses reconstruction updates while copying component instance transforms. */
	bool bLockInstances = false;

	virtual void OnConstruction(const FTransform& Transform) override;
#endif
};

/**
 * Instanced static mesh actor using a hierarchical instanced mesh component.
 * Inherits editor instance synchronization and the empty-component game-world fallback.
 */
UCLASS(meta = (ChildCanTick = true))
class TOROENGINE_API AHierarchicalInstancedStaticMeshActor : public AInstancedStaticMeshActor
{
	GENERATED_BODY()

public:

	/**
	 * Selects the HISM default component while retaining base asset defaults and disabled ticking.
	 * @param ObjectInit Initializer used to override the inherited mesh component class.
	 */
	AHierarchicalInstancedStaticMeshActor(const FObjectInitializer& ObjectInit);
};
