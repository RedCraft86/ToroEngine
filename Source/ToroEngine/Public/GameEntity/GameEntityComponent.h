// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Components/ToroActorComponent.h"
#include "GameEntityComponent.generated.h"

class AGameEntityActor;

/**
 * Base for modular components hosted by AGameEntityActor.
 * Registration binds direct scalar reflected FGameEntityComponentRef fields, including inherited
 * fields, before the base construction callback runs. Unregistration clears those bindings and caches.
 * Nested structs and container-held references are not traversed automatically.
 * Owner-type and component-instance restrictions are editor validation, not runtime enforcement.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (Modular), meta = (BlueprintSpawnableComponent, ChildCanTick = true))
class TOROENGINE_API UGameEntityComponent : public UToroActorComponent
{
	GENERATED_BODY()

public:

	/**
	 * Disables native component tick capability, enables automatic activation, and sets the editor
	 * instance limit to one for this component's actual class and its derived classes.
	 */
	UGameEntityComponent();

protected:

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:

	/**
	 * Binds direct reflected reference fields to an entity owner and invalidates their cached components.
	 * Only direct scalar FGameEntityComponentRef fields are supported; nested structs and containers
	 * are not traversed automatically.
	 * @param Owner Entity actor used for lookup; nullptr clears owner bindings and cached components.
	 */
	void SetReferenceOwner(AActor* Owner);
};
