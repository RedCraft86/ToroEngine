// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "AsyncMessageId.h"
#include "SMStateInstance.h"
#include "SMTextGraphProperty.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "DialogueNodes.generated.h"

class USMCompilerLog;
enum class EGameFlagScope : uint8;

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FDialogueFlagInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = Flag)
	EGameFlagScope Scope;

	UPROPERTY(EditAnywhere, Category = Flag)
	bool bState = true;
};

UCLASS(Abstract, NotBlueprintable, BlueprintType, PrioritizeCategories = (Dialogue))
class TOROSYSTEMS_API UDialogueNodeBase : public USMStateInstance
{
	GENERATED_BODY()

public:

	UDialogueNodeBase();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Dialogue)
	FSMTextGraphProperty Line;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Dialogue, meta = (ForceInlineRow = true))
	TMap<FGameplayTag, FDialogueFlagInfo> Flags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Dialogue, meta = (ForceInlineRow = true))
	TMap<FAsyncMessageId, FInstancedStruct> Events;

	virtual FText GetSpeakerName() const;

protected:

	virtual void OnStateBegin_Implementation() override;
#if WITH_EDITOR
	virtual void OnPreCompileValidate_Implementation(USMCompilerLog* CompilerLog) const override;
#endif
};

UCLASS(NotBlueprintable, BlueprintType, DisplayName = "Dialogue Line")
class TOROSYSTEMS_API UDialogueLineNode final : public UDialogueNodeBase
{
	GENERATED_BODY()

public:

	UDialogueLineNode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dialogue, meta = (Categories = "Character"))
	FGameplayTag Speaker;

	virtual FText GetSpeakerName() const override;
};

UCLASS(NotBlueprintable, BlueprintType, DisplayName = "Dialogue Choice")
class TOROSYSTEMS_API UDialogueChoiceNode final : public UDialogueNodeBase
{
	GENERATED_BODY()

public:

	UDialogueChoiceNode();

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dialogue)
	uint8 AutoShortText = 10;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dialogue,
		meta = (MultiLine = false, EditCondition = "AutoShortText == 0"))
	FText ShortText;

	virtual FText GetSpeakerName() const override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChanged) override;
#endif
};
