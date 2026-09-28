// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Components/SceneComponent.h"
#include "ToroSceneComponent.generated.h"

/**
 * Scene component with optional editor ticking, Blueprint construction callbacks, and editor visualization.
 * EventConstruction runs on each registration and after property edits to non-template instances, so it
 * may run repeatedly. Editor sprite visibility reflects visible descendant components.
 */
UCLASS(Blueprintable, BlueprintType)
class TOROCORE_API UToroSceneComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	UToroSceneComponent();

protected:

#if WITH_EDITORONLY_DATA
	/** Enables this component's tick during editor time when registered. */
	UPROPERTY(EditDefaultsOnly, Category = ComponentTick, DisplayName = "Tick in Editor")
	bool bTickDuringEditor = false;
#endif

	/** Blueprint callback invoked on registration and after instance property edits; can run repeatedly. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Construction Script", meta = (ForceAsFunction = true))
	void EventConstruction();

	/** Invoked on registration and after instance property edits; also calls blueprint event. */
	virtual void OnConstruction();

	virtual void OnRegister() override;

#if WITH_EDITOR
	virtual void OnChildAttached(USceneComponent* ChildComponent) override;
	virtual void OnChildDetached(USceneComponent* ChildComponent) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

private:

	/** Shows the editor sprite only when this component visualizes and no descendant provides visible geometry. */
	void CheckForSpriteVisualization() const;
#endif
};
