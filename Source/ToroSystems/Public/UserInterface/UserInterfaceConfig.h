// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserInterfaceConfig.generated.h"

class UToroSettingsWidget;
class UToroDialogWidget;
class UToroAutoWidget;

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FUserInterfaceConfig final
{
	GENERATED_BODY()

	FUserInterfaceConfig();

	UPROPERTY(EditAnywhere, Category = UserInterface, NoClear, meta = (AllowAbstract = false))
	TSoftClassPtr<UToroSettingsWidget> SettingsWidgetClass;

	UPROPERTY(EditAnywhere, Category = UserInterface, NoClear, meta = (AllowAbstract = false))
	TSoftClassPtr<UToroDialogWidget> DialogWidgetClass;

	UPROPERTY(EditAnywhere, Category = UserInterface, NoClear, meta = (AllowAbstract = false))
	TSet<TSoftClassPtr<UToroAutoWidget>> AutoWidgetClasses;

	[[nodiscard]] static const FUserInterfaceConfig* Get();
};
