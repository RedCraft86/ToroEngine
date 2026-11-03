// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DataTypes/TaggedIdentity.h"
#include "ActorBehaviors/ActorBehaviorBase.h"
#include "ActorBehaviors/ActorBehaviorRef.h"
#include "SaveGameBehavior.generated.h"

class USaveGameSubsystem;

UCLASS(MinimalAPI, NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent))
class USaveGameBehavior final : public UActorBehaviorBase
{
	GENERATED_BODY()

public:

	USaveGameBehavior();

	UFUNCTION(BlueprintCallable, Category = ActorBehaviors)
	TOROSYSTEMS_API bool SaveActor();

	UFUNCTION(BlueprintCallable, Category = ActorBehaviors)
	TOROSYSTEMS_API bool LoadActor();

protected:

	UPROPERTY(EditAnywhere, Category = Dependencies, DisplayName = "Identity")
	FActorBehaviorRef IdentityRef;

	TWeakObjectPtr<USaveGameSubsystem> Subsystem;

	void OnLoadedDifferentSlot();
	TOptional<FTaggedIdentity> GetIdentity();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
