// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Objectives/ObjectiveSubsystem.h"
#include "Objectives/ObjectiveSystemConfig.h"
#include "Objectives/ObjectiveGraph.h"
#include "Objectives/ObjectiveNodes.h"
#include "Objectives/ToroLoadingWidget.h"
#include "Framework/ToroPlayerCharacter.h"
#include "Framework/ToroPlayerHUD.h"
#include "SaveSystem/SaveGameSubsystem.h"
#include "UserInterface/UserInterfaceConfig.h"
#include "UserInterface/UserInterfaceTypes.h"
#include "WorldPartition/DataLayer/DataLayerAsset.h"
#include "WorldPartition/DataLayer/DataLayerInstance.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "WorldPartition/WorldPartitionRuntimeCell.h"
#include "WorldPartition/WorldPartitionStreamingSource.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "DataTypes/ToroLevelInfo.h"
#include "Utilities/ToroLogging.h"
#include "Actors/TeleportPoint.h"
#include "GameFlagManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	TSubclassOf<UToroLoadingWidget> GetLoadingWidgetClass()
	{
		const FUserInterfaceConfig* Config = FUserInterfaceConfig::Get();
		return Config ? Config->LoadingWidgetClass.LoadSynchronous() : nullptr;
	}
}

void UObjectiveSubsystem::FinishTask(FGameplayTag Task, uint8 Amount)
{
	if (ObjectiveTags::IsValid(Task) && Amount != 0)
	{
		uint8& EntryVal = Tasks.FindOrAdd(Task, 0);
		EntryVal = static_cast<uint8>(FMath::Clamp(static_cast<int32>(Amount) + EntryVal, 0, 255));

		OnTaskUpdatedEvent.Broadcast(Task, EntryVal);
		if (CurrentStep.IsValid() && CurrentStep->CountTasks.Contains(Task))
		{
			RequestTransitionEvaluation();
		}
	}
}

uint8 UObjectiveSubsystem::GetTaskProgress(FGameplayTag Task) const
{
	return (ObjectiveTags::IsValid(Task) && Tasks.Contains(Task)) ? Tasks[Task] : 0;
}

void UObjectiveSubsystem::StopStreamingWait()
{
	GetWorld()->GetTimerManager().ClearTimer(StreamingCheckHandle);
	WaitingLayers.Reset();

	if (bLoadingWidget)
	{
		bLoadingWidget = false;

		if (const AToroPlayerHUD* HUD = AToroPlayerHUD::Get(this))
		{
			HUD->RemoveWidgetByClass(EToroMasterWidgetType::Menu, GetLoadingWidgetClass());
		}
	}
}

void UObjectiveSubsystem::CheckStreamingCompletion()
{
	const UWorldPartitionSubsystem* Partition = GetWorld()->GetSubsystem<UWorldPartitionSubsystem>();
	if (Partition)
	{
		if (const AToroPlayerCharacter* Player = AToroPlayerCharacter::Get(this))
		{
			TArray<FWorldPartitionStreamingQuerySource> Queries;
			FWorldPartitionStreamingQuerySource& Query = Queries.Emplace_GetRef(Player->GetActorLocation());
			Query.bDataLayersOnly = true;
			Query.DataLayers.SetNum(1);

			const UDataLayerManager* Manager = UDataLayerManager::GetDataLayerManager(GetWorld());
			for (auto Layer = WaitingLayers.CreateIterator(); Layer; ++Layer)
			{
				const UDataLayerInstance* Instance = Manager ? Manager->GetDataLayerInstance(Layer.Key()) : nullptr;
				if (!Instance || Instance->GetEffectiveRuntimeState() < Layer.Value())
				{
					TORO_REPORT_ERROR(
						TEXT("Objective Data Layer '%s' cannot reach its requested runtime state."),
						*Layer.Key().ToString()
					);
					Layer.RemoveCurrent();
					continue;
				}

				Query.DataLayers[0] = Layer.Key();
				const EWorldPartitionRuntimeCellState Target =
					Layer.Value() == EDataLayerRuntimeState::Activated
					? EWorldPartitionRuntimeCellState::Activated
					: EWorldPartitionRuntimeCellState::Loaded;
				if (Partition->IsStreamingCompleted(Target, Queries, false))
				{
					Layer.RemoveCurrent();
				}
			}
		}
	}

	if (!Partition || WaitingLayers.IsEmpty())
	{
		StopStreamingWait();
		RequestTransitionEvaluation();
	}
}

