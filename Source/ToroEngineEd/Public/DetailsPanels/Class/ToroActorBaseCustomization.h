// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "DetailsPanels/ToroClassCustomization.h"

class TOROENGINEED_API FToroActorBaseCustomization : public FToroClassCustomization
{
protected:

	AActor* GetPrimaryActor() const;

	bool GetIsEnabled()const;
	FReply OnValidateActor() const;

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
