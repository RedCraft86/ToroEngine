// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroUserInterfaceConfig.generated.h"

class UCommonUserWidget;
class UCommonActivatableWidget;

USTRUCT(BlueprintInternalUseOnly)
struct TOROSYSTEMS_API FToroUserInterfaceConfig final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = UserInterface, meta = (AllowAbstract = false))
	TSoftClassPtr<UCommonUserWidget> MasterWidgetClass;

	UPROPERTY(EditAnywhere, Category = UserInterface, meta = (AllowAbstract = false))
	TSoftClassPtr<UCommonActivatableWidget> DialogWidgetClass;
};