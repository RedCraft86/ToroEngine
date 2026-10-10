// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToroClassCustomization.h"

/**
 * A base actor detail customization for AToroActor.
 * Uses inherited metadata visibility filtering without adding a default category allow list.
 */
class TOROCOREED_API FToroActorCustomization : public FToroClassCustomization
{
protected:

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};

/**
 * A base volume detail customization for AToroVolume.
 * Uses inherited metadata visibility filtering without adding a default category allow list.
 */
class TOROCOREED_API FToroVolumeCustomization : public FToroClassCustomization
{
protected:

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};

/**
 * A base character detail customization for AToroCharacter.
 * Supplies a default category allow list; inherited force-show and show metadata take precedence
 * over hide metadata, which takes precedence over that list. Matching includes child categories.
 */
class TOROCOREED_API FToroCharacterCustomization : public FToroClassCustomization
{
protected:

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	virtual TArray<FString> GetDefaultCategories() override;
};
