// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "UserInterfaceConfig.generated.h"

class UToroDialogWidget;
class UToroSettingsWidget;
class UToroInventoryWidget;
class UToroAutoWidget;

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FUserInterfaceConfig final
{
	GENERATED_BODY()

	FUserInterfaceConfig();

	UPROPERTY(EditAnywhere, Category = "UserInterface|TextTiming", meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float TextMinDuration = 2.0f;

	UPROPERTY(EditAnywhere, Category = "UserInterface|TextTiming", meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float TextCharactersPerSecond = 15.0f;

	UPROPERTY(EditAnywhere, Category = "UserInterface|Classes", NoClear, meta = (AllowAbstract = false))
	TSoftClassPtr<UToroDialogWidget> DialogWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UserInterface|Classes", NoClear, meta = (AllowAbstract = false))
	TSoftClassPtr<UToroSettingsWidget> SettingsWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UserInterface|Classes", NoClear, meta = (AllowAbstract = false))
	TSoftClassPtr<UToroInventoryWidget> InventoryWidgetclass;

	UPROPERTY(EditAnywhere, Category = "UserInterface|Classes", NoClear, meta = (AllowAbstract = false))
	TSet<TSoftClassPtr<UToroAutoWidget>> AutoWidgetClasses;

	[[nodiscard]] float CalculateTimeForText(const FText& InText, float ExtraTime = 0.0f) const;

	[[nodiscard]] static const FUserInterfaceConfig* Get();
};
