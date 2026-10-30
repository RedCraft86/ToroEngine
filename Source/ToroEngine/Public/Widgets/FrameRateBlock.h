// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CommonTextBlock.h"
#include "DataTypes/CachedGetter.h"
#include "GameFramework/GameUserSettings.h"
#include "FrameRateBlock.generated.h"

/**
 * Displays engine-wide averaged FPS and frame time, refreshed every 0.1 seconds.
 * Interpolates performance colors between 10 FPS and the configured frame rate limit,
 * using 60 FPS when the limit is disabled or user settings are unavailable.
 * Hidden and collapsed widgets skip display updates; the designer uses static preview text.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class UFrameRateBlock final : public UCommonTextBlock
{
	GENERATED_BODY()

public:

	UFrameRateBlock();

private:

	/** Color at the low-performance end of the FPS range (10 FPS). */
	UPROPERTY(EditAnywhere, Category = Appearance)
	FLinearColor LowPerformanceColor;

	/** Color at the configured FPS limit, or at the fallback target of 60 FPS. */
	UPROPERTY(EditAnywhere, Category = Appearance)
	FLinearColor HighPerformanceColor;

	TWeakPtr<FActiveTimerHandle> TickHandle;
	TCachedGetter<UGameUserSettings> UserSettings;
	FNumberFormattingOptions FrameRateDisplayOptions;
	FNumberFormattingOptions FrameTimeDisplayOptions;

	void ClearActiveTimer();
	EActiveTimerReturnType Update(double, float);

	virtual void BeginDestroy() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
};
