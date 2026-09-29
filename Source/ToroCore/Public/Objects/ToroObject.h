// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Tickable.h"
#include "UObject/Object.h"
#include "ToroObject.generated.h"

/**
 * Instanced Blueprintable object with optional ticking and editor construction callbacks.
 * No automatic initialization is performed. Keep a strong UObject reference to the instance,
 * tick registration and the outer do not keep it alive automatically.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class TOROCORE_API UToroObject : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:

	UToroObject();

	/** Sets the requested tick state for this object. */
	UFUNCTION(BlueprintCallable, Category = Object)
	void SetTickEnabled(const bool bEnabled);

	/** Reports the requested tick state of this context. */
	UFUNCTION(BlueprintPure, Category = Object)
	bool IsTickEnabled() const;

	/** Returns the valid outer's world, or nullptr for templates and missing world contexts. */
	virtual UWorld* GetWorld() const override;

protected:

	/** Requested tick state; setting a default does not register ticking without a SetTickEnabled call. */
	UPROPERTY(EditDefaultsOnly, Category = Ticking)
	bool bCanTick;

	/** Whether eligible game-world ticks continue while paused. */
	UPROPERTY(EditDefaultsOnly, Category = Ticking)
	bool bTickWhenPaused;

#if WITH_EDITORONLY_DATA
	/** Allows editor-world ticking after explicit registration, subject to normal tick eligibility. */
	UPROPERTY(EditDefaultsOnly, Category = Ticking, meta = (EditCondition = bCanTick))
	bool bTickInEditor = false;
#endif

	/**
	 * Receives qualifying editor property changes through OnConstruction.
	 * This event is also dispatched on post-init properties.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Construction Script", meta = (ForceAsFunction = true))
	void ReceiveConstruction();

	/** Receives an eligible tick with DeltaTime measured in seconds. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Tick")
	void ReceiveTick(const float DeltaTime);

	/** Calls ReceiveConstruction; overrides should call Super to retain Blueprint dispatch. */
	virtual void OnConstruction();

	/** Called every tick if ticking is enabled. */
	virtual void Tick(float DeltaTime) override;

	virtual void BeginDestroy() override;
	virtual void PostInitProperties() override;

	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual TStatId GetStatId() const override;

#if WITH_EDITOR
	virtual bool IsTickableInEditor() const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
