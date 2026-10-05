// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "AsyncActions/TrackAsyncLoadAction.h"

UTrackAsyncLoadAction* UTrackAsyncLoadAction::TrackAsyncLoading(const UObject* ContextObject, const float TestInterval, const uint8 FinishThreshold)
{
	checkf(IsInGameThread(), TEXT("UTrackAsyncLoadAction::TrackAsyncLoading(...) should only be called from the GameThread."));

	UTrackAsyncLoadAction* Task = NewObject<UTrackAsyncLoadAction>();
	Task->SetWorldContext(ContextObject);
	Task->Interval = FMath::Max(TestInterval, 0.01f);
	Task->Threshold = FinishThreshold;
	Task->RegisterWithGameInstance(Task->GetWorld());
	return Task;
}

void UTrackAsyncLoadAction::OnFinished()
{
	if (bFinished)
	{
		return;
	}

	bFinished = true;
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}

	const FIntPoint Count(MaxPackages);
	OnUpdateNative.Broadcast(Count, 1.0f);
	OnUpdate.Broadcast(Count, 1.0f);
	SetReadyToDestroy();
}

bool UTrackAsyncLoadAction::OnTick(float DeltaTime)
{
	if ((TickInterval -= DeltaTime) > 0.0f)
	{
		return true;
	}

	TickInterval = Interval;

	const int32 NumPackages = GetNumAsyncPackages();
	if (bInitialized && NumPackages <= Threshold)
	{
		OnFinished();
		return false;
	}

	MaxPackages = FMath::Max(MaxPackages, NumPackages);
	const float NewPercent = (MaxPackages == 0) ? 0.0f : (1.0f - (static_cast<float>(NumPackages) / static_cast<float>(MaxPackages)));
	Percent = FMath::Max(Percent, NewPercent);

	if (MaxPackages > 0)
	{
		const FIntPoint Count(MaxPackages - NumPackages, MaxPackages);
		OnUpdateNative.Broadcast(Count, Percent);
		OnUpdate.Broadcast(Count, Percent);

		bInitialized = true;
	}

	return true;
}

void UTrackAsyncLoadAction::Activate()
{
	if (bFinished || TickHandle.IsValid())
	{
		return;
	}

	Super::Activate();
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UTrackAsyncLoadAction::OnTick));
}

void UTrackAsyncLoadAction::BeginDestroy()
{
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}

	Super::BeginDestroy();
}
