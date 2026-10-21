// Copyright (C) RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Tickable.h"
#include "Containers/Ticker.h"
#include "UObject/Object.h"
#include "ToroObject.generated.h"

/**
 * Blueprint-extensible object with deferred construction, gameplay initialization, and optional ticking.
 * Construction waits for loading to finish and a valid world, and reruns after editor property edits.
 * Gameplay initialization waits for world BeginPlay; it is not ordered against an owning actor's BeginPlay.
 * Lifecycle callbacks and tick controls are intended for the game thread.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROENGINE_API UToroObject : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:

	UToroObject();

	/**
	 * Changes whether this object is permitted to tick.
	 * Actual ticking also requires lifecycle readiness, a valid world, and applicable pause/editor settings.
	 * @param bEnabled Whether to permit ticking.
	 */
	UFUNCTION(BlueprintCallable, Category = Object)
	void SetTickEnabled(bool bEnabled);

	/** Reports the configured tick permission, which does not guarantee that ticking is currently possible. */
	UFUNCTION(BlueprintPure, Category = Object)
	bool IsTickEnabled() const;

	virtual UWorld* GetWorld() const override;

protected:

	/** Default tick permission, also changed at runtime by SetTickEnabled(). */
	UPROPERTY(EditDefaultsOnly, Category = Ticking)
	bool bCanEverTick;

	/** Allows an otherwise eligible object to tick while its world is paused. */
	UPROPERTY(EditDefaultsOnly, Category = Ticking)
	bool bTickWhenPaused;

#if WITH_EDITORONLY_DATA
	/** Allows a constructed, tick-enabled object to tick in a non-game editor world. */
	UPROPERTY(EditDefaultsOnly, Category = Ticking)
	bool bTickInEditor;
#endif

	/**
	 * Blueprint construction hook, called after loading when a valid world is available.
	 * Also runs after instance property edits in a non-game editor world; implementations must tolerate reruns.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Construction Script", meta = (ForceAsFunction = true))
	void ReceiveConstruction();

	/**
	 * Blueprint gameplay initialization hook, called once after construction when the game world has begun play.
	 * Objects created after world BeginPlay initialize on a later core ticker update.
	 * Never called for templates or non-game editor worlds.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Initialize")
	void ReceiveInitialize();

	/**
	 * Blueprint tick hook, dispatched only while the object is eligible to tick.
	 * @param DeltaTime Elapsed seconds supplied by the engine tick dispatcher.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Tick")
	void ReceiveTick(float DeltaTime);

	/** Native construction hook; call the base implementation to dispatch ReceiveConstruction(). */
	virtual void OnConstruction();

	/** Native gameplay initialization hook; call the base implementation to dispatch ReceiveInitialize(). */
	virtual void OnInitialize();

	/** Native gameplay tick hook; call the base implementation to dispatch ReceiveTick(DeltaTime). */
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

private:

	/** Whether the initial deferred construction hook has been entered. */
	bool bConstructed = false;

	/** Whether gameplay initialization has been entered; remains false in non-game worlds. */
	bool bInitialized = false;

	/** Core ticker registration retained while loading, world availability, or world BeginPlay is pending. */
	FTSTicker::FDelegateHandle InitHandle;

	/**
	 * Advances deferred construction and gameplay initialization, enabling ticking when ready.
	 * Cancels polling on destruction or world teardown.
	 * @return True to poll again; false to remove this ticker registration.
	 */
	bool TryInitialize(float);

	/** Reports whether this instance is valid, is not a template, and has not entered destruction. */
	bool IsSafeObject() const;
};

/** Inline-editable, per-owner instanced variant of UToroObject for instanced object properties. */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class TOROENGINE_API UToroInstancedObject : public UToroObject
{
	GENERATED_BODY()

public:

	UToroInstancedObject();
};