void UObjectiveSubsystem::RequestTransitionEvaluation()
{
	if (StateMachine && !PhaseSetupHandle.IsValid() && WaitingLayers.IsEmpty() && !TransitionEvaluationHandle.IsValid())
	{
		TransitionEvaluationHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(
			this, &UObjectiveSubsystem::EvaluateObjectiveTransitions
		);
	}
}

void UObjectiveSubsystem::EvaluateObjectiveTransitions()
{
	TransitionEvaluationHandle.Invalidate();
	if (StateMachine)
	{
		StateMachine->EvaluateTransitions();
	}
}

void UObjectiveSubsystem::ApplyState(const UObjectiveNode* OldState)
{
	if (!CurrentStep.IsValid())
	{
		return;
	}

	const bool bShouldWaitForLoad = !IsValid(OldState) || !CurrentStep->bContinuous;

	TMap<TObjectPtr<UDataLayerAsset>, EDataLayerRuntimeState> ResolvedDataLayers;
	if (IsValid(OldState))
	{
		for (const auto& Layer : OldState->DataLayers)
		{
			if (UDataLayerAsset* LayerAsset = Layer.Key.LoadSynchronous())
			{
				ResolvedDataLayers.Add(LayerAsset, EDataLayerRuntimeState::Unloaded);
			}
		}
	}

	for (const auto& Layer : CurrentStep->DataLayers)
	{
		if (UDataLayerAsset* LayerAsset = Layer.Key.LoadSynchronous())
		{
			ResolvedDataLayers.Add(LayerAsset,
				Layer.Value ? EDataLayerRuntimeState::Activated : EDataLayerRuntimeState::Loaded
			);
		}
	}

	const UWorld* World = GetWorld();
	UDataLayerManager* Manager = UDataLayerManager::GetDataLayerManager(World);
	const UWorldPartitionSubsystem* Partition = World->GetSubsystem<UWorldPartitionSubsystem>();
	for (const auto& Layer : ResolvedDataLayers)
	{
		const UDataLayerInstance* Instance = Manager ? Manager->GetDataLayerInstanceFromAsset(Layer.Key) : nullptr;
		if (!Instance || !Manager->SetDataLayerInstanceRuntimeState(Instance, Layer.Value))
		{
			TORO_REPORT_ERROR(
				TEXT("Could not set the runtime state of objective Data Layer '%s'."),
				*GetNameSafe(Layer.Key.Get())
			);
			continue;
		}

		if (bShouldWaitForLoad && Partition && Layer.Value != EDataLayerRuntimeState::Unloaded)
		{
			WaitingLayers.Add(Instance->GetFName(), Layer.Value);
		}
	}

	if (bShouldWaitForLoad)
	{
		if (const ATeleportPoint* Teleporter = CurrentStep->Teleporter.LoadSynchronous())
		{
			Teleporter->TeleportPlayer();
		}
	}

	if (const AToroPlayerCharacter* Player = AToroPlayerCharacter::Get(this))
	{
		Player->BlockCapabilities(this, static_cast<EPlayerCapability>(CurrentStep->PlayerRestrictions));
	}

	if (!WaitingLayers.IsEmpty())
	{
		World->GetTimerManager().SetTimer(StreamingCheckHandle, this,
			&UObjectiveSubsystem::CheckStreamingCompletion, 0.1f, true);
	}
	CheckStreamingCompletion();

	OnObjectiveUpdatedEvent.Broadcast(CurrentStep.Get());
	RequestTransitionEvaluation();

	if (!bRestoring && CurrentStep->bSavable && SaveSystem.IsValid() && SaveSystem->GetSlotted())
	{
		if (FObjectSaveData* Data = SaveSystem->GetSlotted()->GetObjectRecord(Identity))
		{
			if (!Data->Serialize(this))
			{
				TORO_REPORT_ERROR(TEXT("Failed to serialize objective save data."));
			}
		}
		else
		{
			TORO_REPORT_ERROR(TEXT("Failed to save objective save data."));
		}
	}
}

void UObjectiveSubsystem::ArriveAtState(const UObjectiveNode* State)
{
	if (IsValid(State) && State != CurrentStep)
	{
		const UObjectiveNode* OldState = CurrentStep.Get();
		ActiveStep = State->GetGuid();
		CurrentStep = State;

		if ((!OldState || !State->bContinuous) && !State->DataLayers.IsEmpty())
		{
			if (const AToroPlayerHUD* HUD = AToroPlayerHUD::Get(this))
			{
				bLoadingWidget = IsValid(HUD->AddWidgetByClass(EToroMasterWidgetType::Menu, GetLoadingWidgetClass()));
			}
		}

		if (bLoadingWidget)
		{
			GetWorld()->GetTimerManager().SetTimer(PhaseSetupHandle, FTimerDelegate::CreateWeakLambda(
				this, [this, PreviousStep = TWeakObjectPtr(OldState)]
				{
					PhaseSetupHandle.Invalidate();
					ApplyState(PreviousStep.Get());
				}), 1.0f, false);
		}
		else
		{
			ApplyState(OldState);
		}
	}
}

