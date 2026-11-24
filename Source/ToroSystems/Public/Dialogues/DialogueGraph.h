// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SMInstance.h"
#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "GameplayTagContainer.h"
#include "DialogueGraph.generated.h"

enum class EGameFlagScope : uint8;

namespace DialogueTags
{
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Base);

	[[nodiscard]] inline bool IsValid(const FGameplayTag& InTag)
	{
		return InTag.IsValid()
			&& InTag != Base.GetTag()
			&& InTag.MatchesTag(Base.GetTag());
	}
}

UCLASS(DisplayName = "Dialogue Root", PrioritizeCategories = (Dialogue))
class TOROSYSTEMS_API UDialogueRootNode final : public USMStateMachineInstance
{
	GENERATED_BODY()

public:

	UDialogueRootNode();

	UPROPERTY(EditAnywhere, Category = Dialogue, meta = (Categories = "Dialogue"))
	FGameplayTag DialogueTag;

	UPROPERTY(EditAnywhere, Category = Dialogue)
	EGameFlagScope RememberScope;
};

UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API UDialogueGraph final : public USMInstance
{
	GENERATED_BODY()

public:

	UDialogueGraph();
};
