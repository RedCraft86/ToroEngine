// Copyright (C) RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Logging/LogMacros.h"
#include "Modules/ModuleInterface.h"

TOROSYSTEMS_API DECLARE_LOG_CATEGORY_EXTERN(LogToroSystems, All, All)

/**
 * Runtime systems built on the ToroEngine framework.
 */
class TOROSYSTEMS_API FToroSystemsModule final : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
