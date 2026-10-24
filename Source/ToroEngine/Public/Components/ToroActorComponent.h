// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Components/ActorComponent.h"
#include "ToroActorComponent.generated.h"

/**
 * Base actor component with a repeatable native and Blueprint construction hook.
 * Construction runs after each registration and after editor property changes when the instance survives.
 * Native ticking is disabled by default; Blueprint subclasses may opt into ticking.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, meta = (ChildCanTick = true))
class TOROENGINE_API UToroActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UToroActorComponent();

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
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
