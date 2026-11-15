// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroUserWidget.h"
#include "UserInterface/UserInterfaceTypes.h"
#include "ToroAutoWidget.generated.h"

/** Destination layer for widgets automatically created by the player HUD. */
UENUM(BlueprintInternalUseOnly)
enum class EToroAutoWidgetType : uint8
{
	/** Gameplay content that fades out when a menu opens. */
	Gameplay,

	/** Overlay content displayed above gameplay and menus. */
	Overlay
};

/**
 * Base for widgets automatically created by the player HUD from the UI configuration.
 * Each configured class supplies its destination layer; the HUD retains the instance for lookup
 * and adds it to the master widget before Blueprint BeginPlay. Instances belong to that HUD's lifetime.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, NotPlaceable)
class TOROSYSTEMS_API UToroAutoWidget : public UToroUserWidget
{
	GENERATED_BODY()

public:

	UToroAutoWidget();

	/** Returns the master widget layer selected by AutoLayer. */
	[[nodiscard]] FORCEINLINE EToroMasterWidgetType GetMasterWidgetLayer() const
	{
		switch (AutoLayer)
		{
		case EToroAutoWidgetType::Gameplay:
			return EToroMasterWidgetType::Gameplay;

		case EToroAutoWidgetType::Overlay:
			return EToroMasterWidgetType::Overlay;
		}

		return EToroMasterWidgetType::Gameplay;
	}

protected:

	/**
	 * Destination used when the HUD adds this widget to the master widget; defaults to Gameplay.
	 * Changing this value after insertion does not move the existing instance between layers.
	 */
	UPROPERTY(EditAnywhere, Category = Activation)
	EToroAutoWidgetType AutoLayer;
};
