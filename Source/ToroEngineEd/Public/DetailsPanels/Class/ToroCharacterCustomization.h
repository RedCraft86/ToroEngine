// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroActorCustomization.h"

class TOROENGINEED_API FToroCharacterCustomization : public FToroActorCustomization
{
protected:

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
