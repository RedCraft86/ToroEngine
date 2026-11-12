// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserInterfaceConfig.generated.h"

class UToroDialogWidget;

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FUserInterfaceConfig final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = UserInterface, NoClear, meta = (AllowAbstract = false))
	TSoftClassPtr<UToroDialogWidget> DialogWidgetClass;

	[[nodiscard]] static const FUserInterfaceConfig* Get();
};
