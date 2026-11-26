// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Engine/TimerHandle.h"
#include "ObjectiveSystemConfig.h"
#include "DataTypes/TaggedIdentity.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectiveSubsystem.generated.h"

class UObjectiveNode;
class UObjectiveGraph;
class USaveGameSubsystem;
enum class EGameFlagScope : uint8;
enum class EDataLayerRuntimeState : uint8;

UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class UObjectiveSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

	friend class UObjectiveNode;
	friend class UObjectiveTransition;

public:

	DECLARE_MULTICAST_DELEGATE_TwoParams(FTaskUpdated, const FGameplayTag&, uint8)
	DECLARE_MULTICAST_DELEGATE_OneParam(FObjectiveUpdated, const UObjectiveNode*)

	UFUNCTION(BlueprintCallable, Category = Objectives)
	void FinishTask(UPARAM(meta = (Categories = "Objective")) FGameplayTag Task, uint8 Amount = 1);

	UFUNCTION(BlueprintPure, Category = Objectives)
	uint8 GetTaskProgress(UPARAM(meta = (Categories = "Objective")) FGameplayTag Task) const;

	[[nodiscard]] const UObjectiveNode* GetCurrentStep() const { return CurrentStep.Get(); }

	[[nodiscard]] FORCEINLINE FTaskUpdated::RegistrationType& OnTaskUpdated()
	{
		return OnTaskUpdatedEvent;
	}

	[[nodiscard]] FORCEINLINE FObjectiveUpdated::RegistrationType& OnObjectiveUpdated()
	{
		return OnObjectiveUpdatedEvent;
	}

private:

	static inline const FTaggedIdentity Identity = FTaggedIdentity(ObjectiveTags::Base, true);

	UPROPERTY(Transient)
	TObjectPtr<UObjectiveGraph> StateMachine;

	UPROPERTY(SaveGame)
	FGuid ActiveStep;

	UPROPERTY(SaveGame)
	TMap<FGameplayTag, uint8> Tasks;

	bool bRestoring = false;
	bool bLoadingWidget = false;
	FTaskUpdated OnTaskUpdatedEvent;
	FObjectiveUpdated OnObjectiveUpdatedEvent;
	TMap<FName, EDataLayerRuntimeState> WaitingLayers;
	TWeakObjectPtr<const UObjectiveNode> CurrentStep;
	TWeakObjectPtr<USaveGameSubsystem> SaveSystem;
	FTimerHandle TransitionEvaluationHandle;
	FTimerHandle StreamingCheckHandle;
	FTimerHandle PhaseSetupHandle;

	void StopStreamingWait();
	void CheckStreamingCompletion();
	void RequestTransitionEvaluation();
	void EvaluateObjectiveTransitions();
	void ApplyState(const UObjectiveNode* OldState);
	void ArriveAtState(const UObjectiveNode* State);
	bool CanAdvanceFrom(const UObjectiveNode* State) const;
	void HandleFlagChanged(FGameplayTag Flag, bool bRemoved, EGameFlagScope Scope);

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
