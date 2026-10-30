// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Widgets/FrameRateBlock.h"
#include "Widgets/SWidget.h"
#include "Engine/Engine.h"
#if WITH_EDITOR
#include "Widgets/ToroWidgetCategories.h"
#endif

extern ENGINE_API float GAverageFPS;
extern ENGINE_API float GAverageMS;

UFrameRateBlock::UFrameRateBlock()
	: UserSettings([] -> UGameUserSettings* {
		return GEngine ? GEngine->GetGameUserSettings() : nullptr;
	})
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	Font.Size = 24.0f;
	Font.TypefaceFontName = TEXT("Regular");
	Font.OutlineSettings.OutlineColor = FLinearColor::Black;
	Font.OutlineSettings.OutlineSize = 2;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	Justification = ETextJustify::Right;

	LowPerformanceColor = FLinearColor::Red;
	HighPerformanceColor = FLinearColor::Green;
	FrameRateDisplayOptions.MinimumFractionalDigits = 0;
	FrameRateDisplayOptions.MaximumFractionalDigits = 0;
	FrameTimeDisplayOptions.MinimumFractionalDigits = 2;
	FrameTimeDisplayOptions.MaximumFractionalDigits = 2;
}

void UFrameRateBlock::ClearActiveTimer()
{
	const TSharedPtr<SWidget> CurrentWidget = GetCachedWidget();
	if (CurrentWidget.IsValid() && TickHandle.IsValid())
	{
		CurrentWidget->UnRegisterActiveTimer(TickHandle.Pin().ToSharedRef());
	}

	TickHandle.Reset();
}

EActiveTimerReturnType UFrameRateBlock::Update(double, float)
{
	if (GetVisibility() != ESlateVisibility::Hidden
		&& GetVisibility() != ESlateVisibility::Collapsed)
	{
		const UGameUserSettings* Settings = UserSettings.Get();
		const float FPSLimit = Settings ? Settings->GetFrameRateLimit() : 0.0f;
		SetColorAndOpacity(FMath::Lerp(LowPerformanceColor, HighPerformanceColor,
			FMath::GetMappedRangeValueClamped(
				FVector2D(10.0f, (FPSLimit > 0.0f) ? FPSLimit : 60.0f),
				FVector2D(0.0f, 1.0f),
				GAverageFPS
			)
		));

		SetText(FText::Format(INVTEXT("{0} FPS\n{1} ms"),
			FText::AsNumber(GAverageFPS, &FrameRateDisplayOptions),
			FText::AsNumber(GAverageMS, &FrameTimeDisplayOptions)
		));
	}

	return EActiveTimerReturnType::Continue;
}

void UFrameRateBlock::BeginDestroy()
{
	ClearActiveTimer();
	Super::BeginDestroy();
}

void UFrameRateBlock::ReleaseSlateResources(bool bReleaseChildren)
{
	ClearActiveTimer();
	Super::ReleaseSlateResources(bReleaseChildren);
}

TSharedRef<SWidget> UFrameRateBlock::RebuildWidget()
{
	ClearActiveTimer();
	TSharedRef<SWidget> Widget = Super::RebuildWidget();
	if (IsDesignTime())
	{
		SetText(INVTEXT("60 FPS\n16.67 ms"));
	}
	else
	{
		TickHandle = Widget->RegisterActiveTimer(0.1f, FWidgetActiveTimerDelegate
			::CreateUObject(this, &UFrameRateBlock::Update)
		);
	}
	return Widget;
}

#if WITH_EDITOR
const FText UFrameRateBlock::GetPaletteCategory()
{
	return ToroWidgetCategories::Information;
}

bool UFrameRateBlock::CanEditChange(const FProperty* InProperty) const
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	if (InProperty)
	{
		if (InProperty->GetName() == GET_MEMBER_NAME_CHECKED(UTextBlock, ColorAndOpacity)
			|| InProperty->GetName() == GET_MEMBER_NAME_CHECKED(UTextBlock, Text))
		{
			return false;
		}
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	return Super::CanEditChange(InProperty);
}
#endif
