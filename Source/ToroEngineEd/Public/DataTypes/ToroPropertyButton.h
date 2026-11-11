// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroPropertyButton.generated.h"

USTRUCT(BlueprintInternalUseOnly, meta = (HiddenByDefault))
struct TOROENGINEED_API FToroPropertyButton final
{
	GENERATED_BODY()

	friend class FToroPropertyButtonCustomization;

	FSimpleMulticastDelegate::RegistrationType& OnClicked()
	{
		return Delegate;
	}

private:

	FSimpleMulticastDelegate Delegate;
};