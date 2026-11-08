// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CommonActivatableWidget.h"
#include "ToroMenuWidgetBase.generated.h"

/**
 * Base for modal menus with activation focus, automatic focus restoration, and back handling enabled by default.
 * Activation requests GameAndUI input with a visible cursor and viewport mouse locking through the owning
 * AToroPlayerController, and optionally requests pause. Deactivation releases this widget's requests.
 * Input and pause requests are skipped when the owning player is not an AToroPlayerController.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API UToroMenuWidgetBase : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:

	UToroMenuWidgetBase();

protected:

	/**
	 * Whether to request pause on activation and release it on deactivation through the owning AToroPlayerController.
	 * Defaults to false; configure before activation and keep unchanged until deactivation.
	 */
	UPROPERTY(EditAnywhere, Category = Activation)
	bool bPausesGame;

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
};
