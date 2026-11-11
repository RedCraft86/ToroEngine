// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroUserInterfaceConfig.generated.h"

class UToroDialogWidget;

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FToroUserInterfaceConfig final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = UserInterface, NoClear, meta = (AllowAbstract = false))
	TSoftClassPtr<UToroDialogWidget> DialogWidgetClass;

	[[nodiscard]] static const FToroUserInterfaceConfig* Get();
};
