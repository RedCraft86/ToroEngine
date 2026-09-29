// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Components/ActorComponent.h"
#include "ToroActorComponent.generated.h"

/**
 * Blueprintable actor component with optional ticking, editor construction callbacks, and an instance limit.
 * Ticking is disabled at runtime by default; derived classes may enable it. The construction event runs
 * on registration and after property edits to non-template instances, so it may be called repeatedly.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROCORE_API UToroActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UToroActorComponent();

protected:

#if WITH_EDITORONLY_DATA
	/**
	 * Maximum instances of this component's actual class and its derived classes allowed on one owner actor.
	 * With A -> B -> C inheritance, an A instance counts A/B/C, B counts B/C, and C counts C.
	 * Zero disables the limit. Editor validation allows the first N in owner component order.
	 * Editable only on templates.
	 */
	UPROPERTY(EditAnywhere, Category = Activation)
	uint8 MaxInstancesPerActor = 0;

	/** Enables this component's tick during editor time when it is registered. */
	UPROPERTY(EditDefaultsOnly, Category = ComponentTick, DisplayName = "Tick in Editor")
	bool bTickDuringEditor = false;
#endif

	/** Blueprint construction callback invoked on registration and after instance property edits. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Construction Script", meta = (ForceAsFunction = true))
	void EventConstruction();

	/** Invoked on registration and after instance property edits; also calls blueprint event. */
	virtual void OnConstruction();

	virtual void OnRegister() override;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};