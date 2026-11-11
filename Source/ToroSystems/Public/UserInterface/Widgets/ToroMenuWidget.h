// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CommonActivatableWidget.h"
#include "Input/UIActionBindingHandle.h"
#include "ToroMenuWidget.generated.h"

/**
 * Modal CommonUI menu base with activation focus, automatic focus restoration, and back handling enabled by default.
 * CommonUI applies the desired input configuration while this menu owns input routing and restores the
 * underlying layer's configuration when it closes. Defaults to menu-only input with no mouse capture or locking.
 * Optional source-owned pause requests require an owning AToroPlayerController and end on deactivation.
 * The menu stack owns activation and presentation; auto-activation and activation visibility changes are disabled.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API UToroMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:

	UToroMenuWidget();

protected:

	/**
	 * CommonUI input routing and mouse behavior used when this menu is the active input layer.
	 * An explicit Blueprint Get Desired Input Config implementation takes precedence.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Activation|Input")
	FUIInputConfig InputConfig;

	/**
	 * Requests game pause when this menu activates with an owning AToroPlayerController; false by default.
	 * Changing this setting while active does not immediately update pause. Deactivation releases any existing request.
	 */
	UPROPERTY(EditAnywhere, Category = Activation)
	bool bPausesGame;

	void UpdatePauseRequest() const;

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
};
