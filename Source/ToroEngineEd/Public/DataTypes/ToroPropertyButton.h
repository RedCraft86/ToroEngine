// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#include "ToroPropertyButton.generated.h"

/**
 * Editor Details-panel button whose native click listeners belong to this struct instance.
 * Expose as an editable UPROPERTY; Label metadata supplies the button text, falling back to the property name.
 * WholeRowButton metadata places the property tooltip above a full-row button; otherwise it is the button tooltip.
 * The customization does not provide an actionable button for multiple selected values.
 */
USTRUCT(BlueprintInternalUseOnly, meta = (HiddenByDefault))
struct TOROENGINEED_API FToroPropertyButton final
{
	GENERATED_BODY()

	/**
	 * Provides registration access to the native click event broadcast by the Details customization.
	 * Bind callbacks on the struct instance displayed in Details; no event is raised simply by constructing it.
	 * @return Registration interface owned by this struct, valid for its lifetime.
	 */
	FSimpleMulticastDelegate::RegistrationType& OnClicked()
	{
		return Delegate;
	}

private:

	friend class FToroPropertyButtonCustomization;

	FSimpleMulticastDelegate Delegate;
};
