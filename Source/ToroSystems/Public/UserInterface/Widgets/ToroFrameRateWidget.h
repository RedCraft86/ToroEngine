// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserInterface/Bases/ToroAutoWidget.h"
#include "ToroFrameRateWidget.generated.h"

class UTextBlock;
class UToroGameUserSettings;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType)
class UToroFrameRateWidget final : public UToroAutoWidget
{
	GENERATED_BODY()

public:

	UToroFrameRateWidget();

private:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> InfoTextBlock;

	UPROPERTY(EditAnywhere, Category = Appearance, meta = (HideAlphaChannel = true))
	FLinearColor GoodColor;

	UPROPERTY(EditAnywhere, Category = Appearance, meta = (HideAlphaChannel = true))
	FLinearColor BadColor;

	float TickTime = 0.0f;
	TWeakObjectPtr<UToroGameUserSettings> Settings;

	void OnShowFrameRateChanged(IConsoleVariable* Variable) const;

	virtual void NativeDestruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
