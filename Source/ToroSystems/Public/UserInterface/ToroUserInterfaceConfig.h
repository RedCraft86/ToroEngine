// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroUserInterfaceConfig.generated.h"

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FToroUserInterfaceConfig final
{
	GENERATED_BODY()

	[[nodiscard]] static const FToroUserInterfaceConfig* Get();
};
