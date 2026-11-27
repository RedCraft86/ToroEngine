// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFlags/GameFlagManager.h"
#include "SMTransitionInstance.h"
#include "DialogueTransitions.generated.h"

UCLASS(Abstract, NotBlueprintable, BlueprintType, PrioritizeCategories = (Dialogue))
class TOROSYSTEMS_API UDialogueTransitionBase : public USMTransitionInstance
{
	GENERATED_BODY()

public:

	bool IsAvailable() const;

protected:

	UPROPERTY(EditDefaultsOnly, Category = Dialogue)
	FGameplayTag RequiredFlag;

	UPROPERTY(EditDefaultsOnly, Category = Dialogue)
	EGameFlagScope FlagScope = EGameFlagScope::SlotSave;

	virtual bool CanEnterTransition_Implementation() const override;
	virtual void OnTransitionEntered_Implementation() override;
};

UCLASS(NotBlueprintable, BlueprintType, DisplayName = "Dialogue Continue")
class TOROSYSTEMS_API UDialogueContinueTransition final : public UDialogueTransitionBase
{
	GENERATED_BODY()

public:

	UDialogueContinueTransition();
};

UCLASS(NotBlueprintable, BlueprintType, DisplayName = "Dialogue Choice")
class TOROSYSTEMS_API UDialogueChoiceTransition final : public UDialogueTransitionBase
{
	GENERATED_BODY()

public:

	UDialogueChoiceTransition();
};
