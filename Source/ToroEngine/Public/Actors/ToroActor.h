// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/Actor.h"
#include "Interfaces/ActivatableObject.h"
#include "ToroActor.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType, meta = (ChildCanTick = true))
class TOROENGINE_API AToroActor : public AActor, public IActivatableObject
{
	GENERATED_BODY()

public:

	AToroActor();

	virtual bool GetActiveState_Implementation() const override;

	virtual void SetActiveState_Implementation(bool bNewState) override;

protected:

};
