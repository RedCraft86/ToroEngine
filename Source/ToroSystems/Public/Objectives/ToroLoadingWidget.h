// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserInterface/Bases/ToroMenuWidget.h"
#include "ToroLoadingWidget.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType)
class UToroLoadingWidget final : public UToroMenuWidget
{
	GENERATED_BODY()

public:

	UToroLoadingWidget();

protected:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> LoadingText;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UProgressBar> LoadingBar;

	bool bTracking = false;
	float TickTime = 0.0f;
	int32 TotalPackages = 0;
	int32 RemainPackages = 0;
	FNumberFormattingOptions PercentFormat;

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
