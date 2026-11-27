// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Templates/SubclassOf.h"
#include "ActorBehaviors/Action/ActionBehaviorBase.h"
#include "DialogueActionBehavior.generated.h"

class UDialogueGraph;

/** Starts a dialogue for the local player when triggered. */
UCLASS(NotBlueprintable, ClassGroup = (Feature), meta = (BlueprintSpawnableComponent), DisplayName = "[Misc] Start Dialogue")
class TOROSYSTEMS_API UDialogueActionBehavior final : public UActionBehaviorBase
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, Category = Settings, meta = (AllowAbstract = false))
	TSubclassOf<UDialogueGraph> Dialogue;

	virtual bool CanTrigger() const override;
	virtual void OnTriggered() override;
};
