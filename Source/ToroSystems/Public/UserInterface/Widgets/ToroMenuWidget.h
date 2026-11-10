// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CommonActivatableWidget.h"
#include "ToroMenuWidget.generated.h"

/**
 * Modal CommonUI menu base with activation focus, automatic focus restoration, and back handling enabled by default.
 * While active, requests GameAndUI input with a visible cursor, viewport mouse locking, and this menu as the focus target.
 * Input configuration and optional pause requests are owned by this menu and released on deactivation.
 * These controller requests require an owning AToroPlayerController; other controller types are left unchanged.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API UToroMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:

	UToroMenuWidget();

protected:

	/**
	 * Requests game pause when this menu activates with an owning AToroPlayerController; false by default.
	 * Changing this setting while active does not immediately update pause. Deactivation releases any existing request.
	 */
	UPROPERTY(EditAnywhere, Category = Activation)
	bool bPausesGame;

	void UpdateControllerRequests();

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
};
