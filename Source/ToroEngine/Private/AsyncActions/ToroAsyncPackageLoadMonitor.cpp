// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "AsyncActions/ToroAsyncPackageLoadMonitor.h"
#include "UObject/UObjectGlobals.h"
#include "CoreGlobals.h"

UToroAsyncPackageLoadMonitor* UToroAsyncPackageLoadMonitor::MonitorAsyncPackageLoading(
	const UObject* ContextObject, float TestInterval, float IdleDuration)
{
	checkf(IsInGameThread(), TEXT("UToroAsyncPackageLoadMonitor::MonitorAsyncPackageLoading(...) should only be called from the GameThread."));

	UToroAsyncPackageLoadMonitor* Task = NewObject<UToroAsyncPackageLoadMonitor>();
	Task->SetWorldContext(ContextObject);
	Task->Interval = FMath::IsFinite(TestInterval) ? FMath::Max(TestInterval, 0.01f) : 0.1f;
	Task->IdleWindow = FMath::IsFinite(IdleDuration) ? FMath::Max(IdleDuration, 0.1f) : 0.2f;
	Task->RegisterWithGameInstance(Task->GetWorld());
	return Task;
}

void UToroAsyncPackageLoadMonitor::Cancel()
{
	checkf(IsInGameThread(), TEXT("UToroAsyncPackageLoadMonitor::Cancel() should only be called from the GameThread."));

	if (!bFinished)
	{
		MarkFinished();
		SetReadyToDestroy();
	}
}

void UToroAsyncPackageLoadMonitor::Activate()
{
	checkf(IsInGameThread(), TEXT("UToroAsyncPackageLoadMonitor::Activate() should only be called from the GameThread."));

	if (bFinished || TickHandle.IsValid())
	{
		return;
	}

	Super::Activate();

	TickTime = 0.0f;
	IdleTime = IdleWindow;
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UToroAsyncPackageLoadMonitor::OnTick));
}

void UToroAsyncPackageLoadMonitor::MarkFinished()
{
	bFinished = true;
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
}

bool UToroAsyncPackageLoadMonitor::OnTick(float DeltaTime)
{
	if (bFinished)
	{
		return false;
	}

	const bool bIsLoading = IsAsyncLoading();
	IdleTime = bIsLoading ? IdleWindow : (IdleTime - DeltaTime);
	if (IdleTime <= 0.0f)
	{
		MarkFinished();
		OnCompletedNative.Broadcast(TotalPackages, TotalPackages, false);
		OnCompleted.Broadcast(TotalPackages, TotalPackages, false);
		SetReadyToDestroy();
		return false;
	}

	if ((TickTime -= DeltaTime) > 0.0f)
	{
		return true;
	}
	TickTime = Interval;

	const int32 NumPackages = GetNumAsyncPackages();
	if (NumPackages == RemainPackages)
	{
		return true;
	}

	RemainPackages = NumPackages;
	TotalPackages = FMath::Max(TotalPackages, RemainPackages);
	const int32 CompletedPackages = TotalPackages - RemainPackages;

	OnUpdateNative.Broadcast(CompletedPackages, TotalPackages, bIsLoading);
	OnUpdate.Broadcast(CompletedPackages, TotalPackages, bIsLoading);
	return !bFinished;
}

void UToroAsyncPackageLoadMonitor::BeginDestroy()
{
	MarkFinished();
	Super::BeginDestroy();
}
