// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DialogueSubsystem.generated.h"

class USMInstance;
class UDialogueGraph;
class UDialogueNodeBase;
class UDialogueTransitionBase;
class UGameFlagManager;

struct FDialoguePromptChoice
{
	FGuid TransitionId;
	FText Label;
};

/** A single presentation of a dialogue line, including its available responses. */
struct FDialoguePrompt
{
	FGuid Id;
	FText Speaker;
	FText Line;
	TArray<FDialoguePromptChoice> Choices;
	FGuid ContinueId;
};

UENUM(BlueprintType)
enum class EDialogueResultType : uint8
{
	Success,
	InvalidContext,
	InvalidDialogue,
	StateMachineError,
	DialoguePlaying
};

UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class UDialogueSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

	friend class UDialogueNodeBase;
	friend class UDialogueTransitionBase;

public:

	DECLARE_EVENT_OneParam(UDialogueSubsystem, FOnDialoguePromptChanged, const FDialoguePrompt&);
	DECLARE_EVENT(UDialogueSubsystem, FOnDialogueClosed);

	UDialogueSubsystem();

	UFUNCTION(BlueprintCallable, Category = Dialogues,
		DisplayName = "Start Dialogue", meta = (WorldContext = ContextObject))
	static EDialogueResultType Start(const UObject* ContextObject,
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UDialogueGraph> Dialogue,
		int32 PlayerIdx = 0);

	FOnDialoguePromptChanged& OnDialoguePromptChanged() { return DialoguePromptChanged; }
	FOnDialogueClosed& OnDialogueClosed() { return DialogueClosed; }
	const FDialoguePrompt& GetCurrentPrompt() const { return CurrentPrompt; }

	/** Submits an offered edge, or acknowledges the last line with an invalid edge ID. */
	bool SubmitInput(const FGuid& PromptId, const FGuid& TransitionId);

private:

	UPROPERTY(Transient)
	TObjectPtr<UDialogueGraph> StateMachine;

	TWeakObjectPtr<UGameFlagManager> FlagManager;
	TWeakObjectPtr<UDialogueTransitionBase> PendingTransition;
	TMap<FGuid, TWeakObjectPtr<UDialogueTransitionBase>> OfferedTransitions;
	FOnDialoguePromptChanged DialoguePromptChanged;
	FOnDialogueClosed DialogueClosed;
	FDialoguePrompt CurrentPrompt;
	bool bSubmittingInput = false;

	UFUNCTION()
	void OnDialogueEnded(USMInstance* Instance);

	void PresentNode(const UDialogueNodeBase* Node);
	EDialogueResultType OpenDialogue(TSubclassOf<UDialogueGraph> Dialogue);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
};
