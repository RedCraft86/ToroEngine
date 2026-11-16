// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/Widgets/ToroFrameRateWidget.h"
#include "UserSettings/ToroGameUserSettings.h"
#include "Components/TextBlock.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarShowFrameRate(
	TEXT("r.Debug.ShowFrameRate"),
	0,
	TEXT("Show the frame-rate widget. 0: off, 1: on"),
	ECVF_Default
);

extern ENGINE_API float GAverageFPS;
extern ENGINE_API float GAverageMS;

UToroFrameRateWidget::UToroFrameRateWidget()
	: GoodColor(FLinearColor::Green)
	, BadColor(FLinearColor::Red)
{
	AutoLayer = EToroAutoWidgetType::Overlay;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UToroFrameRateWidget::OnShowFrameRateChanged(IConsoleVariable* Variable) const
{
	InfoTextBlock->SetVisibility(Variable->GetInt() != 0
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed);
}

void UToroFrameRateWidget::NativeDestruct()
{
	CVarShowFrameRate->OnChangedDelegate().RemoveAll(this);
	Super::NativeDestruct();
}

void UToroFrameRateWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!InfoTextBlock)
	{
		RemoveFromParent();
		return;
	}

	CVarShowFrameRate->OnChangedDelegate().AddUObject(this, &UToroFrameRateWidget::OnShowFrameRateChanged);
	OnShowFrameRateChanged(CVarShowFrameRate.AsVariable());
}

void UToroFrameRateWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Settings = UToroGameUserSettings::Get();
}

void UToroFrameRateWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!Settings.IsValid() || !InfoTextBlock 
		|| InfoTextBlock->GetVisibility() == ESlateVisibility::Collapsed)
	{
		TickTime = 0.0f;
		return;
	}

	if ((TickTime -= InDeltaTime) > 0.0f)
	{
		return;
	}
	TickTime = 0.2f;

	float MaxFPS = Settings->GetFrameRateLimit();
	if (MaxFPS <= 0.0f)
	{
		MaxFPS = 60.0f;
	}

	const float Percent = FMath::GetMappedRangeValueClamped(
		FVector2D(10.0f, MaxFPS), FVector2D(0.0f, 1.0f), GAverageFPS);
	InfoTextBlock->SetColorAndOpacity(
		FMath::Lerp(BadColor, GoodColor, Percent).CopyWithNewOpacity(1.0f)
	);

	FNumberFormattingOptions FPSFormat;
	FPSFormat.SetMinimumFractionalDigits(0);
	FPSFormat.SetMaximumFractionalDigits(0);
	FPSFormat.SetRoundingMode(FromZero);
	FPSFormat.SetUseGrouping(false);

	FNumberFormattingOptions MSFormat;
	MSFormat.SetMinimumFractionalDigits(2);
	MSFormat.SetMaximumFractionalDigits(2);
	MSFormat.SetUseGrouping(false);

	InfoTextBlock->SetText(FText::Format(
		INVTEXT("{0} FPS\n{1} MS"),
		FText::AsNumber(GAverageFPS, &FPSFormat),
		FText::AsNumber(GAverageMS, &MSFormat)
	));
}
