// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Objectives/ToroLoadingWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

UToroLoadingWidget::UToroLoadingWidget()
{
	bPausesGame = false;
	bIsBackHandler = false;
	PercentFormat.SetMinimumFractionalDigits(0);
	PercentFormat.SetMaximumFractionalDigits(0);
}

void UToroLoadingWidget::NativeOnActivated()
{
	TickTime = 0.0f;
	TotalPackages = 0;
	RemainPackages = 0;
	LoadingBar->SetPercent(0.0f);
	LoadingText->SetText(INVTEXT("Loading..."));
	Super::NativeOnActivated();
	bTracking = true;
}

void UToroLoadingWidget::NativeOnDeactivated()
{
	bTracking = false;
	TotalPackages = 0;
	RemainPackages = 0;
	LoadingBar->SetPercent(1.0f);
	LoadingText->SetText(INVTEXT("Finished Loading"));
	Super::NativeOnDeactivated();
}

void UToroLoadingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!bTracking)
	{
		return;
	}

	static constexpr float Interval = 0.1f;
	if ((TickTime -= InDeltaTime) > 0.0f)
	{
		return;
	}
	TickTime = Interval;

	RemainPackages = GetNumAsyncPackages();
	TotalPackages = FMath::Max(TotalPackages, RemainPackages);
	if (TotalPackages == 0)
	{
		return;
	}

	const int32 LoadedPackages = TotalPackages - RemainPackages;
	const float LoadFraction = static_cast<float>(LoadedPackages) / TotalPackages;
	LoadingText->SetText(FText::Format(INVTEXT("Loading {0} ({1}/{2} Packages)"),
		FText::AsPercent(LoadFraction, &PercentFormat), LoadedPackages, TotalPackages));
	LoadingBar->SetPercent(LoadFraction);
}
