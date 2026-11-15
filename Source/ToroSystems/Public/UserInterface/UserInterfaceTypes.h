// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserInterfaceTypes.generated.h"

/** Selects the master UI container used for an entry. */
UENUM(BlueprintType)
enum class EToroMasterWidgetType : uint8
{
	/** Gameplay information displayed beneath menus and hidden while a menu is displayed. */
	Gameplay,

	/** Menu stack whose transitions replace the gameplay presentation. */
	Menu,

	/** Independent forced overlays displayed above both gameplay content and menus. */
	Overlay
};
