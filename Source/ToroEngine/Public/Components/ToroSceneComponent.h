// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Components/SceneComponent.h"
#include "ToroSceneComponent.generated.h"

/**
 * Base scene component with a repeatable construction hook and automatic editor sprite visibility.
 * Construction runs after each registration and after editor property changes when the instance survives.
 * Native ticking is disabled by default; Blueprint subclasses may opt into ticking.
 */
UCLASS(Blueprintable, BlueprintType, meta = (ChildCanTick = true))
class TOROENGINE_API UToroSceneComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	UToroSceneComponent();

protected:
#if WITH_EDITORONLY_DATA
	/**
	 * Allows editor ticking by setting bTickInEditor during registration.
	 * Tick capability and tick enablement must also be configured separately.
	 */
	UPROPERTY(EditDefaultsOnly, Category = ComponentTick, DisplayName = "Tick in Editor")
	bool bShouldTickInEditor = false;
#endif

	/**
	 * Blueprint construction hook called by the base OnConstruction implementation.
	 * May run repeatedly during registration and editor property changes; keep repeated calls safe.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Construction Script", meta = (ForceAsFunction = true))
	void ReceiveConstruction();

	/**
	 * Native construction hook called after registration and on surviving instances after editor property changes.
	 * Overrides should call Super::OnConstruction() to dispatch ReceiveConstruction().
	 */
	virtual void OnConstruction();

	virtual void OnRegister() override;

#if WITH_EDITOR
	virtual void OnChildAttached(USceneComponent* ChildComponent) override;
	virtual void OnChildDetached(USceneComponent* ChildComponent) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

private:

	/**
	 * Updates the existing editor sprite using visualization settings and recursive child components.
	 * Hides it in game mode or when a child requests visualization or is a visible non-visualization primitive.
	 * Does nothing if no sprite exists.
	 */
	void CheckForSpriteVisualization() const;
#endif
};