bool UObjectiveSubsystem::CanAdvanceFrom(const UObjectiveNode* State) const
{
	if (!State || CurrentStep.Get() != State || PhaseSetupHandle.IsValid() || !WaitingLayers.IsEmpty())
	{
		return false;
	}

	for (const auto& [Tag, Task] : State->CountTasks)
	{
		if (GetTaskProgress(Tag) < Task.Amount)
		{
			return false;
		}
	}

	const UGameFlagManager* FlagManager = UGameFlagManager::Get(this);
	for (const auto& RequiredFlag : State->FlagTasks)
	{
		if (!FlagManager || !FlagManager->HasGameFlag(RequiredFlag.Key, RequiredFlag.Value.Scope))
		{
			return false;
		}
	}

	return true;
}

void UObjectiveSubsystem::HandleFlagChanged(FGameplayTag Flag, bool bRemoved, EGameFlagScope Scope)
{
	if (CurrentStep.IsValid())
	{
		const FObjectiveFlagTaskInfo* Requirement = CurrentStep->FlagTasks.Find(Flag);
		if (!bRemoved && Requirement && Requirement->Scope == Scope)
		{
			RequestTransitionEvaluation();
		}
	}
}

void UObjectiveSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	const FToroLevelInfo* LevelInfo = FToroLevelInfo::Get(&InWorld);
	const FObjectiveSystemConfig* Config = FObjectiveSystemConfig::Get();
	if (!Config || !LevelInfo || Config->AllowedOnMap != LevelInfo->LevelType || Config->Objective.IsNull())
	{
		return;
	}

	const TSubclassOf<UObjectiveGraph> GraphClass = Config->Objective.LoadSynchronous();
	if (IsValid(GraphClass) && !GraphClass->HasAnyClassFlags(CLASS_Abstract))
	{
		StateMachine = NewObject<UObjectiveGraph>(this, GraphClass);
		if (!StateMachine)
		{
			return;
		}

		StateMachine->SetCanEverTick(false);
		StateMachine->Initialize(this);
		if (ActiveStep.IsValid())
		{
			StateMachine->LoadFromState(ActiveStep);
		}

		// The HUD creates its master widget during actor BeginPlay.
		PhaseSetupHandle = InWorld.GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]
		{
			PhaseSetupHandle.Invalidate();
			StateMachine->Start();
		}));
	}
}

bool UObjectiveSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const FToroLevelInfo* LevelInfo = FToroLevelInfo::Get(Outer);
	return Super::ShouldCreateSubsystem(Outer) && LevelInfo
		&& LevelInfo->LevelType == LevelTypeTags::Gameplay;
}

bool UObjectiveSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UObjectiveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Identity = FTaggedIdentity(ObjectiveTags::Base, true);

	const TGuardValue<bool> GuardRestore(bRestoring, true);

	SaveSystem = USaveGameSubsystem::Get(this);
	if (SaveSystem.IsValid() && SaveSystem->GetSlotted())
	{
		if (const FObjectSaveData* Data = SaveSystem->GetSlotted()->FindObjectRecord(Identity))
		{
			if (!Data->Deserialize(this))
			{
				TORO_REPORT_ERROR(TEXT("Failed to deserialize objective save data."));
			}
		}
	}

	if (UGameFlagManager* FlagManager = UGameFlagManager::Get(this))
	{
		FlagManager->OnFlagStateChanged().AddUObject(this, &UObjectiveSubsystem::HandleFlagChanged);
	}
}

void UObjectiveSubsystem::Deinitialize()
{
	if (UGameFlagManager* FlagManager = UGameFlagManager::Get(this))
	{
		FlagManager->OnFlagStateChanged().RemoveAll(this);
	}
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TransitionEvaluationHandle);
		World->GetTimerManager().ClearTimer(PhaseSetupHandle);
		StopStreamingWait();
	}
	if (StateMachine)
	{
		StateMachine->Shutdown();
		StateMachine = nullptr;
	}
	CurrentStep.Reset();
	Super::Deinitialize();
}
