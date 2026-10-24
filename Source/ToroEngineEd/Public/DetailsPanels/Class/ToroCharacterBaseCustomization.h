// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroActorBaseCustomization.h"

class TOROENGINEED_API FToroCharacterBaseCustomization : public FToroActorBaseCustomization
{
protected:

	virtual TArray<FString> GetDefaultCategories() override;
};
