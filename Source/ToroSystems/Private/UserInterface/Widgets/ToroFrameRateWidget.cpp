// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/Widgets/ToroFrameRateWidget.h"
#include "UserSettings/ToroGameUserSettings.h"
#include "Components/TextBlock.h"

extern ENGINE_API float GAverageFPS;
extern ENGINE_API float GAverageMS;

UToroFrameRateWidget::UToroFrameRateWidget()
	: GoodColor(FLinearColor::Green)
	, BadColor(FLinearColor::Red)
{
	AutoLayer = EToroAutoWidgetType::Overlay;
}

void UToroFrameRateWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!InfoTextBlock)
	{
		RemoveFromParent();
	}
}

void UToroFrameRateWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Settings = UToroGameUserSettings::Get();
}

void UToroFrameRateWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if ((TickTime -= InDeltaTime) > 0.0f)
	{
		return;
	}
	TickTime = 0.2f;

	if (Settings.IsValid() && Settings->GetShowFrameRate())
	{
		InfoTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		InfoTextBlock->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

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
