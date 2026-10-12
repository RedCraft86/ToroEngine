// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Engine/StaticMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "InstancedStaticMeshActor.generated.h"

/**
 * Static mesh actor whose mesh component stores instanced geometry.
 * Editor construction rebuilds the component from local-space authoring transforms and restores
 * one identity instance when the array is empty. Authoring data and synchronization are editor-only;
 * cooked placed actors use serialized component instances, while runtime spawning does not
 * automatically create the authoring array's default instance. Native ticking is disabled.
 */
UCLASS(meta = (ChildCanTick = true))
class TOROENGINE_API AInstancedStaticMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:

	AInstancedStaticMeshActor(const FObjectInitializer& ObjectInit);

#if WITH_EDITORONLY_DATA
	/**
	 * Editor authoring transforms in mesh-component-local space, initially one identity transform.
	 * Construction replaces an empty array with one identity transform unless synchronization is locked.
	 */
	UPROPERTY(Category = StaticMesh, EditAnywhere, meta = (MakeEditWidget, DisplayPriority = 1))
	TArray<FTransform> Instances = { FTransform::Identity };
#endif

	/**
	 * Gets the actor's mesh component cast to the requested component type.
	 * @tparam T Component type to request; defaults to UInstancedStaticMeshComponent.
	 * @return Component pointer, or nullptr when no compatible component is available.
	 */
	template<typename T = UInstancedStaticMeshComponent>
	T* GetMeshComponent() const
	{
		return Cast<T>(GetStaticMeshComponent());
	}

#if WITH_EDITOR
	/**
	 * Replaces all component instances with the current local-space authoring transforms.
	 * Clears and recreates instances even when transforms are unchanged, discarding per-instance
	 * custom data and recreating associated component state. Does nothing while synchronization
	 * is locked or the component is missing. Unlike construction, this method permits an empty array.
	 */
	void UpdateInstances() const;

	/**
	 * Replaces the authoring array with successfully read component-local instance transforms.
	 * Skips failed transform reads, leaves the array unchanged if the component is missing,
	 * and empties it if the component has no instances. Temporarily locks synchronization
	 * and restores the previous lock state on return; does not copy per-instance custom data.
	 */
	void CopyInstancesFromComponent();

protected:

	/** Suppresses construction-driven authoring changes and component rebuilds while instances are copied. */
	bool bLockInstances = false;

	virtual void OnConstruction(const FTransform& Transform) override;
#endif
};

/**
 * Instanced mesh actor using a hierarchical instanced component for clustered geometry.
 * Inherits the editor authoring, full-rebuild synchronization, and minimum-one construction behavior.
 */
UCLASS(meta = (ChildCanTick = true))
class TOROENGINE_API AHierarchicalInstancedStaticMeshActor : public AInstancedStaticMeshActor
{
	GENERATED_BODY()

public:

	AHierarchicalInstancedStaticMeshActor(const FObjectInitializer& ObjectInit);
};
