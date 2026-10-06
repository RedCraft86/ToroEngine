// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "AsyncActions/ToroTrackAsyncLoading.h"
#include "UObject/UObjectGlobals.h"

UToroTrackAsyncLoading* UToroTrackAsyncLoading::TrackAsyncLoading(const UObject* ContextObject, float TestInterval, float IdleDuration)
{
	checkf(IsInGameThread(), TEXT("UTrackAsyncLoadAction::TrackAsyncLoading(...) should only be called from the GameThread."));

	UToroTrackAsyncLoading* Task = NewObject<UToroTrackAsyncLoading>();
	Task->SetWorldContext(ContextObject);
	Task->Interval = FMath::IsFinite(TestInterval) ? FMath::Max(TestInterval, 0.01f) : 0.1f;
	Task->IdleWindow = FMath::IsFinite(IdleDuration) ? FMath::Max(IdleDuration, 0.1f) : 0.2f;
	Task->RegisterWithGameInstance(Task->GetWorld());
	return Task;
}

void UToroTrackAsyncLoading::Cancel()
{
	checkf(IsInGameThread(), TEXT("UTrackAsyncLoadAction::Cancel() should only be called from the GameThread."));
	if (bFinished)
	{
		StopTracking();
		SetReadyToDestroy();
	}
}

void UToroTrackAsyncLoading::Activate()
{
	checkf(IsInGameThread(), TEXT("UTrackAsyncLoadAction::Activate() should only be called from the GameThread."));
	if (bFinished || TickHandle.IsValid())
	{
		return;
	}

	Super::Activate();
	IdleTime = IdleWindow;
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UToroTrackAsyncLoading::OnTick));
}

void UToroTrackAsyncLoading::StopTracking()
{
	bFinished = true;
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
}

bool UToroTrackAsyncLoading::OnTick(float DeltaTime)
{
	if (bFinished)
	{
		return false;
	}

	IdleTime = IsAsyncLoading() ? IdleWindow : (IdleTime - DeltaTime);
	if (IdleTime <= 0.0f)
	{
		StopTracking();
		const FIntPoint Packages(FMath::Max(TotalPackages, 1));
		OnCompletedNative.Broadcast(Packages, false);
		OnCompleted.Broadcast(Packages, false);
		SetReadyToDestroy();
		return false;
	}

	const int32 NumPackages = GetNumAsyncPackages();
	if (NumPackages == RemainPackages)
	{
		return true;
	}

	RemainPackages = NumPackages;
	TotalPackages = FMath::Max(TotalPackages, RemainPackages);
	const FIntPoint LoadProgress(TotalPackages - RemainPackages, TotalPackages);
	OnUpdateNative.Broadcast(LoadProgress, true);
	OnUpdate.Broadcast(LoadProgress, true);

	return true;
}

void UToroTrackAsyncLoading::BeginDestroy()
{
	StopTracking();
	Super::BeginDestroy();
}
